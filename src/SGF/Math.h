#pragma once

namespace Math {

int clamp(int value, int minValue, int maxValue);
float clamp(float value, float minValue, float maxValue);

// Wraps into [minValue, maxValueExclusive).
int wrap(int value, int minValue, int maxValueExclusive);
float wrap(float value, float minValue, float maxValueExclusive);

}  // namespace Math
