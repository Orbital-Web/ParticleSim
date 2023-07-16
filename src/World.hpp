#pragma once
#include "Particle.hpp"
#include "Partition.hpp"
#include "BS_thread_pool_light.hpp"
#include <SFML/Graphics.hpp>
#include <vector>



namespace psim {
class World {
// class variables
public:
    static constexpr int MAX_PARTICLES = 256;

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
        for (auto p : particles)
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
                      const sf::Vector2<double> pos,
                      const sf::Vector2<double> vel,
                      const sf::Vector2<double> acc) {
        sf::Vector2<double> pos_i = pos - vel*(1.0/fps);
        particles.push_back(new Particle(props, color, pos, pos_i, acc));
    }


private:
    // update phyics of every particle
    void update_physics(const double dt) {
        double dt_sub = dt / substeps;
        for (int i=0; i<substeps; i++)
            update_physics_sub(dt_sub);
    }


    // sub-stepped updating of every particle
    void update_physics_sub(const double dt) {
        /*
        // collision
        for (auto p1 : particles)
            for (auto p2 : particles)
                if (p1 != p2)
                    p1->resolve_collision(*p2);
        */
        // initialize partition
        partition.reset();
        for (auto p : particles)
           partition.add_particle(p);
        
        // collision with partition in two passes (even & odd rows)
        for (int pass=0; pass<=1; pass++) {
            for (int r=pass; r<partition.n_rows; r+=2) {
                pool.push_task([r, this] {
                for (int c=0; c<this->partition.n_cols; c++)
                    for (auto p1 : this->partition.get(r, c))
                        for (auto p2 : this->partition.get(r, c))
                            if (p1 != p2)
                                p1->resolve_collision(*p2);
                });
            }
            pool.wait_for_tasks();
        }


        // fix into world and apply motion
        for (auto p : particles)
            pool.push_task([p, this, dt] {p->update_physics(dt, this->width, this->height);});
        pool.wait_for_tasks();
    }


    // renders every particle
    void update_render(sf::RenderWindow& window) {
        window.clear({0, 0, 0});
        for (auto p : particles)
            p->render(window);
        window.display();
    }
};  // World
}   // namespace psim