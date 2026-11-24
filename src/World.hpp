#pragma once
#include "Particle.hpp"
#include "Partition.hpp"
#include "BS_thread_pool_light.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <chrono>



namespace psim {
class World {
// class variables
public:
    static constexpr int MAX_PARTICLES = 2048;

private:
    int fps;
    int width, height;
    int substeps;
    std::vector<psim::Particle*> particles;
    Partition partition;
    BS::thread_pool_light pool;

    // force fields: global forces applied to every particle (e.g. gravity, drag)
    std::vector<std::function<void(psim::Particle&, float)>> force_fields;

    // springs: local forces between pairs of particles (Hooke's law)
    struct Spring {
        Particle* p1;
        Particle* p2;
        float k;
        float rest_length;
    };
    std::vector<Spring> springs;

    // cells: groups of particles with their own internal springs and a color.
    // A particle can belong to multiple cells; forces from all cells add up.
    // If a particle is added to two cells, the cells merge automatically.
    struct Cell {
        std::vector<Particle*> particles;
        std::vector<Spring> springs;   // internal springs of this cell
        sf::Color color;
    };
    std::vector<Cell> cells;



// methods
public:
    // constructor
    World(const int width_in, const int height_in,
          const int fps_in, const int sub_steps_in
    ):  width(width_in), height(height_in), fps(fps_in), substeps(sub_steps_in), partition(width_in, height_in) {}


    // destructor
    ~World() {
        for (auto& p : particles)
            delete p;
    }


    // updates both physics and render
    void update(sf::RenderWindow& window) {
        update_physics(1.0/fps);
        update_render(window);
    }


    // adds a particle
    void add_particle(const Particle::Property props,
                      const sf::Color color,
                      const sf::Vector2f pos,
                      const sf::Vector2f vel,
                      const sf::Vector2f acc) {
        sf::Vector2f pos_i = pos - vel * (1.f / (fps * substeps));
        particles.push_back(new Particle(props, color, pos, pos_i, acc));
    }

    // registers a global force field applied to every particle each substep
    void add_force_field(std::function<void(psim::Particle&, float)> f) {
        force_fields.push_back(f);
    }

    // links two particles (by index) with a spring (Hooke's law: F = k*(dist - rest_length))
    void add_spring(const int i, const int j, const float k, float rest_length = 0.f) {
        springs.push_back({particles[i], particles[j], k, rest_length});
    }

    // creates a new cell and returns its index for further configuration
    int add_cell(const sf::Color color) {
        cells.push_back(Cell{{}, {}, color});
        return static_cast<int>(cells.size()) - 1;
    }

    // adds a particle (by index) to an existing cell.
    // If the particle already belongs to another cell, the two cells merge.
    void add_particle_to_cell(const int cell_idx, const int particle_idx) {
        Particle* p = particles[particle_idx];

        // find if particle is already in another cell
        int other_idx = -1;
        for (int i = 0; i < static_cast<int>(cells.size()); i++) {
            if (i == cell_idx) continue;
            for (auto* q : cells[i].particles) {
                if (q == p) {
                    other_idx = i;
                    break;
                }
            }
            if (other_idx >= 0) break;
        }

        if (other_idx >= 0) {
            // merge the two cells
            merge_cells(cell_idx, other_idx);
        } else {
            cells[cell_idx].particles.push_back(p);
        }
    }

// links two particles (by index) inside an existing cell with a spring
    void add_spring_to_cell(const int cell_idx, const int i, const int j,
                            const float k, float rest_length = 0.f) {
        cells[cell_idx].springs.push_back({particles[i], particles[j], k, rest_length});
    }

    // updates spring links based on slot neighborhood (Rule 3).
    // r = reference radius (slot step a = r, half-width = r/2)
    // 8 slots: (±a, 0), (0, ±a), (±a, ±a). Rest length: a for orthogonal, a√2 for diagonal.
    void update_links(const float r) {
        const float half_r = r / 2.f;
        const float a = r;
        const float diag = a * std::sqrt(2.f);

        const std::vector<sf::Vector2f> slot_offsets = {
            { a,  0}, {-a,  0}, { 0,  a}, { 0, -a},
            { a,  a}, {-a,  a}, { a, -a}, {-a, -a}
        };
        const std::vector<float> slot_rest_lengths = {
            a, a, a, a, diag, diag, diag, diag
        };

        // Clear existing springs (recomputed each tick)
        springs.clear();

        for (size_t i = 0; i < particles.size(); i++) {
            sf::Vector2f pos_i = particles[i]->get_pos();

            for (size_t s = 0; s < slot_offsets.size(); s++) {
                sf::Vector2f slot_pos = pos_i + slot_offsets[s];

                // Find a particle in this slot (distance < r/2)
                for (size_t j = 0; j < particles.size(); j++) {
                    if (i == j) continue;
                    sf::Vector2f d = particles[j]->get_pos() - slot_pos;
                    float dist = std::sqrt(d.x*d.x + d.y*d.y);
                    if (dist < half_r) {
                        springs.push_back({particles[i], particles[j], 5.f, slot_rest_lengths[s]});
                        break;  // one particle per slot
                    }
                }
            }
        }
    }

    // draws springs as lines between linked particles.
    // global_spring_color / global_spring_width: for global springs (default white, 2px)
    // cell_spring_color / cell_spring_width: for cell-internal springs (default cell color, 2px)
    void draw_springs(sf::RenderWindow& window,
                      const sf::Color global_spring_color = sf::Color::White,
                      const float global_spring_width = 2.f,
                      const sf::Color cell_spring_color = sf::Color::White,
                      const float cell_spring_width = 2.f) {
        // global springs
        for (auto& s : springs) {
            sf::Vector2f p1 = s.p1->get_pos();
            sf::Vector2f p2 = s.p2->get_pos();
            sf::Vertex line[] = {
                sf::Vertex(p1, global_spring_color),
                sf::Vertex(p2, global_spring_color)
            };
            window.draw(line, 2, sf::Lines);
        }

        // cell-internal springs
        for (auto& cell : cells) {
            sf::Color color = (cell_spring_color == sf::Color::White) ? cell.color : cell_spring_color;
            for (auto& s : cell.springs) {
                sf::Vector2f p1 = s.p1->get_pos();
                sf::Vector2f p2 = s.p2->get_pos();
                sf::Vertex line[] = {
                    sf::Vertex(p1, color),
                    sf::Vertex(p2, color)
                };
                window.draw(line, 2, sf::Lines);
            }
        }
    }

    // returns the number of particles in the world
    int get_particle_count() const {
        return static_cast<int>(particles.size());
    }

    // returns a pointer to the particle at the given index
    Particle* get_particle(const int i) {
        return particles[i];
    }

    // evaluates life rule (Rule 4), triggers clean deaths (Rule 6), and births (Rule 5).
    // r = reference radius, tau = tolerance (default 0 = strict B3/S23)
    void update_life(const float r, const float tau = 0.f) {
    const float a = r, half_r = r * 0.5f, half_r2 = half_r * half_r;
    const float dt_sub = 1.f / (fps * substeps);
    const sf::Vector2f offsets[8] = {
        { a, 0}, {-a, 0}, {0,  a}, {0, -a},
        { a, a}, {-a, a}, {a, -a}, {-a, -a}
    };

    // 1. snapshot
    const size_t N = particles.size();
    std::vector<sf::Vector2f> pos(N), vel(N);
    for (size_t i = 0; i < N; i++) {
        pos[i] = particles[i]->get_pos();
        vel[i] = particles[i]->get_velocity(dt_sub);
    }

    auto dist2 = [](sf::Vector2f u, sf::Vector2f v) {
        sf::Vector2f d = u - v; return d.x*d.x + d.y*d.y;
    };
    auto occupied = [&](sf::Vector2f p) {
        for (size_t j = 0; j < N; j++)
            if (dist2(pos[j], p) < half_r2) return true;
        return false;
    };
    // voisinage d'un point du réseau : slots remplis autour de `c`
    struct Neigh { int n = 0; sf::Vector2f vel_sum{0, 0}; };
    auto neighborhood = [&](sf::Vector2f c) {
        Neigh nb;
        for (const auto& o : offsets) {
            sf::Vector2f s = c + o;
            for (size_t j = 0; j < N; j++) {
                if (dist2(pos[j], s) < half_r2) {
                    nb.n++; nb.vel_sum += vel[j]; break;
                }
            }
        }
        return nb;
    };

    // 2. décès, décidés sur le snapshot
    std::vector<Particle*> to_remove;
    for (size_t i = 0; i < N; i++) {
        int n = neighborhood(pos[i]).n;
        if (n < 2 - tau || n > 3 + tau) to_remove.push_back(particles[i]);
    }

    // 3. candidats (slots libres dédupliqués), ordre déterministe
    std::vector<sf::Vector2f> candidates;
    for (size_t i = 0; i < N; i++)
        for (const auto& o : offsets) {
            sf::Vector2f s = pos[i] + o;
            if (occupied(s)) continue;
            bool dup = false;
            for (auto& c : candidates)
                if (dist2(c, s) < half_r2) { dup = true; break; }
            if (!dup) candidates.push_back(s);
        }

    // 4. naissances, décidées sur le même snapshot
    struct Birth { sf::Vector2f pos, vel; };
    std::vector<Birth> births;
    for (auto& c : candidates) {
        Neigh nb = neighborhood(c);
        if (nb.n > 0 && nb.n >= 3 - tau && nb.n <= 3 + tau)
            births.push_back({c, nb.vel_sum / float(nb.n)});
    }

    // 5. application simultanée
    for (auto* p : to_remove) remove_particle(p);
    for (auto& b : births)
        add_particle({12.f, 1.f, 0.5f}, sf::Color::Red, b.pos, b.vel, {0, 0});
}

    // cleanly removes a particle from the world: deletes it, removes it from
    // the global list, removes all springs referencing it, and removes it from
    // all cells. No impulse is applied to remaining particles.
    void remove_particle(Particle* p) {
        // 1. Remove from global particles list
        auto it = std::find(particles.begin(), particles.end(), p);
        if (it != particles.end()) {
            particles.erase(it);
        }

        // 2. Remove all global springs referencing p
        springs.erase(
            std::remove_if(springs.begin(), springs.end(),
                [p](const Spring& s) {
                    return s.p1 == p || s.p2 == p;
                }),
            springs.end()
        );

        // 3. Remove from all cells and remove cell-internal springs referencing p
        for (auto& cell : cells) {
            // Remove particle from cell
            cell.particles.erase(
                std::remove(cell.particles.begin(), cell.particles.end(), p),
                cell.particles.end()
            );

            // Remove cell-internal springs referencing p
            cell.springs.erase(
                std::remove_if(cell.springs.begin(), cell.springs.end(),
                    [p](const Spring& s) {
                        return s.p1 == p || s.p2 == p;
                    }),
                cell.springs.end()
            );
        }

        // 4. Free memory
        delete p;
    }


private:
    // merges two cells into one. The major cell (larger population) absorbs the minor.
    void merge_cells(int cell_a, int cell_b) {
        if (cell_a == cell_b) return;

        Cell& ca = cells[cell_a];
        Cell& cb = cells[cell_b];

        // determine minor / major
        int minor_idx, major_idx;
        if (ca.particles.size() <= cb.particles.size()) {
            minor_idx = cell_a;
            major_idx = cell_b;
        } else {
            minor_idx = cell_b;
            major_idx = cell_a;
        }

        Cell& minor = cells[minor_idx];
        Cell& major = cells[major_idx];

        // merge particles (no duplicates)
        for (auto* p : minor.particles) {
            if (std::find(major.particles.begin(), major.particles.end(), p) == major.particles.end())
                major.particles.push_back(p);
        }

        // merge springs
        major.springs.insert(major.springs.end(), minor.springs.begin(), minor.springs.end());

        // mix color: 30% minor + 70% major
        major.color.r = static_cast<sf::Uint8>(0.3f * minor.color.r + 0.7f * major.color.r);
        major.color.g = static_cast<sf::Uint8>(0.3f * minor.color.g + 0.7f * major.color.g);
        major.color.b = static_cast<sf::Uint8>(0.3f * minor.color.b + 0.7f * major.color.b);

        // remove the minor cell
        cells.erase(cells.begin() + minor_idx);
    }


private:
    // update phyics of every particle
    void update_physics(const float dt) {
        float dt_sub = dt / substeps;
        for (int i=0; i<substeps; i++)
            update_physics_sub(dt_sub);
    }


    // sub-stepped updating of every particle
    void update_physics_sub(const float dt) {
        /*
        // collision
        for (auto& p1 : particles)
            for (auto& p2 : particles)
                if (p1 != p2)
                    p1->resolve_collision(*p2);
        */
        // initialize partition
        partition.reset();
        for (auto& p : particles)
           partition.add_particle(p);
        
        // collision with partition in two passes (even & odd rows)
        for (int pass=0; pass<=1; pass++) {
            for (int r=pass; r<partition.n_rows; r+=2) {
                pool.push_task([r, this] {
                for (int c=0; c<this->partition.n_cols; c++)
                    this->collision_partition(r, c);
                });
            }
            pool.wait_for_tasks();
        }

        // reset acceleration accumulator for every particle (base = acc_base, typically 0)
        for (auto& p : particles)
            p->reset_acc();

        // apply global force fields (parallelizable: each particle writes its own acc)
        for (auto& p : particles)
            pool.push_task([p, this, dt] {
                for (auto& f : this->force_fields)
                    f(*p, dt);
            });
        pool.wait_for_tasks();

        // apply local spring forces (sequential: shared particles would race)
        for (auto& s : springs) {
            sf::Vector2f d = s.p2->get_pos() - s.p1->get_pos();
            float dist = std::sqrt(d.x*d.x + d.y*d.y);
            if (dist > 1e-6f) {
                sf::Vector2f dir = d / dist;
                float fmag = s.k * (dist - s.rest_length);   // Hooke: F = k*(L - L0)
                sf::Vector2f f = fmag * dir;                  // force on p1 (toward p2 if stretched)
                s.p1->add_force(f);
                s.p2->add_force(-f);
            }
        }

        // apply cell-internal spring forces (sequential)
        for (auto& cell : cells) {
            for (auto& s : cell.springs) {
                sf::Vector2f d = s.p2->get_pos() - s.p1->get_pos();
                float dist = std::sqrt(d.x*d.x + d.y*d.y);
                if (dist > 1e-6f) {
                    sf::Vector2f dir = d / dist;
                    float fmag = s.k * (dist - s.rest_length);
                    sf::Vector2f f = fmag * dir;
                    s.p1->add_force(f);
                    s.p2->add_force(-f);
                }
            }
        }

        // fix into world and apply motion
        for (auto& p : particles)
            pool.push_task([p, this, dt] {p->update_physics(dt, this->width, this->height);});
        pool.wait_for_tasks();
    }


    // 
    void collision_partition(const int row, const int col) {
        auto cur_cell = partition.get(row, col);
        if (cur_cell.size() == 0)
            return;

        for (int dr=-1; dr<2; dr++) {
            for (int dc=-1; dc<2; dc++) {
                if (row+dr>=0 && row+dr<partition.n_rows && col+dc>=0 && col+dc<partition.n_cols) {
                    auto other_cell = partition.get(row+dr, col+dc);
                    collision_cells(cur_cell, other_cell);
                }
            }
        }
    }


    // 
    void collision_cells(std::vector<Particle*>& cur_cell, std::vector<Particle*>& other_cell) {
        for (auto& p1 : cur_cell)
            for (auto& p2 : other_cell)
                if (p1 != p2)
                    p1->resolve_collision(*p2);
    }


    // renders every particle
    void update_render(sf::RenderWindow& window) {
        window.clear({0, 0, 0});
        for (auto& p : particles)
            p->render(window);
        draw_springs(window);
        window.display();
    }
};  // World
}   // namespace psim