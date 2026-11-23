# Design rules: Game of Life + Verlet physics

## Rule 1 — Discrete life state, continuous position

Each living cell is a Verlet particle (position, previous position, mass). Physics only drives the position, and only the life rule ([Rule 4](#rule-4--unified-life-rule-with-tolerance)) changes the state.

## Rule 2 — Physics runs faster than life

Perform `N` Verlet substeps between two life ticks, so the system approaches equilibrium before evaluation. At each tick, life is evaluated on a **snapshot of positions**, with a **simultaneous update**: all decisions are computed before applying births and deaths.

## Rule 3 — Neighborhood by slots

Around each cell, there are 8 slots: `(±a, 0)`, `(0, ±a)`, `(±a, ±a)`. The step `a` equals `r` (the reference radius).

- A slot is **filled** if a cell is at a distance strictly less than `r/2` from its position. Otherwise, it is **free**.
- The **neighborhood** of a cell consists of its filled slots.
- **Free slots** are the birth candidates. They are **deduplicated**: they are traversed in a deterministic order, and a candidate is rejected if it is less than `r/2` from an already retained candidate.

## Rule 4 — Unified life rule, with tolerance

The number of neighbors `n` is the sum of the slot weights.

| Mode    | Filled slot | Free slot             |
| ------- | ----------- | --------------------- |
| Strict  | `1`         | `0`                   |
| Soft    | `1` → `0`   | linear over distance  |

In soft mode, the weight decreases linearly from `1` (cell right on the slot) to `0` (distance `r/2`).

- A cell **survives** if `n ∈ [2 − τ, 3 + τ]`, and **dies** otherwise (underpopulation if `n < 2 − τ`, overpopulation if `n > 3 + τ`).
- A free candidate gives a **birth** if `n ∈ [3 − τ, 3 + τ]`.
- `τ ≥ 0` is the **tolerance** parameter. With `τ = 0` and strict mode, you get exactly **B3/S23** on aligned positions.

**Hysteresis option:** an additional margin for cells already alive (they survive a bit more easily than a candidate is born).

## Rule 5 — A birth is a physical event

The new cell appears at the position of the candidate, with the average velocity of its neighbors. The "free" criterion (no cell within `r/2`) already guarantees the minimum distance, so no overlap.

## Rule 6 — A death is clean

A death removes the particle and its links, without impulse. Life never creates energy.

## Rule 7 — Links and repulsion

- A cell is **linked by springs** to its neighbors (filled slots). The rest length is that of the slot: `a` for the orthogonal, `a√2` for the diagonal. Cells are thus drawn toward a lattice, and still lifes become equilibria.
- A link **breaks** beyond a maximum elongation. Links are recomputed at each tick according to [Rule 3](#rule-3--neighborhood-by-slots).
- A **soft repulsion** acts between two cells less than `r/2` apart.

## Rule 8 — Environment forces

Each environment force must break symmetry in a controlled and independent way. Enable them one by one:

- **Friction / damping** — dissipates energy, stabilizes structures.
- **Static obstacles** — alter neighborhoods and can act as shelters.
- **Wind / currents** — forced displacement transports or destroys patterns.

## Rule 9 — Everything is deterministic

No fixed time step, no random seeds, stable traversal order of the spatial hash (including for candidate deduplication).