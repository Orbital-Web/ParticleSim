#pragma once
#include "Particle.hpp"
#include "Partition.hpp"
#include "BS_thread_pool_light.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <chrono>



namespace psim {
class World {
// class variables
public:
    static constexpr int MAX_PARTICLES = 2048;

private:
    int fps;
    int width, height;
    int substeps;
    std::vector<psim::Particle*> particles;
    Partition partition;
    BS::thread_pool_light pool;

    // force fields: global forces applied to every particle (e.g. gravity, drag)
    std::vector<std::function<void(psim::Particle&, float)>> force_fields;

    // springs: local forces between pairs of particles (Hooke's law)
    struct Spring {
        Particle* p1;
        Particle* p2;
        float k;
        float rest_length;
    };
    std::vector<Spring> springs;



// methods
public:
    // constructor
    World(const int width_in, const int height_in,
          const int fps_in, const int sub_steps_in
    ):  width(width_in), height(height_in), fps(fps_in), substeps(sub_steps_in), partition(width_in, height_in) {}


    // destructor
    ~World() {
        for (auto& p : particles)
            delete p;
    }


    // updates both physics and render
    void update(sf::RenderWindow& window) {
        update_physics(1.0/fps);
        update_render(window);
    }


    // adds a particle
    void add_particle(const Particle::Property props,
                      const sf::Color color,
                      const sf::Vector2f pos,
                      const sf::Vector2f vel,
                      const sf::Vector2f acc) {
        sf::Vector2f pos_i = pos - vel*float(1.0/fps);
        particles.push_back(new Particle(props, color, pos, pos_i, acc));
    }

    // registers a global force field applied to every particle each substep
    void add_force_field(std::function<void(psim::Particle&, float)> f) {
        force_fields.push_back(f);
    }

    // links two particles (by index) with a spring (Hooke's law: F = k*(dist - rest_length))
    void add_spring(const int i, const int j, const float k, float rest_length = 0.f) {
        springs.push_back({particles[i], particles[j], k, rest_length});
    }


private:
    // update phyics of every particle
    void update_physics(const float dt) {
        float dt_sub = dt / substeps;
        for (int i=0; i<substeps; i++)
            update_physics_sub(dt_sub);
    }


    // sub-stepped updating of every particle
    void update_physics_sub(const float dt) {
        /*
        // collision
        for (auto& p1 : particles)
            for (auto& p2 : particles)
                if (p1 != p2)
                    p1->resolve_collision(*p2);
        */
        // initialize partition
        partition.reset();
        for (auto& p : particles)
           partition.add_particle(p);
        
        // collision with partition in two passes (even & odd rows)
        for (int pass=0; pass<=1; pass++) {
            for (int r=pass; r<partition.n_rows; r+=2) {
                pool.push_task([r, this] {
                for (int c=0; c<this->partition.n_cols; c++)
                    this->collision_partition(r, c);
                });
            }
            pool.wait_for_tasks();
        }

        // reset acceleration accumulator for every particle (base = acc_base, typically 0)
        for (auto& p : particles)
            p->reset_acc();

        // apply global force fields (parallelizable: each particle writes its own acc)
        for (auto& p : particles)
            pool.push_task([p, this, dt] {
                for (auto& f : this->force_fields)
                    f(*p, dt);
            });
        pool.wait_for_tasks();

        // apply local spring forces (sequential: shared particles would race)
        for (auto& s : springs) {
            sf::Vector2f d = s.p2->get_pos() - s.p1->get_pos();
            float dist = std::sqrt(d.x*d.x + d.y*d.y);
            if (dist > 1e-6f) {
                sf::Vector2f dir = d / dist;
                float fmag = s.k * (dist - s.rest_length);   // Hooke: F = k*(L - L0)
                sf::Vector2f f = fmag * dir;                  // force on p1 (toward p2 if stretched)
                s.p1->add_force(f);
                s.p2->add_force(-f);
            }
        }

        // fix into world and apply motion
        for (auto& p : particles)
            pool.push_task([p, this, dt] {p->update_physics(dt, this->width, this->height);});
        pool.wait_for_tasks();
    }


    // 
    void collision_partition(const int row, const int col) {
        auto cur_cell = partition.get(row, col);
        if (cur_cell.size() == 0)
            return;

        for (int dr=-1; dr<2; dr++) {
            for (int dc=-1; dc<2; dc++) {
                if (row+dr>=0 && row+dr<partition.n_rows && col+dc>=0 && col+dc<partition.n_cols) {
                    auto other_cell = partition.get(row+dr, col+dc);
                    collision_cells(cur_cell, other_cell);
                }
            }
        }
    }


    // 
    void collision_cells(std::vector<Particle*>& cur_cell, std::vector<Particle*>& other_cell) {
        for (auto& p1 : cur_cell)
            for (auto& p2 : other_cell)
                if (p1 != p2)
                    p1->resolve_collision(*p2);
    }


    // renders every particle
    void update_render(sf::RenderWindow& window) {
        window.clear({0, 0, 0});
        for (auto& p : particles)
            p->render(window);
        window.display();
    }
};  // World
}   // namespace psim