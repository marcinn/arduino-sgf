#include "Physics.h"

#include <math.h>

void Physics::integrate(RigidBody& body, float delta) {
  body.velY += body.gravityY * delta;
  body.posXf += body.velX * delta;
  body.posYf += body.velY * delta;
  body.onFloor = false;
  body.syncPosition();
}

void Physics::bounceWithinX(RigidBody& body, int minX, int maxX, float bounce) {
  if (body.posXf <= (float)minX) {
    body.posXf = (float)minX;
    body.velX = fabsf(body.velX) * bounce;
    body.syncPosition();
  } else if (body.posXf >= (float)maxX) {
    body.posXf = (float)maxX;
    body.velX = -fabsf(body.velX) * bounce;
    body.syncPosition();
  }
}

void Physics::bounceOnCeiling(RigidBody& body, int minY, float bounce) {
  if (body.posYf <= (float)minY) {
    body.posYf = (float)minY;
    body.velY = fabsf(body.velY) * bounce;
    body.syncPosition();
  }
}

void Physics::bounceOnFloor(RigidBody& body, int maxY, float bounce, float settleSpeed) {
  if (body.posYf < (float)maxY) {
    return;
  }

  body.posYf = (float)maxY;
  body.velY = -fabsf(body.velY) * bounce;
  if (settleSpeed > 0.0f && fabsf(body.velY) <= settleSpeed) {
    body.velY = 0.0f;
    body.onFloor = true;
  } else {
    body.onFloor = false;
  }
  body.syncPosition();
}

void Physics::applyHorizontalDrag(
  RigidBody& body,
  float dragPerSec,
  float delta,
  float stopSpeed) {
  float dragFactor = 1.0f - (dragPerSec * delta);
  if (dragFactor < 0.0f) {
    dragFactor = 0.0f;
  }
  body.velX *= dragFactor;
  if (stopSpeed > 0.0f && fabsf(body.velX) <= stopSpeed) {
    body.velX = 0.0f;
  }
}
