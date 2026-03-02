#include "Physics.h"

#include <math.h>

float Physics::gravityOverride = 0.0f;
bool Physics::hasGravityOverride = false;

void Physics::integrate(RigidBody& body, float delta) {
    Vector2f velocity = body.getVelocity();
    velocity += (body.accumulatedForce / body.getMass()) * delta;
    velocity.y += gravity() * delta;
    if (body.getLinearDamp() > 0.0f) {
        float dampFactor = 1.0f - (body.getLinearDamp() * delta);
        if (dampFactor < 0.0f) {
            dampFactor = 0.0f;
        }
        velocity *= dampFactor;
    }
    if (body.isOnFloor() && body.getFloorDamp() > 0.0f) {
        float dampFactor = 1.0f - (body.getFloorDamp() * delta);
        if (dampFactor < 0.0f) {
            dampFactor = 0.0f;
        }
        velocity *= dampFactor;
    }
    body.setVelocity(velocity);
    body.setPosition(body.getPosition() + velocity * delta);
    body.setOnFloor(false);
    body.accumulatedForce = Vector2f{};
}

void Physics::bounce(RigidBody& body, const Vector2f& normal, float restitution) {
    float normalLengthSq = normal.x * normal.x + normal.y * normal.y;
    if (normalLengthSq <= 0.0f) {
        return;
    }

    Vector2f velocity = body.getVelocity();
    float normalLength = sqrtf(normalLengthSq);
    Vector2f unitNormal = normal / normalLength;
    float normalVelocity = velocity.x * unitNormal.x + velocity.y * unitNormal.y;
    if (normalVelocity >= 0.0f) {
        return;
    }

    body.setVelocity(velocity - ((1.0f + restitution) * normalVelocity * unitNormal));
}

void Physics::resolveBodies(RigidBody& first, RigidBody& second, float restitution) {
    ColliderCollision collision = collidablesCollision(first, second);
    if (!collision.isColliding()) {
        return;
    }

    Vector2f normal = collision.normal();
    float normalLengthSq = normal.x * normal.x + normal.y * normal.y;
    if (normalLengthSq <= 0.0f) {
        return;
    }

    float normalLength = sqrtf(normalLengthSq);
    Vector2f unitNormal = normal / normalLength;
    Vector2f relativeVelocity = first.getVelocity() - second.getVelocity();
    float normalVelocity = relativeVelocity.x * unitNormal.x + relativeVelocity.y * unitNormal.y;
    if (normalVelocity >= 0.0f) {
        return;
    }

    float inverseMassSum = (1.0f / first.getMass()) + (1.0f / second.getMass());
    if (inverseMassSum <= 0.0f) {
        return;
    }

    float impulseMagnitude = -(1.0f + restitution) * normalVelocity / inverseMassSum;
    Vector2f impulse = impulseMagnitude * unitNormal;
    first.setVelocity(first.getVelocity() + (impulse / first.getMass()));
    second.setVelocity(second.getVelocity() - (impulse / second.getMass()));
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
