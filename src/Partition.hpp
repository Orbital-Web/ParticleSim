#pragma once
#include "Particle.hpp"
#include <vector>
#include <math.h>



namespace psim {
class Partition {
// class variables
public:
    static constexpr int cellsize = psim::Particle::MAX_RADIUS;
    int row0, col0;     // top-left cell with particle
    int row1, col1;     // bottom-right cell with particle

private:
    int n_rows, n_cols;
    std::vector<std::vector<Particle*>> pgrid;  // a vector of pointers to particles in each cell



// methods
public:
    // constructor
    Partition(const int width, const int height) {
        int minsize = psim::Particle::MIN_RADIUS;

        // set variables
        n_rows = ceil((float)width / cellsize);
        n_cols = ceil((float)height / cellsize);
        reset();
    }


    // adds a particle
    void add_particle(Particle* particle) {
        sf::Vector2<double> pos = particle->get_pos();
        int row = pos.y / cellsize;
        int col = pos.x / cellsize;
        row0 = std::min(row, row0);
        col0 = std::min(col, col0);
        row1 = std::max(row, row1);
        col1 = std::max(col, col1);
        constraint();
        pgrid[n_rows*row + col].push_back(particle);
    }


    // constraints row0, col0, row1, and col1
    void constraint() {
        row0 = std::min(n_rows-2, std::max(1, row0));
        col0 = std::min(n_cols-2, std::max(1, col0));
        row1 = std::min(n_rows-2, std::max(1, row1));
        col1 = std::min(n_cols-2, std::max(1, col1));
    }


    // returns all particles in the specified cell as well as its neighbors
    // requires 1<=row<n_rows-1, 1<=col<n_cols-1
    std::vector<Particle*> relevant_particles(const int row, const int col) const {
        std::vector<Particle*> particles;

        for (int i=-1; i<2; i++) {
            for (int j=-1; j<2; j++) {
                int r = row + i;
                int c = col + j;
                if (pgrid[n_rows*r+c].size() > 0)
                    particles.insert(particles.end(), pgrid[n_rows*r+c].begin(), pgrid[n_rows*r+c].end());
            }
        }

        return particles;
    }


    // resets the grid
    void reset() {
        pgrid.clear();
        pgrid.resize(n_rows*n_cols);
        row0 = n_rows - 2;
        col0 = n_cols - 2;
        row1 = 1;
        col1 = 1;
    }
};  // partitions
}   // namespace psim