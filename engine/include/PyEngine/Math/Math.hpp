#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace PyEngine::Math {

struct AABB {
    glm::vec3 Min;
    glm::vec3 Max;

    AABB() : Min(0.0f), Max(0.0f) {}
    AABB(const glm::vec3& min, const glm::vec3& max) : Min(min), Max(max) {}

    glm::vec3 GetCenter() const { return (Min + Max) * 0.5f; }
    glm::vec3 GetSize() const { return Max - Min; }
    void Expand(const glm::vec3& point);
    void Expand(const AABB& other);
    bool Contains(const glm::vec3& point) const;
    bool Intersects(const AABB& other) const;
};

struct Plane {
    glm::vec3 Normal;
    float Distance;  // Distance from origin

    Plane() : Normal(0.0f, 1.0f, 0.0f), Distance(0.0f) {}
    Plane(const glm::vec3& normal, float distance) : Normal(glm::normalize(normal)), Distance(distance) {}
    Plane(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);

    float GetDistanceToPoint(const glm::vec3& point) const;
};

struct Frustum {
    Plane Top, Bottom, Left, Right, Near, Far;
};

struct Ray {
    glm::vec3 Origin;
    glm::vec3 Direction;

    Ray(const glm::vec3& origin, const glm::vec3& direction) : Origin(origin), Direction(glm::normalize(direction)) {}

    glm::vec3 GetPoint(float distance) const { return Origin + Direction * distance; }
};

// Intersection Tests
bool IntersectRayAABB(const Ray& ray, const AABB& aabb, float& t);
bool IntersectRayPlane(const Ray& ray, const Plane& plane, float& t);
bool IntersectRayTriangle(const Ray& ray, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t);

// Camera Frustum
Frustum CreateFrustumFromCamera(const glm::mat4& view, const glm::mat4& proj);
bool FrustumContainsPoint(const Frustum& frustum, const glm::vec3& point);
bool FrustumIntersectsAABB(const Frustum& frustum, const AABB& aabb);

}  // namespace PyEngine::Math
