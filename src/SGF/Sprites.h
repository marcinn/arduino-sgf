#pragma once

#include <stdint.h>
#include <array>

// Simple software sprites layer; meant to be composed over a background buffer.
// Clients fill sprite/missile slots and call renderRegion(...) after the background
// for a region is written into the buffer.
class SpriteLayer {
public:
  enum class ScaleX {
    Normal,
    DoubleWidth,
  };

  struct Sprite {
    bool active = false;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    const uint16_t* pixels565 = nullptr;
    uint16_t transparent = 0;  // pixels matching this value are skipped
    ScaleX scale = ScaleX::Normal;
  };

  struct Missile {
    bool active = false;
    int x = 0;
    int y = 0;
    int w = 1;    // typically 1-2 px
    int h = 0;
    uint16_t color = 0xFFFF;
    ScaleX scale = ScaleX::Normal;
  };

  static constexpr int kMaxSprites = 8;
  static constexpr int kMaxMissiles = 4;

  SpriteLayer();

  void clearSprites();
  void clearMissiles();
  void clearAll();

  Sprite& sprite(int index);
  Missile& missile(int index);

  void renderRegion(int x0, int y0, int w, int h, uint16_t* buf) const;

private:
  std::array<Sprite, kMaxSprites> sprites_{};
  std::array<Missile, kMaxMissiles> missiles_{};
};
