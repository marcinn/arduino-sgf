#pragma once

#include "ColliderCollision.h"
#include "ICollider.h"
#include "RigidBody.h"
#include "Vector2.h"

#include <stdint.h>

class CharTileCollider : public ICollider {
   public:
    using CheckTileCollisionFn = bool (*)(uint8_t tile);

    CharTileCollider();
    CharTileCollider(const uint8_t* tiles, const Vector2i& tileCount,
                     const Vector2i& tileSize, const Vector2f& minBounds,
                     const Vector2f& maxBounds,
                     CheckTileCollisionFn checkTileCollisionFn,
                     float restitution = 1.0f);
    CharTileCollider(const uint8_t* tiles, const Vector2i& tileCount,
                     const Vector2i& tileSize, const Vector2i& minBounds,
                     const Vector2i& maxBounds,
                     CheckTileCollisionFn checkTileCollisionFn,
                     float restitution = 1.0f);

    const uint8_t* tiles() const;
    Vector2i tileCount() const;
    Vector2i tileSize() const;
    Vector2f minBounds() const;
    Vector2f maxBounds() const;
    float restitution() const;

    void setTiles(const uint8_t* tiles);
    void setTileCount(const Vector2i& tileCount);
    void setTileSize(const Vector2i& tileSize);
    void setBounds(const Vector2i& minBounds, const Vector2i& maxBounds);
    void setBounds(const Vector2f& minBounds, const Vector2f& maxBounds);
    void setCheckTileCollisionFn(CheckTileCollisionFn fn);
    void setRestitution(float restitutionValue);

    bool isColliding(const ICollidable& collidable) const override;
    ColliderCollision getCollision(const ICollidable& collidable) const override;
    void resolve(RigidBody& body) const override;

   private:
    static bool defaultCheckTileCollision(uint8_t tile);

    const uint8_t* tileBuffer = nullptr;
    Vector2i tileMapCount{};
    Vector2i tileScreenSize{1, 1};
    Vector2f minCollisionBounds{};
    Vector2f maxCollisionBounds{};
    CheckTileCollisionFn checkTileCollision = defaultCheckTileCollision;
    float collisionRestitution = 1.0f;
};
