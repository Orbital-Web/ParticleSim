#pragma once
#include "Types.hpp"
#include <cassert>
#include <cmath>

namespace psim {

class Particle {
public:
    static constexpr float MAX_RADIUS = 12;

    struct Property {
        float radius;      // (0, MAX_RADIUS]
        float mass;        // (0, inf)
        float restitution; // (0, 1]
    };

    // Physical + visual data. Copyable value stored in std::vector<Particle>.
    Property prop;
    Rgb color;
    Vec2 pos, pos_i, acc;
    Vec2 acc_base; // base acceleration (reset each substep)

    Particle(Property prop_in, Rgb color_in, Vec2 pos_in, Vec2 pos_i_in, Vec2 acc_in)
        : prop(prop_in), color(color_in), pos(pos_in), pos_i(pos_i_in), acc(acc_in), acc_base(acc_in) {
        assert(prop_in.radius <= MAX_RADIUS);
    }

    // Verlet integration step, then keep the particle inside the world.
    void update_physics(float dt, int width, int height) {
        Vec2 pos_old = pos;
        // x(n+1) = 2x(n) - x(n-1) + a*t^2
        pos = 2.f * pos - pos_i + acc * dt * dt;
        pos_i = pos_old;
        contain(width, height);
    }

    // Resolve an overlap between this particle and another.
    void resolve_collision(Particle& other) {
        Vec2 dir = other.pos - pos;
        float d_sq = dir.x * dir.x + dir.y * dir.y;
        float sum_r = prop.radius + other.prop.radius;

        if (sum_r * sum_r > d_sq) {
            float d = std::sqrt(d_sq);
            float sum_m = prop.mass + other.prop.mass;
            Vec2 ndir = dir / (d * sum_m);
            float d_adjust = 0.5f * prop.restitution * other.prop.restitution * (sum_r - d);

            pos -= ndir * other.prop.mass * d_adjust;
            other.pos += ndir * prop.mass * d_adjust;
        }
    }

    // Add a force (divided by mass) to the acceleration accumulator.
    void add_force(const Vec2& f) {
        acc += f / prop.mass;
    }

    // Reset the acceleration accumulator to the base value (each substep).
    void reset_acc() {
        acc = acc_base;
    }

    Vec2 get_pos() const {
        return pos;
    }

    // Velocity estimate from Verlet positions (v ~= (pos - pos_i) / dt).
    Vec2 get_velocity(float dt) const {
        return (pos - pos_i) / dt;
    }

private:
    void contain(int width, int height) {
        if (pos.x - prop.radius < 0) pos.x = prop.radius;
        if (pos.x + prop.radius > width) pos.x = width - prop.radius;
        if (pos.y - prop.radius < 0) pos.y = prop.radius;
        if (pos.y + prop.radius > height) pos.y = height - prop.radius;
    }
};

} // namespace psim