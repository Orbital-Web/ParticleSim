#pragma once
#include "Types.hpp"
#include "SpatialHash.hpp"
#include <array>
#include <cmath>

namespace psim {

// The 8 slot offsets in a FIXED order.
inline std::array<Vec2, 8> slot_offsets(float a) {
    return {{
        { a,  0}, {-a,  0}, { 0,  a}, { 0, -a},
        { a,  a}, {-a,  a}, { a, -a}, {-a, -a}
    }};
}

// Rest length per slot: a for orthogonal, a*sqrt(2) for diagonal.
inline std::array<float, 8> slot_rest_lengths(float a) {
    const float diag = a * std::sqrt(2.f);
    return {{ a, a, a, a, diag, diag, diag, diag }};
}

// For each slot, the index of the particle nearest to (center+offset) within
// a/2 (strictly), or -1 if the slot is empty.
inline std::array<int, 8> neighborhood(const SpatialHash& hash, Vec2 center, float a) {
    const auto offsets = slot_offsets(a);
    std::array<int, 8> result;
    for (size_t s = 0; s < offsets.size(); s++)
        result[s] = hash.nearest_within(center + offsets[s], a * 0.5f);
    return result;
}

} // namespace psim