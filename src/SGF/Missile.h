#pragma once

#include <stdint.h>

#include "Sprite.h"
#include "Vector2.h"

class Missile {
   public:
    bool isActive() const;
    Vector2i position() const;
    Vector2i size() const;
    uint16_t color() const;
    SpriteScale scale() const;

    void setActive(bool enabled);
    void setPosition(const Vector2i& newPosition);
    void setSize(const Vector2i& newSize);
    void setColor(uint16_t newColor);
    void setScale(SpriteScale newScale);

   private:
    bool active = false;
    Vector2i missilePosition{};
    Vector2i missileSize{1, 0};
    uint16_t missileColor = 0xFFFFu;
    SpriteScale missileScale = SpriteScale::Normal;
};
