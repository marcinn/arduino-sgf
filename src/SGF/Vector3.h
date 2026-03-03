#pragma once

struct Vector3f {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vector3f() = default;
    Vector3f(float xValue, float yValue, float zValue)
        : x(xValue), y(yValue), z(zValue) {}
};
