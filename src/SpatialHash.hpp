#pragma once
#include "Types.hpp"
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstddef>

namespace psim {

class SpatialHash {
public:
    SpatialHash(int width, int height, float cell_size)
        : width_(width), height_(height), cell_size_(cell_size) {
        n_cols_ = static_cast<int>(std::ceil(static_cast<float>(width) / cell_size));
        n_rows_ = static_cast<int>(std::ceil(static_cast<float>(height) / cell_size));
        if (n_cols_ < 1) n_cols_ = 1;
        if (n_rows_ < 1) n_rows_ = 1;
        cells_.assign(static_cast<size_t>(n_rows_) * n_cols_, {});
    }

    // Rebuild the grid from positions; index i corresponds to positions[i].
    void rebuild(const std::vector<Vec2>& positions) {
        positions_ = positions; // store a copy
        for (auto& cell : cells_)
            cell.clear();
        for (size_t i = 0; i < positions_.size(); i++)
            cells_[cell_index(positions_[i])].push_back(static_cast<int>(i));
    }

    // Closest particle strictly within radius, smallest index on tie, -1 if none.
    int nearest_within(Vec2 p, float radius) const {
        int best = -1;
        float best_d2 = radius * radius;
        int c0 = clamp_col(p.x - radius), c1 = clamp_col(p.x + radius);
        int r0 = clamp_row(p.y - radius), r1 = clamp_row(p.y + radius);
        for (int r = r0; r <= r1; r++) {
            for (int c = c0; c <= c1; c++) {
                for (int i : cells_[static_cast<size_t>(r) * n_cols_ + c]) {
                    Vec2 d = positions_[i] - p;
                    float d2 = d.x * d.x + d.y * d.y;
                    if (d2 < best_d2 || (best >= 0 && d2 == best_d2 && i < best)) {
                        best_d2 = d2;
                        best = i;
                    }
                }
            }
        }
        return best;
    }

    // All particles strictly within radius, sorted by increasing index.
    void neighbors_within(Vec2 p, float radius, std::vector<int>& out) const {
        out.clear();
        float r2 = radius * radius;
        int c0 = clamp_col(p.x - radius), c1 = clamp_col(p.x + radius);
        int r0 = clamp_row(p.y - radius), r1 = clamp_row(p.y + radius);
        for (int r = r0; r <= r1; r++) {
            for (int c = c0; c <= c1; c++) {
                for (int i : cells_[static_cast<size_t>(r) * n_cols_ + c]) {
                    Vec2 d = positions_[i] - p;
                    if (d.x * d.x + d.y * d.y < r2)
                        out.push_back(i);
                }
            }
        }
        std::sort(out.begin(), out.end());
    }

private:
    int width_, height_;
    float cell_size_;
    int n_cols_ = 0, n_rows_ = 0;
    std::vector<std::vector<int>> cells_;
    std::vector<Vec2> positions_;

    int col_of(float x) const {
        int c = static_cast<int>(std::floor(x / cell_size_));
        return c;
    }
    int row_of(float y) const {
        return static_cast<int>(std::floor(y / cell_size_));
    }
    int clamp_col(float x) const {
        int c = col_of(x);
        if (c < 0) c = 0;
        if (c >= n_cols_) c = n_cols_ - 1;
        return c;
    }
    int clamp_row(float y) const {
        int r = row_of(y);
        if (r < 0) r = 0;
        if (r >= n_rows_) r = n_rows_ - 1;
        return r;
    }
    size_t cell_index(Vec2 p) const {
        return static_cast<size_t>(clamp_row(p.y)) * n_cols_ + clamp_col(p.x);
    }
};

} // namespace psim