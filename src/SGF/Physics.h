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
    static void bounceWithinX(RigidBody& body, int minX, int maxX, float bounce);
    static void bounceOnCeiling(RigidBody& body, int minY, float bounce);
    static void bounceOnFloor(RigidBody& body, int maxY, float bounce, float settleSpeed = 0.0f);
    static void applyHorizontalDrag(RigidBody& body, float dragPerSec, float delta,
                                    float stopSpeed = 0.0f);
    static void setGravity(float gravityValue);
    static void clearGravityOverride();

   private:
    static float gravity();
    static float gravityOverride;
    static bool hasGravityOverride;
};
