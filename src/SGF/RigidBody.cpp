#include "RigidBody.h"

Vector2f RigidBody::getPhysicsPosition() const { return physicsPosition; }

void RigidBody::setPhysicsPosition(const Vector2i& position) {
    setPhysicsPosition(Vector2f{position});
}

void RigidBody::setPhysicsPosition(const Vector2f& position) {
    physicsPosition = position;
    syncPosition();
}

Vector2f RigidBody::getVelocity() const { return velocity; }

void RigidBody::setVelocity(const Vector2i& velocity) {
    setVelocity(Vector2f{velocity});
}

void RigidBody::setVelocity(const Vector2f& velocity) {
    this->velocity = velocity;
    onFloor = false;
}

void RigidBody::applyForce(const Vector2i& force) {
    applyForce(Vector2f{force});
}

void RigidBody::applyForce(const Vector2f& force) {
    velocity.x += force.x;
    velocity.y += force.y;
    onFloor = false;
}

void RigidBody::setGravity(float gravity) { gravityY = gravity; }

float RigidBody::gravity() const { return gravityY; }

bool RigidBody::isOnFloor() const { return onFloor; }

void RigidBody::didSetPosition() {
    if (syncingPosition) {
        return;
    }
    physicsPosition = Vector2f{getPosition()};
}

void RigidBody::syncPosition() {
    syncingPosition = true;
    Character::setPosition((Vector2i)physicsPosition);
    syncingPosition = false;
}
