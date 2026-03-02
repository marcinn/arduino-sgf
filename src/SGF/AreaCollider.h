#pragma once

#include "RigidBody.h"
#include "Vector2.h"

class AreaCollider {
   public:
    static constexpr int EDGE_LEFT = 0;
    static constexpr int EDGE_RIGHT = 1;
    static constexpr int EDGE_TOP = 2;
    static constexpr int EDGE_BOTTOM = 3;

    using CalculateResponseFn =
        void (*)(RigidBody& body, int edge, Vector2f* normal, float* restitution,
                 bool* setOnFloor, bool* enabled);

    AreaCollider();
    AreaCollider(const Vector2i& minBounds, const Vector2i& maxBounds, float restitution = 1.0f);
    AreaCollider(const Vector2f& minBounds, const Vector2f& maxBounds, float restitution = 1.0f);

    Vector2f minBounds() const;
    Vector2f maxBounds() const;

    void setMinBounds(const Vector2i& minBounds);
    void setMinBounds(const Vector2f& minBounds);
    void setMaxBounds(const Vector2i& maxBounds);
    void setMaxBounds(const Vector2f& maxBounds);
    void setBounds(const Vector2i& minBounds, const Vector2i& maxBounds);
    void setBounds(const Vector2f& minBounds, const Vector2f& maxBounds);

    void setLeftResponse(float restitution, bool enabled = true);
    void setRightResponse(float restitution, bool enabled = true);
    void setTopResponse(float restitution, bool enabled = true);
    void setBottomResponse(float restitution, bool enabled = true, bool setOnFloor = true);
    void setCalculateResponseFn(CalculateResponseFn fn);

    bool resolve(RigidBody& body) const;

   private:
    struct EdgeResponse {
        float restitution = 1.0f;
        bool setOnFloor = false;
        bool enabled = true;
    };

    static void sanitizeResponse(EdgeResponse* response);
    bool resolveEdge(RigidBody& body, int edge, float* positionAxis, float bound,
                     const Vector2f& defaultNormal, const EdgeResponse& defaultResponse,
                     bool* setOnFloor) const;

    Vector2f minCollisionBounds{};
    Vector2f maxCollisionBounds{};
    EdgeResponse leftResponse{};
    EdgeResponse rightResponse{};
    EdgeResponse topResponse{};
    EdgeResponse bottomResponse{1.0f, true, true};
    CalculateResponseFn calculateResponseFn = nullptr;
};
