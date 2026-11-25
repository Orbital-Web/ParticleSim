#pragma once
#include <SFML/System.hpp>
#include <cstdint>

namespace psim {

using Vec2 = sf::Vector2f;

struct Rgb {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

} // namespace psim