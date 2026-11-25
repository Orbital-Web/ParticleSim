#pragma once
#include "Types.hpp"
#include "SpatialHash.hpp"
#include "SlotLattice.hpp"
#include <vector>
#include <cstddef>

namespace psim {

struct Link {
    size_t i, j;
    float k;
    float rest;
};

// Build springs for every (particle, slot) pair that has a neighbor.
// No deduplication (matches previous behavior).
inline std::vector<Link> build_links(const std::vector<Vec2>& pos, int w, int h, float a, float k) {
    SpatialHash hash(w, h, a);
    hash.rebuild(pos);

    const auto rest_lengths = slot_rest_lengths(a);
    std::vector<Link> links;
    for (size_t i = 0; i < pos.size(); i++) {
        auto nb = neighborhood(hash, pos[i], a);
        for (size_t s = 0; s < nb.size(); s++) {
            int j = nb[s];
            if (j >= 0)
                links.push_back({i, static_cast<size_t>(j), k, rest_lengths[s]});
        }
    }
    return links;
}

} // namespace psim