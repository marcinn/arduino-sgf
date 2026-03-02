#include "CollisionState.h"

CollisionState::CollisionState() = default;

bool CollisionState::isColliding() const { return colliding; }

bool CollisionState::isJustCollided() const { return justCollided; }

bool CollisionState::isJustSeparated() const { return justSeparated; }

const ColliderCollision& CollisionState::collision() const { return currentCollision; }

void CollisionState::reset() {
    colliding = false;
    justCollided = false;
    justSeparated = false;
    currentCollision = ColliderCollision{};
}

void CollisionState::update(bool collidingValue, const ColliderCollision& collisionValue) {
    justCollided = !colliding && collidingValue;
    justSeparated = colliding && !collidingValue;
    colliding = collidingValue;
    if (collidingValue) {
        currentCollision = collisionValue;
        return;
    }
    if (justSeparated) {
        currentCollision = collisionValue;
        return;
    }
    currentCollision = ColliderCollision{};
}
