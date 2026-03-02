#include "SGF/Vector2.h"

Vector2i::Vector2i(const Vector2f& value)
    : x((int)(value.x >= 0.0f ? value.x + 0.5f : value.x - 0.5f)),
      y((int)(value.y >= 0.0f ? value.y + 0.5f : value.y - 0.5f)) {}

Vector2f::operator Vector2i() const {
    return Vector2i{
        (int)(x >= 0.0f ? x + 0.5f : x - 0.5f),
        (int)(y >= 0.0f ? y + 0.5f : y - 0.5f),
    };
}
