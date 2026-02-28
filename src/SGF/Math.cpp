#include "SGF/Math.h"

namespace Math {

int clamp(int value, int minValue, int maxValue) {
  if (value < minValue) {
    return minValue;
  }
  if (value > maxValue) {
    return maxValue;
  }
  return value;
}

float clamp(float value, float minValue, float maxValue) {
  if (value < minValue) {
    return minValue;
  }
  if (value > maxValue) {
    return maxValue;
  }
  return value;
}

int wrap(int value, int minValue, int maxValueExclusive) {
  int span = maxValueExclusive - minValue;
  if (span <= 0) {
    return minValue;
  }

  int wrapped = (value - minValue) % span;
  if (wrapped < 0) {
    wrapped += span;
  }
  return minValue + wrapped;
}

float wrap(float value, float minValue, float maxValueExclusive) {
  float span = maxValueExclusive - minValue;
  if (span <= 0.0f) {
    return minValue;
  }

  while (value < minValue) {
    value += span;
  }
  while (value >= maxValueExclusive) {
    value -= span;
  }
  return value;
}

}  // namespace Math
