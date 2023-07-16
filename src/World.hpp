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