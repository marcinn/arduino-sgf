#pragma once

#include "SGF/Character.h"

class Physics;

class RigidBody : public Character {
   public:
    Vector2f getPhysicsPosition() const;
    void setPhysicsPosition(const Vector2i& position);
    void setPhysicsPosition(const Vector2f& position);

    Vector2f getVelocity() const;
    void setVelocity(const Vector2i& velocity);
    void setVelocity(const Vector2f& velocity);

    void applyForce(const Vector2i& force);
    void applyForce(const Vector2f& force);

    void setGravity(float gravity);
    float gravity() const;

    bool isOnFloor() const;

   protected:
    void didSetPosition() override;

   private:
    friend class Physics;

    void syncPosition();

    Vector2f physicsPosition{};
    Vector2f velocity{};
    float gravityY = 0.0f;
    bool onFloor = false;
    bool syncingPosition = false;
};
