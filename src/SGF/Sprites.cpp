#include "Sprites.h"

SpriteLayer::SpriteLayer() = default;

void SpriteLayer::clearSprites() {
  for (auto& s : sprites_) s.active = false;
}

void SpriteLayer::clearMissiles() {
  for (auto& m : missiles_) m.active = false;
}

void SpriteLayer::clearAll() {
  clearSprites();
  clearMissiles();
}

SpriteLayer::Sprite& SpriteLayer::sprite(int index) {
  if (index < 0) index = 0;
  if (index >= kMaxSprites) index = kMaxSprites - 1;
  return sprites_[index];
}

SpriteLayer::Missile& SpriteLayer::missile(int index) {
  if (index < 0) index = 0;
  if (index >= kMaxMissiles) index = kMaxMissiles - 1;
  return missiles_[index];
}

void SpriteLayer::renderRegion(int x0, int y0, int w, int h, uint16_t* buf) const {
  if (!buf || w <= 0 || h <= 0) return;

  auto blitMissile = [&](const Missile& m) {
    if (!m.active || m.h <= 0 || m.w <= 0) return;
    int mx0 = m.x;
    int my0 = m.y;
    int mx1 = m.x + ((m.scale == ScaleX::DoubleWidth) ? (m.w * 2) : m.w) - 1;
    int my1 = m.y + m.h - 1;
    if (mx1 < x0 || mx0 >= x0 + w || my1 < y0 || my0 >= y0 + h) return;

    int rx0 = (mx0 < x0) ? x0 : mx0;
    int ry0 = (my0 < y0) ? y0 : my0;
    int rx1 = (mx1 > x0 + w - 1) ? (x0 + w - 1) : mx1;
    int ry1 = (my1 > y0 + h - 1) ? (y0 + h - 1) : my1;

    for (int yy = ry0; yy <= ry1; ++yy) {
      for (int xx = rx0; xx <= rx1; ++xx) {
        int sx = xx - mx0;
        if (m.scale == ScaleX::DoubleWidth) sx /= 2;
        if (sx < 0 || sx >= m.w) continue;

        int bufX = xx - x0;
        int bufY = yy - y0;
        buf[bufY * w + bufX] = m.color;
      }
    }
  };

  auto blitSprite = [&](const Sprite& s) {
    if (!s.active || !s.pixels565 || s.w <= 0 || s.h <= 0) return;
    int sx0 = s.x;
    int sy0 = s.y;
    int scaledW = (s.scale == ScaleX::DoubleWidth) ? (s.w * 2) : s.w;
    int sx1 = sx0 + scaledW - 1;
    int sy1 = sy0 + s.h - 1;
    if (sx1 < x0 || sx0 >= x0 + w || sy1 < y0 || sy0 >= y0 + h) return;

    int rx0 = (sx0 < x0) ? x0 : sx0;
    int ry0 = (sy0 < y0) ? y0 : sy0;
    int rx1 = (sx1 > x0 + w - 1) ? (x0 + w - 1) : sx1;
    int ry1 = (sy1 > y0 + h - 1) ? (y0 + h - 1) : sy1;

    for (int yy = ry0; yy <= ry1; ++yy) {
      int srcY = yy - sy0;
      for (int xx = rx0; xx <= rx1; ++xx) {
        int srcX = xx - sx0;
        if (s.scale == ScaleX::DoubleWidth) srcX /= 2;
        if (srcX < 0 || srcX >= s.w) continue;

        const uint16_t color = s.pixels565[srcY * s.w + srcX];
        if (color == s.transparent) continue;

        int bufX = xx - x0;
        int bufY = yy - y0;
        buf[bufY * w + bufX] = color;
      }
    }
  };

  for (const auto& m : missiles_) blitMissile(m);
  for (const auto& s : sprites_) blitSprite(s);
}
