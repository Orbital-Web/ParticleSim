#include "src/World.hpp"
#include <SFML/Graphics.hpp>



int main() {
    const int width = 800;
    const int height = 800;
    const int fps = 30;
    const int substeps = 6;

    // setup
    sf::RenderWindow window(sf::VideoMode(width, height), "Particule Simulation");
    window.setFramerateLimit(fps);
    sf::Event event;
    psim::World world(width, height, fps, substeps);

    // --- use case: two particles linked by a spring ---
    // A and B start symmetric about the world center on the x-axis.
    const float cx = width / 2.f;
    const float cy = height / 2.f;
    const float offset = 100.f;          // distance from center along x
    const float weak_vel = 1.f;          // weak opposite initial velocity

    // Particle A: left of center, moving right
    world.add_particle(
        {12.f, 1.f, 0.05f},                // radius, mass, restitution
        sf::Color::Green,
        {cx - offset - 100, cy},          // position
        {weak_vel, 0.f},                  // velocity
        {0.f, 0.f}                        // base acceleration (no gravity)
    );

    // Particle B: right of center, moving left
    world.add_particle(
        {12.f, 1.f, 0.05f},
        sf::Color::Red,
        {cx + offset, cy},
        {-weak_vel, 0.f},
        {0.f, 0.f}
    );

    // Link A (index 0) and B (index 1) with a spring.
    // k = 5, rest_length = 2 * offset = 200 (initial distance)
    world.add_spring(0, 1, 5.f, 2.f * offset);

    // Add viscous drag (friction of the medium) on every particle.
    // F = -c * v  (linear in velocity)
    const float drag_coeff = 0.5f;
    world.add_force_field([drag_coeff](psim::Particle& p, float dt) {
        sf::Vector2f v = p.get_velocity(dt);
        p.add_force(-drag_coeff * v);
    });

    // until window closed
    while (window.isOpen()) {
        // events
        while (window.pollEvent(event)) {
            // close window
            if (event.type == sf::Event::Closed)
                window.close();
        }

        world.update(window);
    }
}