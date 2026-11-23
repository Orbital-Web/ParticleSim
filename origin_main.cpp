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
    int n_particles = 0;

    // until window closed
    while (window.isOpen()) {
        // events
        while (window.pollEvent(event)) {
            // close window
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (n_particles < psim::World::MAX_PARTICLES) {
            float s = 4*float(n_particles % psim::World::MAX_PARTICLES) / psim::World::MAX_PARTICLES;
            s = s - (int)s;
            world.add_particle({float(12-std::pow(6*s-3,2)), 25-20*s, float(0.5*s+0.5)},
                {
                    sf::Uint8(7.6*(1.05-s)*pow(s,3)*255),
                    sf::Uint8(15*pow(1.0-s,2)*pow(s,2)*255),
                    sf::Uint8(7.5*pow(1.0-s,3)*(s+0.05)*255)
                },
                {50,50}, {120,0}, {0,1000}
            );
            n_particles++;
        }

        world.update(window);
    }
}