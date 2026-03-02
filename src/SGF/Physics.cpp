#include "Physics.h"

#include <math.h>

void Physics::integrate(RigidBody& body, float delta) {
    body.velocity.y += body.gravityY * delta;
    body.physicsPosition.x += body.velocity.x * delta;
    body.physicsPosition.y += body.velocity.y * delta;
    body.onFloor = false;
    body.syncPosition();
}

void Physics::bounceWithinX(RigidBody& body, int minX, int maxX, float bounce) {
    if (body.physicsPosition.x <= (float)minX) {
        body.physicsPosition.x = (float)minX;
        body.velocity.x = fabsf(body.velocity.x) * bounce;
        body.syncPosition();
    } else if (body.physicsPosition.x >= (float)maxX) {
        body.physicsPosition.x = (float)maxX;
        body.velocity.x = -fabsf(body.velocity.x) * bounce;
        body.syncPosition();
    }
}

void Physics::bounceOnCeiling(RigidBody& body, int minY, float bounce) {
    if (body.physicsPosition.y <= (float)minY) {
        body.physicsPosition.y = (float)minY;
        body.velocity.y = fabsf(body.velocity.y) * bounce;
        body.syncPosition();
    }
}

void Physics::bounceOnFloor(RigidBody& body, int maxY, float bounce, float settleSpeed) {
    if (body.physicsPosition.y < (float)maxY) {
        return;
    }

    body.physicsPosition.y = (float)maxY;
    body.velocity.y = -fabsf(body.velocity.y) * bounce;
    if (settleSpeed > 0.0f && fabsf(body.velocity.y) <= settleSpeed) {
        body.velocity.y = 0.0f;
        body.onFloor = true;
    } else {
        body.onFloor = false;
    }
    body.syncPosition();
}

void Physics::applyHorizontalDrag(RigidBody& body, float dragPerSec, float delta, float stopSpeed) {
    float dragFactor = 1.0f - (dragPerSec * delta);
    if (dragFactor < 0.0f) {
        dragFactor = 0.0f;
    }
    body.velocity.x *= dragFactor;
    if (stopSpeed > 0.0f && fabsf(body.velocity.x) <= stopSpeed) {
        body.velocity.x = 0.0f;
    }
}
