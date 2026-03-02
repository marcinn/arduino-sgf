#pragma once

#include "Vector2.h"

class RigidBody {
   public:
    using Position = Vector2f;
    using Velocity = Vector2f;

    Vector2f getPosition() const;
    void setPosition(const Vector2i& position);
    virtual void setPosition(const Vector2f& position);

    Vector2f getVelocity() const;
    void setVelocity(const Vector2i& velocity);
    void setVelocity(const Vector2f& velocity);

    float getLinearDamp() const;
    void setLinearDamp(float linearDampValue);

    float getMass() const;
    void setMass(float massValue);

    void applyCentralForce(const Vector2i& force);
    void applyCentralForce(const Vector2f& force);
    void applyCentralImpulse(const Vector2i& impulse = Vector2i{});
    void applyCentralImpulse(const Vector2f& impulse = Vector2f{});
    void applyForce(const Vector2i& force, const Vector2i& position = Vector2i{});
    void applyForce(const Vector2f& force, const Vector2f& position = Vector2f{});
    void applyImpulse(const Vector2i& impulse, const Vector2i& position = Vector2i{});
    void applyImpulse(const Vector2f& impulse, const Vector2f& position = Vector2f{});

    bool isOnFloor() const;
    void setOnFloor(bool onFloorValue);

   private:
    friend class Physics;

    Vector2f position{};
    Vector2f velocity{};
    Vector2f accumulatedForce{};
    float linearDamp = 0.0f;
    float mass = 1.0f;
    bool onFloor = false;
};
