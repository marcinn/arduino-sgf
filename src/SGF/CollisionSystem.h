#pragma once

#include <stddef.h>

#include "CollisionBinding.h"
#include "ICollider.h"

class CollisionSystem {
   public:
    CollisionSystem();

    void configure(const CollisionBinding* bindings, size_t count);
    void configureBodies(ICollidable* const* bodies, size_t count);
    void configureColliders(ICollider* const* colliders, size_t count);
    void reset();
    void update(float delta);

    const CollisionBinding* bindings() const;
    size_t bindingCount() const;

   private:
    ICollidable* const* bodies = nullptr;
    size_t bodyCount = 0;
    ICollider* const* colliders = nullptr;
    size_t colliderCount = 0;
    const CollisionBinding* collisionBindings = nullptr;
    size_t collisionBindingCount = 0;
};
