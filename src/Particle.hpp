#pragma once
#include <SFML/Graphics.hpp>
#include <math.h>
#include <cassert>



namespace psim {
class Particle {
// class variables
public:
    static constexpr float MAX_RADIUS = 12;
    struct Property {
        float radius;      // (0, MAX_RADIUS]
        float mass;        // (0, inf)
        float restitution; // (0, 1]
    };

private:
    // visual
    sf::Color color;
    sf::CircleShape sprite;

    // physical
    Property prop; 
    sf::Vector2f pos, pos_i, acc;



// methods
public:
    // constructor
    Particle(const Property prop_in,
             const sf::Color color_in,
             const sf::Vector2f pos_in,
             const sf::Vector2f pos_i_in,
             const sf::Vector2f acc_in
    ):  prop(prop_in), color(color_in), pos(pos_in), pos_i(pos_i_in), acc(acc_in) {
        assert((prop_in.radius <= MAX_RADIUS));
        sprite.setRadius(prop_in.radius);
        sprite.setOrigin(prop_in.radius, prop_in.radius);
        sprite.setFillColor(color_in);
    }


    // draws the particle on the window
    void render(sf::RenderWindow& window) {
        sprite.setPosition((sf::Vector2f)pos);
        window.draw(sprite);
    }


    // updates the position of the particle
    void update_physics(const float dt, const int width, const int height) {
        sf::Vector2f pos_old = pos;
        // x(n+1) = 2x(n) - x(n-1) + a*t^2
        pos = float(2)*pos - pos_i + acc*dt*dt;
        pos_i = pos_old;
        contain(width, height);
    }


    // updates positions so two this and other are no longer in collision
    void resolve_collision(Particle& other) {
        sf::Vector2f dir = other.pos - pos;
        float d_sq = dir.x*dir.x + dir.y*dir.y;
        float sum_r = prop.radius + other.prop.radius;

        // objects in collision
        if (sum_r*sum_r > d_sq) {
            float d = sqrt(d_sq);
            float sum_m = prop.mass + other.prop.mass;
            sf::Vector2f ndir = dir / (d * sum_m);
            float d_adjust = 0.5 * prop.restitution * other.prop.restitution * (sum_r - d);

            // apply collision
            pos -= ndir * other.prop.mass * d_adjust;
            other.pos += ndir * prop.mass * d_adjust;
        }
    }


    // returns position
    sf::Vector2f get_pos() const {
        return pos;
    }

private:
    // ensures the particle stays in the world
    void contain(const int width, const int height) {
        // left
        if (pos.x - prop.radius < 0)
            pos.x = prop.radius;
        // right
        if (pos.x + prop.radius > width)
            pos.x = width - prop.radius;
        // top
        if (pos.y - prop.radius < 0)
            pos.y = prop.radius;
        // bottom
        if (pos.y + prop.radius > height)
            pos.y = height - prop.radius;
    }
};  // Particle
}   // namespace psim