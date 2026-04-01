#pragma once

#include <stdint.h>

namespace Color565 {

constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t value = ((r & 0xF8u) << 8) | ((g & 0xFCu) << 3) | (b >> 3);
    return value;
}

constexpr uint16_t bswap(uint16_t v) {
    uint16_t value = (v << 8) | (v >> 8);
    return value;
}

constexpr int clampInt(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

constexpr uint16_t lighten(uint16_t c) {
    int r = (c >> 11) & 0x1F;
    int g = (c >> 5) & 0x3F;
    int b = c & 0x1F;

    r = clampInt(r + ((r / 3) > 0 ? (r / 3) : 1), 0, 31);
    g = clampInt(g + ((g / 3) > 0 ? (g / 3) : 1), 0, 63);
    b = clampInt(b + ((b / 3) > 0 ? (b / 3) : 1), 0, 31);

    uint16_t value = (r << 11) | (g << 5) | b;
    return value;
}

constexpr uint16_t darken(uint16_t c) {
    int r = (c >> 11) & 0x1F;
    int g = (c >> 5) & 0x3F;
    int b = c & 0x1F;

    r = (r * 2) / 3;
    g = (g * 2) / 3;
    b = (b * 2) / 3;

    uint16_t value = (r << 11) | (g << 5) | b;
    return value;
}

}  // namespace Color565
