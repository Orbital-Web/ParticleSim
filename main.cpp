#include "src/World.hpp"
#include "src/Renderer.hpp"
#include <SFML/Graphics.hpp>

#include <random>
#include <vector>
#include <cmath>
#include <algorithm>

int main() {
    const int width = 800;
    const int height = 800;
    const int fps = 60;
    const int substeps = 6;

    sf::RenderWindow window(sf::VideoMode(width, height), "Particule Simulation");
    window.setFramerateLimit(fps);
    sf::Event event;

    psim::SimConfig cfg;
    cfg.width = width;
    cfg.height = height;
    cfg.fps = fps;
    cfg.substeps = substeps;

    psim::World world(cfg);
    psim::Renderer renderer;

    // --- Random initial population: N cells in a centered R x R square ---
    const int      N = 10;
    const float    R = 5.f * cfg.a;      // side of the square (kept inside the window below)
    const bool     snap_to_lattice = true;
    // const unsigned seed = 4200;            // fixed seed: deterministic runs (Rule 9)

    const float a  = cfg.a;
    const float cx = width / 2.f;
    const float cy = height / 2.f;
    const psim::Rgb red{255, 0, 0};
    const psim::Particle::Property prop = cfg.default_prop;

    // half-side, clamped so every particle stays fully inside the window
    const float half = std::min(R * 0.5f, std::min(width, height) * 0.5f - prop.radius);
    // min spacing: lattice step if snapped, else no overlap
    const float min_d = snap_to_lattice ? a : 2.f * prop.radius;

    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> uni(-half, half);

    std::vector<psim::Vec2> placed;
    for (int attempt = 0; (int)placed.size() < N && attempt < 100000; attempt++) {
        psim::Vec2 p{cx + uni(rng), cy + uni(rng)};

        if (snap_to_lattice) {
            p.x = cx + std::round((p.x - cx) / a) * a;
            p.y = cy + std::round((p.y - cy) / a) * a;
            if (std::abs(p.x - cx) > half || std::abs(p.y - cy) > half)
                continue;                // snapped outside the square
        }

        bool ok = true;
        for (const psim::Vec2& q : placed) {
            float dx = p.x - q.x, dy = p.y - q.y;
            if (dx*dx + dy*dy < 0.99f * min_d * min_d) { ok = false; break; }
        }
        if (!ok) continue;

        placed.push_back(p);
        world.add_particle(prop, red, p, {0, 0}, {0, 0});
    }

    // Viscous drag (friction of the medium) on every particle.
    const float drag_coeff = 0.3f;
    world.add_force_field([drag_coeff](psim::Particle& p, float dt) {
        psim::Vec2 v = p.get_velocity(dt);
        p.add_force(-drag_coeff * v);
    });

    while (window.isOpen()) {
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        world.update();
        renderer.draw(window, world.particles(), world.links());
    }
}