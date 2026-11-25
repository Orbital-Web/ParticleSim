#pragma once
#include "Types.hpp"
#include "Particle.hpp"
#include "SpatialHash.hpp"
#include "LinkBuilder.hpp"
#include "BS_thread_pool_light.hpp"
#include <vector>
#include <functional>

namespace psim {

class PhysicsEngine {
public:
    PhysicsEngine(int width, int height)
        : width_(width), height_(height), cell_size_(2.f * Particle::MAX_RADIUS) {}

    void add_force_field(std::function<void(Particle&, float)> f) {
        force_fields_.push_back(std::move(f));
    }

    void substep(std::vector<Particle>& particles, const std::vector<Link>& links, float dt_sub) {
        const size_t N = particles.size();

        // 1. Collisions: sequential and deterministic.
        {
            std::vector<Vec2> pos(N);
            for (size_t i = 0; i < N; i++)
                pos[i] = particles[i].pos;
            SpatialHash hash(width_, height_, cell_size_);
            hash.rebuild(pos);

            std::vector<int> neighbors;
            for (size_t i = 0; i < N; i++) {
                hash.neighbors_within(particles[i].pos, 2.f * Particle::MAX_RADIUS, neighbors);
                for (int j : neighbors) {
                    if (static_cast<size_t>(j) > i)
                        particles[i].resolve_collision(particles[static_cast<size_t>(j)]);
                }
            }
        }

        // 2. Reset acceleration accumulators.
        for (size_t i = 0; i < N; i++)
            particles[i].reset_acc();

        // 3. Force fields: parallel, one task per particle.
        for (size_t i = 0; i < N; i++) {
            pool_.push_task([&particles, i, this, dt_sub] {
                for (auto& f : force_fields_)
                    f(particles[i], dt_sub);
            });
        }
        pool_.wait_for_tasks();

        // 4. Springs: sequential (shared particles would race). Hooke F = k*(dist-rest).
        for (const Link& l : links) {
            Vec2 d = particles[l.j].pos - particles[l.i].pos;
            float dist = std::sqrt(d.x * d.x + d.y * d.y);
            if (dist > 1e-6f) {
                Vec2 dir = d / dist;
                float fmag = l.k * (dist - l.rest);
                Vec2 f = fmag * dir;
                particles[l.i].add_force(f);
                particles[l.j].add_force(-f);
            }
        }

        // 5. Integrate motion: parallel, one task per particle.
        for (size_t i = 0; i < N; i++) {
            pool_.push_task([&particles, i, this, dt_sub] {
                particles[i].update_physics(dt_sub, width_, height_);
            });
        }
        pool_.wait_for_tasks();
    }

private:
    int width_, height_;
    float cell_size_;
    BS::thread_pool_light pool_;
    std::vector<std::function<void(Particle&, float)>> force_fields_;
};

} // namespace psim