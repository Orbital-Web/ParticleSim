#include "Particle.hpp"
#include <SFML/Graphics.hpp>
#include <vector>



namespace psim {
class World {
// class variables
public:
    static constexpr int MAX_PARTICLES = 1024;

private:
    int fps;
    int width, height;
    int sub_steps;
    std::vector<psim::Particle*> particles;



// methods
public:
    // constructor
    World(const int width_in, const int height_in,
          const int fps_in, const int sub_steps_in
    ):  width(width_in), height(height_in), fps(fps_in), sub_steps(sub_steps_in) {}


    // updates both physics and render
    void update(sf::RenderWindow& window) {
        update_physics(1.0/fps);
        update_render(window);
    }


    // adds an object
    void add_particle(const psim::Particle::Property props,
                      const sf::Color color,
                      const sf::Vector2<double> pos,
                      const sf::Vector2<double> vel,
                      const sf::Vector2<double> acc) {
        sf::Vector2<double> pos_i = pos - vel*(1.0/fps);
        particles.push_back(new psim::Particle(props, color, pos, pos_i, acc));
    }


private:
    // update phyics of every particle
    void update_physics(const double dt) {
        double dt_sub = dt / sub_steps;
        for (int i=0; i<sub_steps; i++)
            update_physics_sub(dt_sub);
    }


    // sub-stepped updating of every particle
    void update_physics_sub(const double dt) {
        // collision
        for (auto p1: particles)
            for (auto p2: particles)
                if (p1 != p2)
                    p1->resolve_collision(*p2);
        // fix into world
        for (auto p : particles)
            p->contain(width, height);
        // update
        for (auto p : particles)
            p->update_physics(dt);
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