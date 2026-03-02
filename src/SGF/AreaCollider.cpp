#include "AreaCollider.h"

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

bool AreaCollider::resolve(RigidBody& body) const {
    Vector2f position = body.getPosition();
    bool collided = false;
    bool setOnFloor = false;

    if (position.x < minCollisionBounds.x) {
        collided |= resolveEdge(body, EDGE_LEFT, &position.x, minCollisionBounds.x,
                                Vector2f{1.0f, 0.0f}, leftResponse, &setOnFloor);
    } else if (position.x > maxCollisionBounds.x) {
        collided |= resolveEdge(body, EDGE_RIGHT, &position.x, maxCollisionBounds.x,
                                Vector2f{-1.0f, 0.0f}, rightResponse, &setOnFloor);
    }

    if (position.y < minCollisionBounds.y) {
        collided |= resolveEdge(body, EDGE_TOP, &position.y, minCollisionBounds.y,
                                Vector2f{0.0f, 1.0f}, topResponse, &setOnFloor);
    } else if (position.y > maxCollisionBounds.y) {
        collided |= resolveEdge(body, EDGE_BOTTOM, &position.y, maxCollisionBounds.y,
                                Vector2f{0.0f, -1.0f}, bottomResponse, &setOnFloor);
    }

    if (!collided) {
        return false;
    }

    body.setPosition(position);
    body.setOnFloor(setOnFloor);
    return true;
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
