#include "CollisionSystem.h"

#include "Collision.h"
#include "Physics.h"

CollisionSystem::CollisionSystem() = default;

void CollisionSystem::configure(const CollisionBinding* bindings, size_t count) {
    collisionBindings = bindings;
    collisionBindingCount = count;
    reset();
}

void CollisionSystem::configureBodies(ICollidable* const* bodies, size_t count) {
    this->bodies = bodies;
    bodyCount = count;
    for (size_t i = 0; i < colliderCount; ++i) {
        colliders[i]->setBodies(bodies, count);
    }
}

void CollisionSystem::configureColliders(ICollider* const* colliders, size_t count) {
    this->colliders = colliders;
    colliderCount = count;
    for (size_t i = 0; i < colliderCount; ++i) {
        colliders[i]->setBodies(bodies, bodyCount);
    }
}

void CollisionSystem::reset() {
    for (size_t i = 0; i < collisionBindingCount; ++i) {
        collisionBindings[i].state().reset();
    }
}

void CollisionSystem::update(float delta) {
    for (size_t i = 0; i < bodyCount; ++i) {
        if (bodies[i]->collisionBodyType() == CollisionBodyType::Rigid) {
            Physics::integrate(*(RigidBody*)bodies[i], delta);
        }
    }

    for (size_t i = 0; i < colliderCount; ++i) {
        colliders[i]->resolveConfiguredBodies();
        for (size_t j = 0; j < bodyCount; ++j) {
            if (bodies[j]->collisionBodyType() != CollisionBodyType::Rigid) {
                continue;
            }
            colliders[i]->resolve(*(RigidBody*)bodies[j]);
        }
    }

    for (size_t i = 0; i < collisionBindingCount; ++i) {
        const CollisionBinding& binding = collisionBindings[i];
        ColliderCollision collision = collidablesCollision(binding.first(), binding.second());
        binding.state().update(collision.isColliding(), collision);
    }
}

const CollisionBinding* CollisionSystem::bindings() const { return collisionBindings; }

size_t CollisionSystem::bindingCount() const { return collisionBindingCount; }
