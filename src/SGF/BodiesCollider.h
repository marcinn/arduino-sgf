#pragma once

#include "ICollider.h"

class BodiesCollider : public ICollider {
   public:
    BodiesCollider();

    float restitution() const;
    void setRestitution(float restitutionValue);

    bool isColliding(const ICollidable& collidable) const override;
    ColliderCollision getCollision(const ICollidable& collidable) const override;
    void setBodies(ICollidable* const* bodies, size_t count) override;
    void resolveConfiguredBodies() const override;
    void resolve(RigidBody& body) const override;

   private:
    ICollidable* const* configuredBodies = nullptr;
    size_t configuredBodyCount = 0;
    float bodiesRestitution = 1.0f;
};
