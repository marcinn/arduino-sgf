#pragma once

#include "ColliderCollision.h"
#include "ICollider.h"
#include "RigidBody.h"
#include "Vector2.h"

#include <stdint.h>

class BitTileCollider : public ICollider {
   public:
    BitTileCollider();
    BitTileCollider(const uint8_t* tiles, const Vector2i& tileCount,
                    const Vector2i& tileSize, const Vector2f& minBounds,
                    const Vector2f& maxBounds,
                    float restitution = 1.0f);
    BitTileCollider(const uint8_t* tiles, const Vector2i& tileCount,
                    const Vector2i& tileSize, const Vector2i& minBounds,
                    const Vector2i& maxBounds,
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
    void setRestitution(float restitutionValue);

    bool isColliding(const ICollidable& collidable) const override;
    ColliderCollision getCollision(const ICollidable& collidable) const override;
    void resolve(RigidBody& body) const override;

   private:
    const uint8_t* tileBuffer = nullptr;
    Vector2i tileMapCount{};
    Vector2i tileScreenSize{1, 1};
    Vector2f minCollisionBounds{};
    Vector2f maxCollisionBounds{};
    float collisionRestitution = 1.0f;
};
