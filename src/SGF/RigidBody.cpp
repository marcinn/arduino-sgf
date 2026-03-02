#include "RigidBody.h"

Vector2f RigidBody::getPosition() const { return position; }

void RigidBody::setPosition(const Vector2i& position) {
    setPosition(Vector2f{position});
}

void RigidBody::setPosition(const Vector2f& position) {
    this->position = position;
}

Vector2f RigidBody::getVelocity() const { return velocity; }

void RigidBody::setVelocity(const Vector2i& velocity) {
    setVelocity(Vector2f{velocity});
}

void RigidBody::setVelocity(const Vector2f& velocity) {
    this->velocity = velocity;
    onFloor = false;
}

float RigidBody::getMass() const { return mass; }

void RigidBody::setMass(float massValue) {
    mass = massValue > 0.0f ? massValue : 1.0f;
}

void RigidBody::applyCentralForce(const Vector2i& force) {
    applyCentralForce(Vector2f{force});
}

void RigidBody::applyCentralForce(const Vector2f& force) {
    applyForce(force, Vector2f{});
}

void RigidBody::applyCentralImpulse(const Vector2i& impulse) {
    applyCentralImpulse(Vector2f{impulse});
}

void RigidBody::applyCentralImpulse(const Vector2f& impulse) {
    applyImpulse(impulse, Vector2f{});
}

void RigidBody::applyForce(const Vector2i& force, const Vector2i& position) {
    applyForce(Vector2f{force}, Vector2f{position});
}

void RigidBody::applyForce(const Vector2f& force, const Vector2f& position) {
    (void)position;
    accumulatedForce += force;
}

void RigidBody::applyImpulse(const Vector2i& impulse, const Vector2i& position) {
    applyImpulse(Vector2f{impulse}, Vector2f{position});
}

void RigidBody::applyImpulse(const Vector2f& impulse, const Vector2f& position) {
    (void)position;
    velocity += impulse / mass;
    onFloor = false;
}

bool RigidBody::isOnFloor() const { return onFloor; }

void RigidBody::setOnFloor(bool onFloorValue) { onFloor = onFloorValue; }
