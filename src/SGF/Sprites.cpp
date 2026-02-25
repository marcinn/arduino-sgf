#include "Sprites.h"

namespace {

bool doublesX(SpriteLayer::Scale scale) {
  return scale == SpriteLayer::Scale::DoubleX || scale == SpriteLayer::Scale::Double;
}

bool doublesY(SpriteLayer::Scale scale) {
  return scale == SpriteLayer::Scale::DoubleY || scale == SpriteLayer::Scale::Double;
}

int scaledWidth(const SpriteLayer::Sprite& s) {
  return doublesX(s.scale) ? (s.w * 2) : s.w;
}

int scaledHeight(const SpriteLayer::Sprite& s) {
  return doublesY(s.scale) ? (s.h * 2) : s.h;
}

int anchorOffset(float anchor, int span) {
  if (span <= 0) {
    return 0;
  }
  float value = anchor * (float)(span - 1);
  return (value >= 0.0f) ? (int)(value + 0.5f) : (int)(value - 0.5f);
}

void spriteTopLeft(const SpriteLayer::Sprite& s, int* x, int* y) {
  int w = scaledWidth(s);
  int h = scaledHeight(s);
  if (x) {
    *x = s.x - anchorOffset(s.anchorX, w);
  }
  if (y) {
    *y = s.y - anchorOffset(s.anchorY, h);
  }
}

}  // namespace

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

void SpriteLayer::spriteBounds(const Sprite& s, int* x0, int* y0, int* x1, int* y1) {
  int scaledW = scaledWidth(s);
  int scaledH = scaledHeight(s);
  int left = 0;
  int top = 0;
  spriteTopLeft(s, &left, &top);
  int right = left + scaledW - 1;
  int bottom = top + scaledH - 1;

  if (x0) *x0 = left;
  if (y0) *y0 = top;
  if (x1) *x1 = right;
  if (y1) *y1 = bottom;
}

void SpriteLayer::spriteBounds(const Sprite& s, int16_t* x0, int16_t* y0, int16_t* x1, int16_t* y1) {
  int lx = 0, ly = 0, rx = 0, ry = 0;
  spriteBounds(s, &lx, &ly, &rx, &ry);
  if (x0) *x0 = (int16_t)lx;
  if (y0) *y0 = (int16_t)ly;
  if (x1) *x1 = (int16_t)rx;
  if (y1) *y1 = (int16_t)ry;
}

void SpriteLayer::spriteBoundsPadded(const Sprite& s, int pad, int* x0, int* y0, int* x1, int* y1) {
  int left = 0;
  int top = 0;
  int right = 0;
  int bottom = 0;
  spriteBounds(s, &left, &top, &right, &bottom);
  if (x0) *x0 = left - pad;
  if (y0) *y0 = top - pad;
  if (x1) *x1 = right + pad;
  if (y1) *y1 = bottom + pad;
}

void SpriteLayer::renderRegion(int x0, int y0, int w, int h, uint16_t* buf) const {
  if (!buf || w <= 0 || h <= 0) return;

  auto blitMissile = [&](const Missile& m) {
    if (!m.active || m.h <= 0 || m.w <= 0) return;
    int mx0 = m.x;
    int my0 = m.y;
    int scaledW = (doublesX(m.scale) ? (m.w * 2) : m.w);
    int scaledH = (doublesY(m.scale) ? (m.h * 2) : m.h);
    int mx1 = m.x + scaledW - 1;
    int my1 = m.y + scaledH - 1;
    if (mx1 < x0 || mx0 >= x0 + w || my1 < y0 || my0 >= y0 + h) return;

    int rx0 = (mx0 < x0) ? x0 : mx0;
    int ry0 = (my0 < y0) ? y0 : my0;
    int rx1 = (mx1 > x0 + w - 1) ? (x0 + w - 1) : mx1;
    int ry1 = (my1 > y0 + h - 1) ? (y0 + h - 1) : my1;

    for (int yy = ry0; yy <= ry1; ++yy) {
      for (int xx = rx0; xx <= rx1; ++xx) {
        int sx = xx - mx0;
        if (doublesX(m.scale)) sx /= 2;
        if (sx < 0 || sx >= m.w) continue;

        int bufX = xx - x0;
        int bufY = yy - y0;
        buf[bufY * w + bufX] = m.color;
      }
    }
  };

  auto blitSprite = [&](const Sprite& s) {
    if (!s.active || !s.pixels565 || s.w <= 0 || s.h <= 0) return;
    int sx0 = 0;
    int sy0 = 0;
    int sx1 = 0;
    int sy1 = 0;
    spriteBounds(s, &sx0, &sy0, &sx1, &sy1);
    if (sx1 < x0 || sx0 >= x0 + w || sy1 < y0 || sy0 >= y0 + h) return;

    int rx0 = (sx0 < x0) ? x0 : sx0;
    int ry0 = (sy0 < y0) ? y0 : sy0;
    int rx1 = (sx1 > x0 + w - 1) ? (x0 + w - 1) : sx1;
    int ry1 = (sy1 > y0 + h - 1) ? (y0 + h - 1) : sy1;

    for (int yy = ry0; yy <= ry1; ++yy) {
      int srcY = yy - sy0;
      if (doublesY(s.scale)) srcY /= 2;
      for (int xx = rx0; xx <= rx1; ++xx) {
        int srcX = xx - sx0;
        if (doublesX(s.scale)) srcX /= 2;
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
