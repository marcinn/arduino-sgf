#pragma once

#include "ColliderCollision.h"

class CollisionState {
   public:
    CollisionState();

    bool isColliding() const;
    bool isJustCollided() const;
    bool isJustSeparated() const;
    const ColliderCollision& collision() const;

    void reset();
    void update(bool collidingValue, const ColliderCollision& collisionValue);

   private:
    bool colliding = false;
    bool justCollided = false;
    bool justSeparated = false;
    ColliderCollision currentCollision{};
};
