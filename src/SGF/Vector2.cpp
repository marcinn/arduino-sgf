#include "Vector2.h"

Vector2i::Vector2i(const Vector2f& value)
    : x((int)(value.x >= 0.0f ? value.x + 0.5f : value.x - 0.5f)),
      y((int)(value.y >= 0.0f ? value.y + 0.5f : value.y - 0.5f)) {}

Vector2i& Vector2i::operator+=(const Vector2i& other) {
    x += other.x;
    y += other.y;
    return *this;
}

Vector2i& Vector2i::operator-=(const Vector2i& other) {
    x -= other.x;
    y -= other.y;
    return *this;
}

Vector2i& Vector2i::operator*=(const Vector2i& other) {
    x *= other.x;
    y *= other.y;
    return *this;
}

Vector2i& Vector2i::operator/=(const Vector2i& other) {
    x /= other.x;
    y /= other.y;
    return *this;
}

Vector2i& Vector2i::operator*=(int scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
}

Vector2i& Vector2i::operator/=(int scalar) {
    x /= scalar;
    y /= scalar;
    return *this;
}

Vector2f::operator Vector2i() const {
    return Vector2i{
        (int)(x >= 0.0f ? x + 0.5f : x - 0.5f),
        (int)(y >= 0.0f ? y + 0.5f : y - 0.5f),
    };
}

Vector2f& Vector2f::operator+=(const Vector2f& other) {
    x += other.x;
    y += other.y;
    return *this;
}

Vector2f& Vector2f::operator-=(const Vector2f& other) {
    x -= other.x;
    y -= other.y;
    return *this;
}

Vector2f& Vector2f::operator*=(const Vector2f& other) {
    x *= other.x;
    y *= other.y;
    return *this;
}

Vector2f& Vector2f::operator/=(const Vector2f& other) {
    x /= other.x;
    y /= other.y;
    return *this;
}

Vector2f& Vector2f::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
}

Vector2f& Vector2f::operator/=(float scalar) {
    x /= scalar;
    y /= scalar;
    return *this;
}

bool operator==(const Vector2i& left, const Vector2i& right) {
    return left.x == right.x && left.y == right.y;
}

bool operator!=(const Vector2i& left, const Vector2i& right) {
    return !(left == right);
}

Vector2i operator-(const Vector2i& value) { return Vector2i{-value.x, -value.y}; }

Vector2i operator+(Vector2i left, const Vector2i& right) {
    left += right;
    return left;
}

Vector2i operator-(Vector2i left, const Vector2i& right) {
    left -= right;
    return left;
}

Vector2i operator*(Vector2i left, const Vector2i& right) {
    left *= right;
    return left;
}

Vector2i operator/(Vector2i left, const Vector2i& right) {
    left /= right;
    return left;
}

Vector2i operator*(Vector2i vector, int scalar) {
    vector *= scalar;
    return vector;
}

Vector2i operator*(int scalar, Vector2i vector) {
    vector *= scalar;
    return vector;
}

Vector2i operator/(Vector2i vector, int scalar) {
    vector /= scalar;
    return vector;
}

bool operator==(const Vector2f& left, const Vector2f& right) {
    return left.x == right.x && left.y == right.y;
}

bool operator!=(const Vector2f& left, const Vector2f& right) {
    return !(left == right);
}

Vector2f operator-(const Vector2f& value) { return Vector2f{-value.x, -value.y}; }

Vector2f operator+(Vector2f left, const Vector2f& right) {
    left += right;
    return left;
}

Vector2f operator-(Vector2f left, const Vector2f& right) {
    left -= right;
    return left;
}

Vector2f operator*(Vector2f left, const Vector2f& right) {
    left *= right;
    return left;
}

Vector2f operator/(Vector2f left, const Vector2f& right) {
    left /= right;
    return left;
}

Vector2f operator*(Vector2f vector, float scalar) {
    vector *= scalar;
    return vector;
}

Vector2f operator*(float scalar, Vector2f vector) {
    vector *= scalar;
    return vector;
}

Vector2f operator/(Vector2f vector, float scalar) {
    vector /= scalar;
    return vector;
}
