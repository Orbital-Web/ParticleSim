#include "src/World.hpp"
#include <SFML/Graphics.hpp>



int main() {
    const int width = 1024;
    const int height = 1024;
    const int fps = 30;
    const int sub_steps = 8;

    // setup
    sf::RenderWindow window(sf::VideoMode(width, height), "Particule Simulation");
    window.setFramerateLimit(fps);
    sf::Event event;
    psim::World world(width, height, fps, sub_steps);
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
                float s = float(n_particles % psim::World::MAX_PARTICLES) / psim::World::MAX_PARTICLES;
                world.add_particle({15-pow(6*s-3,2), 10*s+5, 0.5*s+0.5},
                    {
                        sf::Uint8(7.6*(1.05-s)*pow(s,3)*255),
                        sf::Uint8(15*pow(1.0-s,2)*pow(s,2)*255),
                        sf::Uint8(7.5*pow(1.0-s,3)*(s+0.05)*255)
                    },
                    {50,50}, {30,0}, {0,1000}
                );
                n_particles++;
            }

            world.update(window);
        }
}

/*
g++ -c main.cpp -ISFML-2.6.0/include -DSFML_STATIC
g++ -o main main.o -LSFML-2.6.0/lib -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lwinmm -lgdi32
del main.o
main.exe

*/