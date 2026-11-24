#include "src/World.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>

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

    // --- use case: test update_life with a cross pattern (birth demonstration) ---
    const float cx = width / 2.f;
    const float cy = height / 2.f;
    const float a = 20.f;                    // slot step = reference radius

    // Create a cross pattern of 5 particles (center + 4 orthogonal neighbors)
    // This pattern triggers births in the diagonal slots according to B3/S23 rules.
    world.add_particle({12.f, 1.f, 0.5f}, sf::Color::Red, {cx, cy}, {0, 0}, {1, 0});       // center
    world.add_particle({12.f, 1.f, 0.5f}, sf::Color::Red, {cx, cy - a}, {0, 0}, {1, 0});   // up
    world.add_particle({12.f, 1.f, 0.5f}, sf::Color::Red, {cx, cy + a}, {0, 0}, {1, 0});   // down
    world.add_particle({12.f, 1.f, 0.5f}, sf::Color::Red, {cx - a, cy}, {0, 0}, {1, 0});   // left
    world.add_particle({12.f, 1.f, 0.5f}, sf::Color::Red, {cx + a, cy}, {0, 0}, {1, 0});   // right

    // Add viscous drag (friction of the medium) on every particle.
    const float drag_coeff = 0.3f;
    world.add_force_field([drag_coeff](psim::Particle& p, float dt) {
        sf::Vector2f v = p.get_velocity(dt);
        p.add_force(-drag_coeff * v);
    });

    // until window closed
    int frame_count = 0;
    while (window.isOpen()) {
        // events
        while (window.pollEvent(event)) {
            // close window
            if (event.type == sf::Event::Closed)
                window.close();
        }

        world.update(window);

        // every 5 frames, update links based on slot neighborhood (Rule 3)
        frame_count++;
        if (frame_count % 5 == 0) {
            world.update_links(a);
        }

        // every 30 frames (1 second), evaluate life rule and trigger deaths
        if (frame_count % 30 == 0) {
            world.update_life(a, 0.f);
        }
    }
}


