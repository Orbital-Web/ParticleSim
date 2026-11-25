#pragma once
#include "Types.hpp"
#include "Particle.hpp"
#include "LinkBuilder.hpp"
#include <SFML/Graphics.hpp>
#include <vector>

namespace psim {

class Renderer {
public:
    // Clear, draw circles (Rgb -> sf::Color) and links as white lines, then display.
    void draw(sf::RenderWindow& window,
              const std::vector<Particle>& particles,
              const std::vector<Link>& links) {
        window.clear(sf::Color::Black);

        for (const Particle& p : particles) {
            sf::CircleShape shape(p.prop.radius);
            shape.setOrigin(p.prop.radius, p.prop.radius);
            shape.setPosition(p.pos);
            shape.setFillColor(to_color(p.color));
            window.draw(shape);
        }

        for (const Link& l : links) {
            sf::Vertex line[] = {
                sf::Vertex(particles[l.i].pos, sf::Color::White),
                sf::Vertex(particles[l.j].pos, sf::Color::White)
            };
            window.draw(line, 2, sf::Lines);
        }

        window.display();
    }

private:
    static sf::Color to_color(const Rgb& c) {
        return sf::Color(c.r, c.g, c.b);
    }
};

} // namespace psim