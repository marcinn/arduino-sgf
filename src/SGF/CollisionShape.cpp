#include "CollisionShape.h"

CollisionShape::CollisionShape() = default;

CollisionShape::CollisionShape(CollisionShapeType typeValue) : collisionShapeType(typeValue) {}

CollisionShape::CollisionShape(CollisionShapeType typeValue, const Vector2i& sizeValue,
                               int radiusValue)
    : collisionShapeType(typeValue), collisionShapeSize(sizeValue),
      collisionShapeRadius(radiusValue) {}

CollisionShape CollisionShape::point() { return CollisionShape(CollisionShapeType::Point); }

CollisionShape CollisionShape::rect(const Vector2i& size) {
    return CollisionShape(CollisionShapeType::Rect, size, 0);
}

CollisionShape CollisionShape::circle(int radius) {
    return CollisionShape(CollisionShapeType::Circle, Vector2i{}, radius);
}

CollisionShapeType CollisionShape::type() const { return collisionShapeType; }

Vector2i CollisionShape::size() const { return collisionShapeSize; }

int CollisionShape::radius() const { return collisionShapeRadius; }

void CollisionShape::setType(CollisionShapeType typeValue) { collisionShapeType = typeValue; }

void CollisionShape::setSize(const Vector2i& sizeValue) { collisionShapeSize = sizeValue; }

void CollisionShape::setRadius(int radiusValue) {
    if (radiusValue < 0) {
        collisionShapeRadius = 0;
    } else {
        collisionShapeRadius = radiusValue;
    }
}
