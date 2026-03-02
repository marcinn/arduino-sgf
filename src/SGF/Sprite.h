#pragma once

#include <stdint.h>

#include "Vector2.h"

enum class SpriteScale {
    Normal,
    DoubleX,
    DoubleY,
    Double,
    QuadX,
    QuadY,
    Quad,
};

class Sprite {
   public:
    bool isActive() const;
    Vector2i position() const;
    int width() const;
    int height() const;
    const uint16_t* pixels() const;
    uint16_t transparentColor() const;
    SpriteScale scale() const;
    Vector2f anchor() const;

    void setAnchor(const Vector2f& newAnchor);
    void setScale(SpriteScale newScale);
    void setBitmap(const uint16_t* pixels, int width, int height);
    void setBitmap(const uint16_t* pixels, int width, int height, uint16_t transparentColor);
    void setTransparent(uint16_t transparentColor);
    void setActive(bool enabled);
    void setPosition(const Vector2i& newPosition);
    void translate(const Vector2i& delta);
    void redraw();
    uint32_t redrawRevision() const;

   private:
    bool active = false;
    Vector2i spritePosition{};
    int spriteWidth = 0;
    int spriteHeight = 0;
    const uint16_t* spritePixels = nullptr;
    uint16_t transparent = 0;
    SpriteScale spriteScale = SpriteScale::Normal;
    Vector2f spriteAnchor{};
    uint32_t redrawRevision_ = 1;
};
