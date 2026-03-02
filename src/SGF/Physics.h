#pragma once

#ifndef PIXELS_PER_METER
#define PIXELS_PER_METER 48.0f
#endif

#ifndef GRAVITY
#define GRAVITY (9.81f * PIXELS_PER_METER)
#endif

#include "RigidBody.h"

class Physics {
   public:
    static void integrate(RigidBody& body, float delta);
    static void bounce(RigidBody& body, const Vector2f& normal, float restitution);
    static void setGravity(float gravityValue);
    static void clearGravityOverride();

   private:
    static float gravity();
    static float gravityOverride;
    static bool hasGravityOverride;
};
