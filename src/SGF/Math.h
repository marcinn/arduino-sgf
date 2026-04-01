#pragma once

#include "Vector3.h"

namespace Math {

int clamp(int value, int minValue, int maxValue);
float clamp(float value, float minValue, float maxValue);

// Wraps into [minValue, maxValueExclusive).
int wrap(int value, int minValue, int maxValueExclusive);
float wrap(float value, float minValue, float maxValueExclusive);

float dot(const Vector3f& a, const Vector3f& b);
Vector3f cross(const Vector3f& a, const Vector3f& b);
float lengthSq(const Vector3f& value);
float length(const Vector3f& value);
Vector3f normalize(const Vector3f& value);
Vector3f projectOntoPlane(const Vector3f& value, const Vector3f& planeNormal);

}  // namespace Math
