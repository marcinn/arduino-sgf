#include "BodiesCollider.h"

#include "Collision.h"
#include "Physics.h"

#include <math.h>

namespace {
float supportDistance(const ICollidable& collidable, const Vector2f& normal) {
    CollisionShape shape = collidable.getCollisionShape();
    if (shape.type() == CollisionShapeType::Point) {
        return 0.0f;
    }
    if (shape.type() == CollisionShapeType::Circle) {
        return (float)shape.radius();
    }

    Vector2f halfSize = Vector2f{shape.size()} / 2.0f;
    return fabsf(normal.x) * halfSize.x + fabsf(normal.y) * halfSize.y;
}

Vector2f centerToReferenceOffset(const RigidBody& body) {
    CollisionShape shape = body.getCollisionShape();
    if (shape.type() == CollisionShapeType::Point) {
        return Vector2f{};
    }

    Vector2f size{};
    if (shape.type() == CollisionShapeType::Rect) {
        size = Vector2f{shape.size()};
    } else if (shape.type() == CollisionShapeType::Circle) {
        float diameter = (float)(shape.radius() * 2);
        size = Vector2f{diameter, diameter};
    }

    return (body.anchor() - Vector2f{0.5f, 0.5f}) * size;
}

void resolveRigidAgainstStatic(RigidBody& rigidBody, const ICollidable& other, float restitution) {
    ColliderCollision collision = collidablesCollision(rigidBody, other);
    if (!collision.isColliding()) {
        return;
    }

    Vector2f normal = collision.normal();
    float normalLengthSq = normal.x * normal.x + normal.y * normal.y;
    if (normalLengthSq <= 0.0f) {
        return;
    }

    float normalLength = sqrtf(normalLengthSq);
    Vector2f unitNormal = normal / normalLength;
    float rigidSupport = supportDistance(rigidBody, unitNormal);
    Vector2f correctedCenter = collision.position() + (unitNormal * rigidSupport);
    rigidBody.setPosition(correctedCenter + centerToReferenceOffset(rigidBody));
    Physics::bounce(rigidBody, unitNormal, restitution);
    rigidBody.setOnFloor(unitNormal == Vector2f{0.0f, -1.0f});
}
}  // namespace

BodiesCollider::BodiesCollider() = default;

float BodiesCollider::restitution() const { return bodiesRestitution; }

void BodiesCollider::setRestitution(float restitutionValue) {
    if (restitutionValue < 0.0f) {
        bodiesRestitution = 0.0f;
    } else {
        bodiesRestitution = restitutionValue;
    }
}

bool BodiesCollider::isColliding(const ICollidable& collidable) const {
    return getCollision(collidable).isColliding();
}

ColliderCollision BodiesCollider::getCollision(const ICollidable& collidable) const {
    if (!configuredBodies) {
        return ColliderCollision{};
    }

    for (size_t i = 0; i < configuredBodyCount; ++i) {
        if (configuredBodies[i] == &collidable) {
            continue;
        }

        ColliderCollision collision = collidablesCollision(collidable, *configuredBodies[i]);
        if (collision.isColliding()) {
            return collision;
        }
    }

    return ColliderCollision{};
}

void BodiesCollider::setBodies(ICollidable* const* bodies, size_t count) {
    configuredBodies = bodies;
    configuredBodyCount = count;
}

void BodiesCollider::resolveConfiguredBodies() const {
    if (!configuredBodies) {
        return;
    }

    for (size_t i = 0; i < configuredBodyCount; ++i) {
        if (configuredBodies[i]->collisionBodyType() != CollisionBodyType::Rigid) {
            continue;
        }

        RigidBody& first = *(RigidBody*)configuredBodies[i];
        for (size_t j = i + 1; j < configuredBodyCount; ++j) {
            if (configuredBodies[j]->collisionBodyType() == CollisionBodyType::Rigid) {
                Physics::resolveBodies(first, *(RigidBody*)configuredBodies[j], bodiesRestitution);
            } else if (configuredBodies[j]->collisionBodyType() == CollisionBodyType::Static) {
                resolveRigidAgainstStatic(first, *configuredBodies[j], bodiesRestitution);
            }
        }
    }
}

void BodiesCollider::resolve(RigidBody& body) const { (void)body; }
