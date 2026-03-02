#pragma once

#include "SGF/Character.h"

class Physics;

class RigidBody : public Character {
   public:
    Vector2f getPhysicsPosition() const { return Vector2f{posXf, posYf}; }

    void setPhysicsPosition(const Vector2i& position) {
        setPhysicsPosition((float)position.x, (float)position.y);
    }

    void setPhysicsPosition(const Vector2f& position) { setPhysicsPosition(position.x, position.y); }

    void setPhysicsPosition(float x, float y) {
        posXf = x;
        posYf = y;
        syncPosition();
    }

    void setVelocity(const Vector2i& velocity) { setVelocity((float)velocity.x, (float)velocity.y); }

    void setVelocity(const Vector2f& velocity) { setVelocity(velocity.x, velocity.y); }

    void setVelocity(float newVelX, float newVelY) {
        velX = newVelX;
        velY = newVelY;
        onFloor = false;
    }

    float physicsX() const { return posXf; }

    float physicsY() const { return posYf; }

    float velocityX() const { return velX; }

    float velocityY() const { return velY; }

    Vector2f getVelocity() const { return Vector2f{velX, velY}; }

    void setVelocityX(float newVelX) {
        velX = newVelX;
        onFloor = false;
    }

    void setVelocityY(float newVelY) {
        velY = newVelY;
        onFloor = false;
    }

    void applyForce(float forceX, float forceY) {
        velX += forceX;
        velY += forceY;
        onFloor = false;
    }

    void applyForce(const Vector2i& force) { applyForce((float)force.x, (float)force.y); }

    void applyForce(const Vector2f& force) { applyForce(force.x, force.y); }

    void setGravity(float gravity) { gravityY = gravity; }

    float gravity() const { return gravityY; }

    bool isOnFloor() const { return onFloor; }

   protected:
    void didSetPosition() override {
        if (syncingPosition) {
            return;
        }
        posXf = (float)positionX();
        posYf = (float)positionY();
    }

   private:
    friend class Physics;

    void syncPosition() {
        syncingPosition = true;
        Character::setPosition((int)(posXf + 0.5f), (int)(posYf + 0.5f));
        syncingPosition = false;
    }

    float posXf = 0.0f;
    float posYf = 0.0f;
    float velX = 0.0f;
    float velY = 0.0f;
    float gravityY = 0.0f;
    bool onFloor = false;
    bool syncingPosition = false;
};
