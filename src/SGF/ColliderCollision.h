#pragma once

#include "Vector2.h"

#include <stdint.h>

class ColliderCollision {
   public:
    ColliderCollision();

    bool isColliding() const;
    void setColliding(bool collidingValue);

    Vector2f position() const;
    void setPosition(const Vector2i& positionValue);
    void setPosition(const Vector2f& positionValue);

    Vector2f normal() const;
    void setNormal(const Vector2i& normalValue);
    void setNormal(const Vector2f& normalValue);

    int tileX() const;
    void setTileX(int tileXValue);

    int tileY() const;
    void setTileY(int tileYValue);

    int tileIndex() const;
    void setTileIndex(int tileIndexValue);

    int bufferIndex() const;
    void setBufferIndex(int bufferIndexValue);

    int bitIndex() const;
    void setBitIndex(int bitIndexValue);

    uint8_t tileValue() const;
    void setTileValue(uint8_t tileValueValue);

   private:
    bool colliding = false;
    Vector2f collisionPosition{};
    Vector2f collisionNormal{};
    int collisionTileX = -1;
    int collisionTileY = -1;
    int collisionTileIndex = -1;
    int collisionBufferIndex = -1;
    int collisionBitIndex = -1;
    uint8_t collisionTileValue = 0;
};
