#pragma once
#include "Types.hpp"
#include "Particle.hpp"
#include "LinkBuilder.hpp"
#include "LifeRules.hpp"
#include "PhysicsEngine.hpp"
#include <vector>
#include <functional>
#include <cstddef>
#include <utility>

namespace psim {

struct SimConfig {
    int width;
    int height;
    int fps;
    int substeps;
    float a = 30.f;
    float tau = 0.f;
    float spring_k = 5.f;
    float birth_offset = 0.05f;
    int link_every = 5;   // in frames
    int life_every = 60;  // in frames
    Particle::Property default_prop{12.f, 1.f, 0.5f};
    Rgb default_color{255, 0, 0};
};

class World {
public:
    explicit World(SimConfig cfg)
        : cfg_(cfg), engine_(cfg.width, cfg.height) {}

    void add_particle(Particle::Property prop, Rgb color, Vec2 pos, Vec2 vel_px_per_s, Vec2 acc) {
        float dt_sub = 1.f / (cfg_.fps * cfg_.substeps);
        Vec2 pos_i = pos - vel_px_per_s * dt_sub;
        particles_.push_back(Particle(prop, color, pos, pos_i, acc));
    }

    void add_force_field(std::function<void(Particle&, float)> f) {
        engine_.add_force_field(std::move(f));
    }

    // Advance one frame (physics only, no rendering).
    void update() {
        const float dt_sub = 1.f / (cfg_.fps * cfg_.substeps);
        for (int s = 0; s < cfg_.substeps; s++)
            engine_.substep(particles_, links_, dt_sub);

        frame_++;

        if (frame_ % cfg_.link_every == 0)
            rebuild_links();

        if (frame_ % cfg_.life_every == 0) {
            Snapshot snap;
            snap.pos.reserve(particles_.size());
            snap.vel.reserve(particles_.size());
            for (const Particle& p : particles_) {
                snap.pos.push_back(p.pos);
                snap.vel.push_back(p.get_velocity(dt_sub));
            }

            LifeResult res = evaluate_life(snap, cfg_.width, cfg_.height, cfg_.a, cfg_.tau);

            // Remove deaths in a single compaction pass (indices sorted ascending).
            if (!res.deaths.empty()) {
                std::vector<Particle> kept;
                kept.reserve(particles_.size() - res.deaths.size());
                size_t d = 0;
                for (size_t i = 0; i < particles_.size(); i++) {
                    if (d < res.deaths.size() && res.deaths[d] == i) {
                        d++;
                        continue;
                    }
                    kept.push_back(std::move(particles_[i]));
                }
                particles_ = std::move(kept);
            }

            // Add births, displaced toward the neighbors' centroid to trigger the spring effect.
            // Neighborhood is evaluated on the pre-death snapshot (simultaneous update, Rule 2).
            if (!res.births.empty()) {
                SpatialHash hash(cfg_.width, cfg_.height, cfg_.a);
                hash.rebuild(snap.pos);
                const float shift = cfg_.birth_offset * cfg_.a;

                for (const Birth& b : res.births) {
                    Vec2 spawn = b.pos;

                    Vec2 sum{0.f, 0.f};
                    int n = 0;
                    for (int j : neighborhood(hash, b.pos, cfg_.a)) {
                        if (j >= 0) { sum += snap.pos[j]; n++; }
                    }
                    if (n > 0) {
                        Vec2 dir = sum / float(n) - b.pos;
                        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                        if (len > 1e-6f)
                            spawn = b.pos + dir * (shift / len);
                    }

                    add_particle(cfg_.default_prop, cfg_.default_color, spawn, b.vel, {0.f, 0.f});
                }
            }

            // Indices changed: rebuild links immediately.
            rebuild_links();
        }
    }

    const std::vector<Particle>& particles() const { return particles_; }
    const std::vector<Link>& links() const { return links_; }

private:
    void rebuild_links() {
        std::vector<Vec2> pos;
        pos.reserve(particles_.size());
        for (const Particle& p : particles_)
            pos.push_back(p.pos);
        links_ = build_links(pos, cfg_.width, cfg_.height, cfg_.a, cfg_.spring_k);
    }

    SimConfig cfg_;
    PhysicsEngine engine_;
    std::vector<Particle> particles_;
    std::vector<Link> links_;
    int frame_ = 0;
};

} // namespace psim