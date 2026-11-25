#pragma once
#include "Types.hpp"
#include "SpatialHash.hpp"
#include "SlotLattice.hpp"
#include <vector>
#include <cstddef>

namespace psim {

struct Snapshot {
    std::vector<Vec2> pos;
    std::vector<Vec2> vel;
};

struct Birth {
    Vec2 pos;
    Vec2 vel;
};

struct LifeResult {
    std::vector<size_t> deaths;
    std::vector<Birth> births;
};

// Pure evaluation of the life rule (strict mode, weight 1 per filled slot).
// Reads only its input; produces deaths and births decided on the snapshot.
inline LifeResult evaluate_life(const Snapshot& snap, int width, int height, float a, float tau) {
    (void)width;
    (void)height;

    const size_t N = snap.pos.size();
    SpatialHash hash(width, height, a);
    hash.rebuild(snap.pos);

    LifeResult result;

    // 2. Deaths, decided on the snapshot.
    for (size_t i = 0; i < N; i++) {
        auto nb = neighborhood(hash, snap.pos[i], a);
        int n = 0;
        for (int idx : nb)
            if (idx >= 0) n++;
        if (n < 2 - tau || n > 3 + tau)
            result.deaths.push_back(i);
    }

    // 3. Candidates: free slots, deduplicated in deterministic order.
    const auto offsets = slot_offsets(a);
    const float half = a * 0.5f;
    const float half2 = half * half;
    std::vector<Vec2> candidates;
    for (size_t i = 0; i < N; i++) {
        for (const Vec2& o : offsets) {
            Vec2 s = snap.pos[i] + o;
            if (hash.nearest_within(s, half) != -1)
                continue; // slot not free
            bool dup = false;
            for (const Vec2& c : candidates) {
                Vec2 d = c - s;
                if (d.x * d.x + d.y * d.y < half2) { dup = true; break; }
            }
            if (!dup)
                candidates.push_back(s);
        }
    }

    // 4. Births, decided on the same snapshot.
    for (const Vec2& c : candidates) {
        auto nb = neighborhood(hash, c, a);
        int n = 0;
        Vec2 vel_sum{0.f, 0.f};
        for (int idx : nb) {
            if (idx >= 0) {
                n++;
                vel_sum += snap.vel[idx];
            }
        }
        if (n > 0 && n >= 3 - tau && n <= 3 + tau)
            result.births.push_back({c, vel_sum / static_cast<float>(n)});
    }

    return result;
}

} // namespace psim