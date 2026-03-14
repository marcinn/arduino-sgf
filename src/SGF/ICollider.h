#pragma once

#include "ColliderCollision.h"
#include "ICollidable.h"
#include "RigidBody.h"

#include <stddef.h>

class ICollider {
   public:
    virtual ~ICollider();

    virtual bool isColliding(const ICollidable& collidable) const = 0;
    virtual ColliderCollision getCollision(const ICollidable& collidable) const = 0;

    virtual void setBodies(ICollidable* const* bodies, size_t count);
    virtual void resolveConfiguredBodies() const;
    virtual void resolve(RigidBody& body) const = 0;
};
