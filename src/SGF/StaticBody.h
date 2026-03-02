#pragma once

#include "CollisionShape.h"
#include "ICollidable.h"
#include "Vector2.h"

class StaticBody : public ICollidable {
   public:
    CollisionBodyType collisionBodyType() const override;

    Vector2f getPosition() const;
    void setPosition(const Vector2i& position);
    void setPosition(const Vector2f& position);
    Vector2f anchor() const;
    void setAnchor(const Vector2f& newAnchor);

    Vector2f getCollisionPosition() const override;
    CollisionShape getCollisionShape() const override;
    void setCollisionShape(const CollisionShape& collisionShapeValue);

   private:
    Vector2f position{};
    Vector2f bodyAnchor{};
    CollisionShape collisionShape{};
};
