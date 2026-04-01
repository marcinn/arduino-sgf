#pragma once

struct Vector2f;

struct Vector2i {
    int x = 0;
    int y = 0;

    Vector2i() = default;
    Vector2i(int xValue, int yValue) : x(xValue), y(yValue) {}
    explicit Vector2i(const Vector2f& value);

    Vector2i& operator+=(const Vector2i& other);
    Vector2i& operator-=(const Vector2i& other);
    Vector2i& operator*=(const Vector2i& other);
    Vector2i& operator/=(const Vector2i& other);
    Vector2i& operator*=(int scalar);
    Vector2i& operator/=(int scalar);
};

struct Vector2f {
    float x = 0.0f;
    float y = 0.0f;

    Vector2f() = default;
    Vector2f(float xValue, float yValue) : x(xValue), y(yValue) {}
    Vector2f(const Vector2i& value) : x((float)value.x), y((float)value.y) {}
    explicit operator Vector2i() const;

    Vector2f& operator+=(const Vector2f& other);
    Vector2f& operator-=(const Vector2f& other);
    Vector2f& operator*=(const Vector2f& other);
    Vector2f& operator/=(const Vector2f& other);
    Vector2f& operator*=(float scalar);
    Vector2f& operator/=(float scalar);
};

bool operator==(const Vector2i& left, const Vector2i& right);
bool operator!=(const Vector2i& left, const Vector2i& right);
Vector2i operator-(const Vector2i& value);
Vector2i operator+(Vector2i left, const Vector2i& right);
Vector2i operator-(Vector2i left, const Vector2i& right);
Vector2i operator*(Vector2i left, const Vector2i& right);
Vector2i operator/(Vector2i left, const Vector2i& right);
Vector2i operator*(Vector2i vector, int scalar);
Vector2i operator*(int scalar, Vector2i vector);
Vector2i operator/(Vector2i vector, int scalar);

bool operator==(const Vector2f& left, const Vector2f& right);
bool operator!=(const Vector2f& left, const Vector2f& right);
Vector2f operator-(const Vector2f& value);
Vector2f operator+(Vector2f left, const Vector2f& right);
Vector2f operator-(Vector2f left, const Vector2f& right);
Vector2f operator*(Vector2f left, const Vector2f& right);
Vector2f operator/(Vector2f left, const Vector2f& right);
Vector2f operator*(Vector2f vector, float scalar);
Vector2f operator*(float scalar, Vector2f vector);
Vector2f operator/(Vector2f vector, float scalar);
