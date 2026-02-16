#include "PyEngine/Math/Math.hpp"

#include <algorithm>
#include <cmath>

namespace PyEngine::Math {

// ═══════════════════════════════════════════════════════════════
// AABB
// ═══════════════════════════════════════════════════════════════

void AABB::Expand(const glm::vec3& point) {
    Min = glm::min(Min, point);
    Max = glm::max(Max, point);
}

void AABB::Expand(const AABB& other) {
    Min = glm::min(Min, other.Min);
    Max = glm::max(Max, other.Max);
}

bool AABB::Contains(const glm::vec3& point) const {
    return point.x >= Min.x && point.x <= Max.x && point.y >= Min.y && point.y <= Max.y && point.z >= Min.z &&
           point.z <= Max.z;
}

bool AABB::Intersects(const AABB& other) const {
    return (Min.x <= other.Max.x && Max.x >= other.Min.x) && (Min.y <= other.Max.y && Max.y >= other.Min.y) &&
           (Min.z <= other.Max.z && Max.z >= other.Min.z);
}

// ═══════════════════════════════════════════════════════════════
// Plane
// ═══════════════════════════════════════════════════════════════

Plane::Plane(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) {
    Normal = glm::normalize(glm::cross(p2 - p1, p3 - p1));
    Distance = glm::dot(Normal, p1);
}

float Plane::GetDistanceToPoint(const glm::vec3& point) const {
    return glm::dot(Normal, point) - Distance;
}

// ═══════════════════════════════════════════════════════════════
// Intersections
// ═══════════════════════════════════════════════════════════════

bool IntersectRayAABB(const Ray& ray, const AABB& aabb, float& t) {
    glm::vec3 invDir = 1.0f / ray.Direction;
    glm::vec3 tMin = (aabb.Min - ray.Origin) * invDir;
    glm::vec3 tMax = (aabb.Max - ray.Origin) * invDir;

    glm::vec3 t1 = glm::min(tMin, tMax);
    glm::vec3 t2 = glm::max(tMin, tMax);

    float tNear = std::max(std::max(t1.x, t1.y), t1.z);
    float tFar = std::min(std::min(t2.x, t2.y), t2.z);

    if (tNear > tFar || tFar < 0.0f)
        return false;

    t = tNear;
    return true;
}

bool IntersectRayPlane(const Ray& ray, const Plane& plane, float& t) {
    float denom = glm::dot(plane.Normal, ray.Direction);
    if (std::abs(denom) > 1e-6) {
        t = (plane.Distance - glm::dot(plane.Normal, ray.Origin)) / denom;
        return t >= 0.0f;
    }
    return false;
}

bool IntersectRayTriangle(const Ray& ray, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t) {
    const float EPSILON = 0.0000001f;
    glm::vec3 edge1, edge2, h, s, q;
    float a, f, u, v;

    edge1 = v1 - v0;
    edge2 = v2 - v0;
    h = glm::cross(ray.Direction, edge2);
    a = glm::dot(edge1, h);

    if (a > -EPSILON && a < EPSILON)
        return false;

    f = 1.0f / a;
    s = ray.Origin - v0;
    u = f * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f)
        return false;

    q = glm::cross(s, edge1);
    v = f * glm::dot(ray.Direction, q);
    if (v < 0.0f || u + v > 1.0f)
        return false;

    t = f * glm::dot(edge2, q);
    return t > EPSILON;
}

// ═══════════════════════════════════════════════════════════════
// Frustum
// ═══════════════════════════════════════════════════════════════

Frustum CreateFrustumFromCamera(const glm::mat4& view, const glm::mat4& proj) {
    Frustum f;
    glm::mat4 vp = proj * view;

    // Left
    f.Left.Normal.x = vp[0][3] + vp[0][0];
    f.Left.Normal.y = vp[1][3] + vp[1][0];
    f.Left.Normal.z = vp[2][3] + vp[2][0];
    f.Left.Distance = vp[3][3] + vp[3][0];

    // Right
    f.Right.Normal.x = vp[0][3] - vp[0][0];
    f.Right.Normal.y = vp[1][3] - vp[1][0];
    f.Right.Normal.z = vp[2][3] - vp[2][0];
    f.Right.Distance = vp[3][3] - vp[3][0];

    // Top
    f.Top.Normal.x = vp[0][3] - vp[0][1];
    f.Top.Normal.y = vp[1][3] - vp[1][1];
    f.Top.Normal.z = vp[2][3] - vp[2][1];
    f.Top.Distance = vp[3][3] - vp[3][1];

    // Bottom
    f.Bottom.Normal.x = vp[0][3] + vp[0][1];
    f.Bottom.Normal.y = vp[1][3] + vp[1][1];
    f.Bottom.Normal.z = vp[2][3] + vp[2][1];
    f.Bottom.Distance = vp[3][3] + vp[3][1];

    // Near
    f.Near.Normal.x = vp[0][3] + vp[0][2];
    f.Near.Normal.y = vp[1][3] + vp[1][2];
    f.Near.Normal.z = vp[2][3] + vp[2][2];
    f.Near.Distance = vp[3][3] + vp[3][2];

    // Far
    f.Far.Normal.x = vp[0][3] - vp[0][2];
    f.Far.Normal.y = vp[1][3] - vp[1][2];
    f.Far.Normal.z = vp[2][3] - vp[2][2];
    f.Far.Distance = vp[3][3] - vp[3][2];

    // Normalize planes
    auto normalize = [](Plane& p) {
        float len = glm::length(p.Normal);
        p.Normal /= len;
        p.Distance /= len;
    };

    normalize(f.Left);
    normalize(f.Right);
    normalize(f.Top);
    normalize(f.Bottom);
    normalize(f.Near);
    normalize(f.Far);

    return f;
}

bool FrustumContainsPoint(const Frustum& frustum, const glm::vec3& point) {
    if (frustum.Left.GetDistanceToPoint(point) < 0)
        return false;
    if (frustum.Right.GetDistanceToPoint(point) < 0)
        return false;
    if (frustum.Top.GetDistanceToPoint(point) < 0)
        return false;
    if (frustum.Bottom.GetDistanceToPoint(point) < 0)
        return false;
    if (frustum.Near.GetDistanceToPoint(point) < 0)
        return false;
    if (frustum.Far.GetDistanceToPoint(point) < 0)
        return false;
    return true;
}

bool FrustumIntersectsAABB(const Frustum& frustum, const AABB& aabb) {
    // Check all 8 corners against frustum planes
    glm::vec3 corners[8] = {{aabb.Min.x, aabb.Min.y, aabb.Min.z}, {aabb.Max.x, aabb.Min.y, aabb.Min.z},
                            {aabb.Min.x, aabb.Max.y, aabb.Min.z}, {aabb.Max.x, aabb.Max.y, aabb.Min.z},
                            {aabb.Min.x, aabb.Min.y, aabb.Max.z}, {aabb.Max.x, aabb.Min.y, aabb.Max.z},
                            {aabb.Min.x, aabb.Max.y, aabb.Max.z}, {aabb.Max.x, aabb.Max.y, aabb.Max.z}};

    // Note: This is an approximation (check if all points are outside one plane)
    // Precise test requires checking frustum points against AABB too

    auto checkPlane = [&](const Plane& p) {
        int out = 0;
        for (int i = 0; i < 8; i++) {
            if (p.GetDistanceToPoint(corners[i]) < 0)
                out++;
        }
        return out == 8;
    };

    if (checkPlane(frustum.Left))
        return false;
    if (checkPlane(frustum.Right))
        return false;
    if (checkPlane(frustum.Top))
        return false;
    if (checkPlane(frustum.Bottom))
        return false;
    if (checkPlane(frustum.Near))
        return false;
    if (checkPlane(frustum.Far))
        return false;

    return true;
}

}  // namespace PyEngine::Math
