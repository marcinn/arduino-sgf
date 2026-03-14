#pragma once

#include <stdint.h>

#include <array>

#include "Missile.h"
#include "Sprite.h"

class SpriteLayer {
   public:
    static constexpr int MAX_SPRITES = 8;
    static constexpr int MAX_MISSILES = 4;

    SpriteLayer();

    void clearSprites();
    void clearMissiles();
    void clearAll();

    void renderRegion(int x0, int y0, int w, int h, uint16_t* buf) const;

   private:
    friend class Renderer2D;

    Sprite& sprite(int index);
    Missile& missile(int index);
    static Vector2i scaleFactor(SpriteScale scale);
    static Vector2i scaledSize(const Sprite& sprite);
    static Vector2i anchorOffset(const Vector2f& anchor, const Vector2i& size);
    static Vector2i spriteTopLeft(const Sprite& sprite);
    static void spriteBounds(const Sprite& s, int* x0, int* y0, int* x1, int* y1);
    static void spriteBounds(const Sprite& s, int16_t* x0, int16_t* y0, int16_t* x1, int16_t* y1);
    static void spriteBoundsPadded(const Sprite& s, int pad, int* x0, int* y0, int* x1, int* y1);

    std::array<Sprite, MAX_SPRITES> sprites_{};
    std::array<Missile, MAX_MISSILES> missiles_{};
};
