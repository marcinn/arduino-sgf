#include "Math.h"

#include <math.h>

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

float dot(const Vector3f& a, const Vector3f& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector3f cross(const Vector3f& a, const Vector3f& b) {
    return Vector3f{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}

float lengthSq(const Vector3f& value) {
    return dot(value, value);
}

float length(const Vector3f& value) {
    return sqrtf(lengthSq(value));
}

Vector3f normalize(const Vector3f& value) {
    float valueLength = length(value);
    if (valueLength <= 0.0f) {
        return Vector3f{};
    }
    return Vector3f{value.x / valueLength, value.y / valueLength, value.z / valueLength};
}

Vector3f projectOntoPlane(const Vector3f& value, const Vector3f& planeNormal) {
    Vector3f unitNormal = normalize(planeNormal);
    return Vector3f{
        value.x - unitNormal.x * dot(value, unitNormal),
        value.y - unitNormal.y * dot(value, unitNormal),
        value.z - unitNormal.z * dot(value, unitNormal),
    };
}

}  // namespace Math
