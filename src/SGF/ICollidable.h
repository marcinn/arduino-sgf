#pragma once

#include "CollisionShape.h"
#include "Vector2.h"

enum class CollisionBodyType {
    Static,
    Character,
    Rigid,
};

class ICollidable {
   public:
    virtual ~ICollidable();

    virtual CollisionBodyType collisionBodyType() const = 0;
    virtual Vector2f getCollisionPosition() const = 0;
    virtual CollisionShape getCollisionShape() const = 0;
};
