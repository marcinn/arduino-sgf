#include "AreaCollider.h"

#include "ColliderCollision.h"
#include "CollisionShape.h"
#include "Physics.h"

AreaCollider::AreaCollider() = default;

AreaCollider::AreaCollider(const Vector2i& minBounds, const Vector2i& maxBounds, float restitution)
    : AreaCollider(Vector2f{minBounds}, Vector2f{maxBounds}, restitution) {}

AreaCollider::AreaCollider(const Vector2f& minBounds, const Vector2f& maxBounds, float restitution)
    : minCollisionBounds(minBounds), maxCollisionBounds(maxBounds) {
    setLeftResponse(restitution);
    setRightResponse(restitution);
    setTopResponse(restitution);
    setBottomResponse(restitution);
}

Vector2f AreaCollider::minBounds() const { return minCollisionBounds; }

Vector2f AreaCollider::maxBounds() const { return maxCollisionBounds; }

void AreaCollider::setMinBounds(const Vector2i& minBounds) { setMinBounds(Vector2f{minBounds}); }

void AreaCollider::setMinBounds(const Vector2f& minBounds) { minCollisionBounds = minBounds; }

void AreaCollider::setMaxBounds(const Vector2i& maxBounds) { setMaxBounds(Vector2f{maxBounds}); }

void AreaCollider::setMaxBounds(const Vector2f& maxBounds) { maxCollisionBounds = maxBounds; }

void AreaCollider::setBounds(const Vector2i& minBounds, const Vector2i& maxBounds) {
    setBounds(Vector2f{minBounds}, Vector2f{maxBounds});
}

void AreaCollider::setBounds(const Vector2f& minBounds, const Vector2f& maxBounds) {
    minCollisionBounds = minBounds;
    maxCollisionBounds = maxBounds;
}

void AreaCollider::setLeftResponse(float restitution, bool enabled) {
    leftResponse.restitution = restitution;
    leftResponse.setOnFloor = false;
    leftResponse.enabled = enabled;
    sanitizeResponse(&leftResponse);
}

void AreaCollider::setRightResponse(float restitution, bool enabled) {
    rightResponse.restitution = restitution;
    rightResponse.setOnFloor = false;
    rightResponse.enabled = enabled;
    sanitizeResponse(&rightResponse);
}

void AreaCollider::setTopResponse(float restitution, bool enabled) {
    topResponse.restitution = restitution;
    topResponse.setOnFloor = false;
    topResponse.enabled = enabled;
    sanitizeResponse(&topResponse);
}

void AreaCollider::setBottomResponse(float restitution, bool enabled, bool setOnFloor) {
    bottomResponse.restitution = restitution;
    bottomResponse.setOnFloor = setOnFloor;
    bottomResponse.enabled = enabled;
    sanitizeResponse(&bottomResponse);
}

void AreaCollider::setCalculateResponseFn(CalculateResponseFn fn) { calculateResponseFn = fn; }

bool AreaCollider::isColliding(const ICollidable& collidable) const {
    return getCollision(collidable).isColliding();
}

ColliderCollision AreaCollider::getCollision(const ICollidable& collidable) const {
    ColliderCollision collision;
    Vector2f position = collidable.getCollisionPosition();
    CollisionShape collisionShape = collidable.getCollisionShape();
    Vector2f minPosition = position;
    Vector2f maxPosition = position;

    if (collisionShape.type() == CollisionShapeType::Rect) {
        Vector2f halfSize = Vector2f{collisionShape.size()} / 2.0f;
        minPosition = position - halfSize;
        maxPosition = position + halfSize;
    } else if (collisionShape.type() == CollisionShapeType::Circle) {
        float radius = (float)collisionShape.radius();
        minPosition = position - Vector2f{radius, radius};
        maxPosition = position + Vector2f{radius, radius};
    }

    if (minPosition.x < minCollisionBounds.x) {
        collision.setColliding(true);
        collision.setPosition(
            Vector2f{position.x + (minCollisionBounds.x - minPosition.x), position.y});
        collision.setNormal(Vector2f{1.0f, 0.0f});
        return collision;
    }
    if (maxPosition.x > maxCollisionBounds.x) {
        collision.setColliding(true);
        collision.setPosition(
            Vector2f{position.x - (maxPosition.x - maxCollisionBounds.x), position.y});
        collision.setNormal(Vector2f{-1.0f, 0.0f});
        return collision;
    }
    if (minPosition.y < minCollisionBounds.y) {
        collision.setColliding(true);
        collision.setPosition(
            Vector2f{position.x, position.y + (minCollisionBounds.y - minPosition.y)});
        collision.setNormal(Vector2f{0.0f, 1.0f});
        return collision;
    }
    if (maxPosition.y > maxCollisionBounds.y) {
        collision.setColliding(true);
        collision.setPosition(
            Vector2f{position.x, position.y - (maxPosition.y - maxCollisionBounds.y)});
        collision.setNormal(Vector2f{0.0f, -1.0f});
        return collision;
    }
    return collision;
}

void AreaCollider::resolve(RigidBody& body) const {
    ColliderCollision collision = getCollision(body);
    if (!collision.isColliding()) {
        return;
    }
    Vector2f collisionOffset = collision.position() - body.getCollisionPosition();
    body.setPosition(body.getPosition() + collisionOffset);
    Physics::bounce(body, collision.normal(), collision.normal() == Vector2f{1.0f, 0.0f}
                                                 ? leftResponse.restitution
                                                 : collision.normal() == Vector2f{-1.0f, 0.0f}
                                                       ? rightResponse.restitution
                                                       : collision.normal() == Vector2f{0.0f, 1.0f}
                                                             ? topResponse.restitution
                                                             : bottomResponse.restitution);
    body.setOnFloor(collision.normal() == Vector2f{0.0f, -1.0f} && bottomResponse.setOnFloor);
}

void AreaCollider::sanitizeResponse(EdgeResponse* response) {
    if (response->restitution < 0.0f) {
        response->restitution = 0.0f;
    }
}

bool AreaCollider::resolveEdge(RigidBody& body, int edge, float* positionAxis, float bound,
                               const Vector2f& defaultNormal,
                               const EdgeResponse& defaultResponse,
                               bool* setOnFloor) const {
    Vector2f normal = defaultNormal;
    float restitution = defaultResponse.restitution;
    bool edgeSetOnFloor = defaultResponse.setOnFloor;
    bool enabled = defaultResponse.enabled;

    if (calculateResponseFn) {
        calculateResponseFn(body, edge, &normal, &restitution, &edgeSetOnFloor, &enabled);
    }

    if (!enabled) {
        return false;
    }

    if (restitution < 0.0f) {
        restitution = 0.0f;
    }

    *positionAxis = bound;
    Physics::bounce(body, normal, restitution);
    if (edgeSetOnFloor) {
        *setOnFloor = true;
    }
    return true;
}
