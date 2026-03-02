#pragma once

#include "CollisionShape.h"
#include "Vector2.h"

class ICollidable {
   public:
    virtual ~ICollidable();

    virtual Vector2f getCollisionPosition() const = 0;
    virtual CollisionShape getCollisionShape() const = 0;
};
