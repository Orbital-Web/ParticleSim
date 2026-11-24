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

    // --- use case: a red multicellular structure of 5 linked cells ---
    const float cx = width / 2.f;
    const float cy = height / 2.f;
    const float spacing = 80.f;          // distance between cell centers
    const float rest_length = 60.f;     // spring rest length between cells
    const float k = 5.f;                // spring stiffness

    // Create 5 particles arranged in a cross (+) pattern
    // Center particle at (cx, cy)
    world.add_particle({12.f, 1.f, 0.05f}, sf::Color::Red, {cx, cy}, {0, 0}, {0, 0});
    // Up
    world.add_particle({12.f, 1.f, 0.05f}, sf::Color::Red, {cx, cy - spacing}, {0, 0}, {0, 0});
    // Down
    world.add_particle({12.f, 1.f, 0.05f}, sf::Color::Red, {cx, cy + spacing}, {0, 0}, {0, 0});
    // Left
    world.add_particle({12.f, 1.f, 0.05f}, sf::Color::Red, {cx - spacing, cy}, {0, 0}, {0, 0});
    // Right
    world.add_particle({12.f, 1.f, 0.05f}, sf::Color::Red, {cx + spacing, cy}, {0, 0}, {0, 0});

    // Create one red cell containing all 5 particles
    int cell0 = world.add_cell(sf::Color::Red);
    for (int i = 0; i < 5; i++)
        world.add_particle_to_cell(cell0, i);


    // Link center (0) to each of the 4 outer particles (1..4)
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 5; j++)
            if (i != j)
                world.add_spring_to_cell(cell0, i, j, k, rest_length);



    // Add viscous drag (friction of the medium) on every particle.
    // F = -c * v  (linear in velocity)
    const float drag_coeff = 0.5f;
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

        // every 15 frames, auto-link nearby particles
        frame_count++;
        if (frame_count % 15 == 0) {
            world.auto_link(120.f, 5.f, 80.f);
        }

        // after 30 seconds (900 frames at 30 fps), remove the center particle
        if (frame_count == 900 && world.get_particle_count() > 0) {
            world.remove_particle(world.get_particle(0));
        }
    }
}