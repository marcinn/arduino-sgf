#pragma once

#include "CollisionShape.h"
#include "ICollidable.h"
#include "Vector2.h"

class CharacterBody : public ICollidable {
   public:
    using Position = Vector2i;

    virtual ~CharacterBody();

    Vector2i getPosition() const;
    virtual void setPosition(const Position& pos);
    virtual void setPosition(int newX, int newY);
    Vector2f anchor() const;
    virtual void setAnchor(const Vector2f& newAnchor);

    Vector2f getCollisionPosition() const override;
    CollisionShape getCollisionShape() const override;
    void setCollisionShape(const CollisionShape& collisionShapeValue);

   private:
    int posX = 0;
    int posY = 0;
    Vector2f bodyAnchor{};
    CollisionShape collisionShape{};
};
