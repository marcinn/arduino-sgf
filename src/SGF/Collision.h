#pragma once

#include "Vector2.h"

bool circleRectHit(const Vector2i& center, int radius, const Vector2i& rectMin,
                   const Vector2i& rectMax);
bool aabbHit(const Vector2i& aMin, const Vector2i& aMax, const Vector2i& bMin,
             const Vector2i& bMax);
bool circleCircleHit(const Vector2i& aCenter, int aRadius, const Vector2i& bCenter, int bRadius);
bool pointInRect(const Vector2i& point, const Vector2i& rectMin, const Vector2i& rectMax);
bool raycastToRect(const Vector2i& origin, const Vector2i& delta, const Vector2i& rectMin,
                   const Vector2i& rectMax, float* tHit);
