#pragma once

#include "Vector2.h"

enum class CollisionShapeType {
    Point,
    Rect,
    Circle,
};

class CollisionShape {
   public:
    CollisionShape();
    explicit CollisionShape(CollisionShapeType typeValue);
    CollisionShape(CollisionShapeType typeValue, const Vector2i& sizeValue, int radiusValue);

    static CollisionShape point();
    static CollisionShape rect(const Vector2i& size);
    static CollisionShape circle(int radius);

    CollisionShapeType type() const;
    Vector2i size() const;
    int radius() const;

    void setType(CollisionShapeType typeValue);
    void setSize(const Vector2i& sizeValue);
    void setRadius(int radiusValue);

   private:
    CollisionShapeType collisionShapeType = CollisionShapeType::Point;
    Vector2i collisionShapeSize{};
    int collisionShapeRadius = 0;
};
