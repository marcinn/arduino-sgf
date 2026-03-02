#include "Physics.h"

#include <math.h>

float Physics::gravityOverride = 0.0f;
bool Physics::hasGravityOverride = false;

void Physics::integrate(RigidBody& body, float delta) {
    Vector2f velocity = body.getVelocity();
    velocity += (body.accumulatedForce / body.getMass()) * delta;
    velocity.y += gravity() * delta;
    body.setVelocity(velocity);
    body.setPosition(body.getPosition() + velocity * delta);
    body.setOnFloor(false);
    body.accumulatedForce = Vector2f{};
}

void Physics::bounceWithinX(RigidBody& body, int minX, int maxX, float bounce) {
    Vector2f position = body.getPosition();
    Vector2f velocity = body.getVelocity();

    if (position.x <= (float)minX) {
        position.x = (float)minX;
        velocity.x = fabsf(velocity.x) * bounce;
        body.setPosition(position);
        body.setVelocity(velocity);
    } else if (position.x >= (float)maxX) {
        position.x = (float)maxX;
        velocity.x = -fabsf(velocity.x) * bounce;
        body.setPosition(position);
        body.setVelocity(velocity);
    }
}

void Physics::bounceOnCeiling(RigidBody& body, int minY, float bounce) {
    Vector2f position = body.getPosition();
    Vector2f velocity = body.getVelocity();

    if (position.y <= (float)minY) {
        position.y = (float)minY;
        velocity.y = fabsf(velocity.y) * bounce;
        body.setPosition(position);
        body.setVelocity(velocity);
    }
}

void Physics::bounceOnFloor(RigidBody& body, int maxY, float bounce, float settleSpeed) {
    Vector2f position = body.getPosition();
    Vector2f velocity = body.getVelocity();

    if (position.y < (float)maxY) {
        return;
    }

    position.y = (float)maxY;
    velocity.y = -fabsf(velocity.y) * bounce;
    if (settleSpeed > 0.0f && fabsf(velocity.y) <= settleSpeed) {
        velocity.y = 0.0f;
    }
    body.setPosition(position);
    body.setVelocity(velocity);
    body.setOnFloor(settleSpeed > 0.0f && velocity.y == 0.0f);
}

void Physics::applyHorizontalDrag(RigidBody& body, float dragPerSec, float delta, float stopSpeed) {
    Vector2f velocity = body.getVelocity();
    float dragFactor = 1.0f - (dragPerSec * delta);
    if (dragFactor < 0.0f) {
        dragFactor = 0.0f;
    }

    velocity.x *= dragFactor;
    if (stopSpeed > 0.0f && fabsf(velocity.x) <= stopSpeed) {
        velocity.x = 0.0f;
    }
    body.setVelocity(velocity);
}

void Physics::setGravity(float gravityValue) {
    gravityOverride = gravityValue;
    hasGravityOverride = true;
}

void Physics::clearGravityOverride() { hasGravityOverride = false; }

float Physics::gravity() {
    if (hasGravityOverride) {
        return gravityOverride;
    }
    return (float)GRAVITY;
}
