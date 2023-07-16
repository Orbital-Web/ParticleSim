#pragma once
#include "Particle.hpp"
#include <vector>
#include <math.h>



namespace psim {
class Partition {
// class variables
public:
    static constexpr int cellsize = psim::Particle::MAX_RADIUS;
    int n_rows, n_cols;

private:
    std::vector<std::vector<Particle*>> pgrid;  // a vector of pointers to particles in each cell



// methods
public:
    // constructor
    Partition(const int width, const int height) {
        n_rows = ceil((float)width / cellsize);
        n_cols = ceil((float)height / cellsize);
        reset();
    }


    // adds a particle to the correct cell
    void add_particle(Particle* particle) {
        auto pos = particle->get_pos();
        int r = pos.y / cellsize;
        int c = pos.x / cellsize;
        pgrid[n_rows*r + c].push_back(particle);
    }


    // returns all particles in the specified cell
    std::vector<Particle*> get(const int row, const int col) {
        return pgrid[n_rows*row + col];
    }


    // resets the grid
    void reset() {
        pgrid.clear();
        pgrid.resize(n_rows*n_cols);
    }


    // prints number of particles in each cell for debugging
    void print() {
        for (int r=0; r<n_rows; r++) {
            for (int c=0; c<n_cols; c++)
                printf("%d ", get(r, c).size());
            printf("\n");
        }
        printf("\n");
    }
};  // partitions
}   // namespace psim