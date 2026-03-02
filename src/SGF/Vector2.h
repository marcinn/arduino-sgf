#pragma once

struct Vector2f;

struct Vector2i {
    int x = 0;
    int y = 0;

    Vector2i() = default;
    Vector2i(int xValue, int yValue) : x(xValue), y(yValue) {}
    explicit Vector2i(const Vector2f& value);
};

struct Vector2f {
    float x = 0.0f;
    float y = 0.0f;

    Vector2f() = default;
    Vector2f(float xValue, float yValue) : x(xValue), y(yValue) {}
    Vector2f(const Vector2i& value) : x((float)value.x), y((float)value.y) {}
    explicit operator Vector2i() const;
};
