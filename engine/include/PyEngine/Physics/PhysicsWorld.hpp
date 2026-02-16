#pragma once

#include <algorithm>
#include <cmath>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <limits>
#include <memory>
#include <unordered_map>
#include <vector>

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// Forward declarations
// ═══════════════════════════════════════════════════════════════
struct RaycastHit;
class Collider;
class PhysicsWorld;

// ═══════════════════════════════════════════════════════════════
// AABB — Axis-Aligned Bounding Box
// ═══════════════════════════════════════════════════════════════
struct AABB {
    glm::vec3 Min{-0.5f};
    glm::vec3 Max{0.5f};

    AABB() = default;
    AABB(const glm::vec3& min, const glm::vec3& max) : Min(min), Max(max) {}

    glm::vec3 GetCenter() const { return (Min + Max) * 0.5f; }
    glm::vec3 GetExtents() const { return (Max - Min) * 0.5f; }
    glm::vec3 GetSize() const { return Max - Min; }
    float GetVolume() const {
        auto s = GetSize();
        return s.x * s.y * s.z;
    }
    float GetSurfaceArea() const {
        auto s = GetSize();
        return 2.0f * (s.x * s.y + s.y * s.z + s.z * s.x);
    }

    bool Contains(const glm::vec3& point) const {
        return point.x >= Min.x && point.x <= Max.x && point.y >= Min.y && point.y <= Max.y && point.z >= Min.z &&
               point.z <= Max.z;
    }

    bool Intersects(const AABB& other) const {
        return Min.x <= other.Max.x && Max.x >= other.Min.x && Min.y <= other.Max.y && Max.y >= other.Min.y &&
               Min.z <= other.Max.z && Max.z >= other.Min.z;
    }

    AABB Merge(const AABB& other) const { return AABB(glm::min(Min, other.Min), glm::max(Max, other.Max)); }

    void Expand(const glm::vec3& point) {
        Min = glm::min(Min, point);
        Max = glm::max(Max, point);
    }

    void Transform(const glm::vec3& position, const glm::vec3& scale) {
        glm::vec3 center = GetCenter();
        glm::vec3 extents = GetExtents();
        center += position;
        extents *= glm::abs(scale);
        Min = center - extents;
        Max = center + extents;
    }
};

// ═══════════════════════════════════════════════════════════════
// Ray
// ═══════════════════════════════════════════════════════════════
struct Ray {
    glm::vec3 Origin{0.0f};
    glm::vec3 Direction{0.0f, 0.0f, -1.0f};
    float MaxDistance = std::numeric_limits<float>::max();

    Ray() = default;
    Ray(const glm::vec3& origin, const glm::vec3& direction, float maxDist = std::numeric_limits<float>::max())
        : Origin(origin), Direction(glm::normalize(direction)), MaxDistance(maxDist) {}

    glm::vec3 GetPoint(float t) const { return Origin + Direction * t; }

    bool IntersectsAABB(const AABB& box, float& tMin) const {
        glm::vec3 invDir = 1.0f / Direction;
        glm::vec3 t0 = (box.Min - Origin) * invDir;
        glm::vec3 t1 = (box.Max - Origin) * invDir;

        glm::vec3 tmin = glm::min(t0, t1);
        glm::vec3 tmax = glm::max(t0, t1);

        float tNear = std::max({tmin.x, tmin.y, tmin.z});
        float tFar = std::min({tmax.x, tmax.y, tmax.z});

        if (tNear > tFar || tFar < 0.0f)
            return false;

        tMin = tNear > 0.0f ? tNear : tFar;
        return tMin <= MaxDistance;
    }

    bool IntersectsSphere(const glm::vec3& center, float radius, float& t) const {
        glm::vec3 oc = Origin - center;
        float a = glm::dot(Direction, Direction);
        float b = 2.0f * glm::dot(oc, Direction);
        float c = glm::dot(oc, oc) - radius * radius;
        float discriminant = b * b - 4.0f * a * c;

        if (discriminant < 0.0f)
            return false;

        float sqrtDisc = std::sqrt(discriminant);
        float t0v = (-b - sqrtDisc) / (2.0f * a);
        float t1v = (-b + sqrtDisc) / (2.0f * a);

        t = (t0v >= 0.0f) ? t0v : t1v;
        return t >= 0.0f && t <= MaxDistance;
    }

    bool IntersectsPlane(const glm::vec3& normal, float d, float& t) const {
        float denom = glm::dot(normal, Direction);
        if (std::abs(denom) < 1e-6f)
            return false;
        t = -(glm::dot(normal, Origin) + d) / denom;
        return t >= 0.0f && t <= MaxDistance;
    }

    bool IntersectsTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t, float& u,
                            float& v) const {
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 h = glm::cross(Direction, edge2);
        float a = glm::dot(edge1, h);
        if (std::abs(a) < 1e-6f)
            return false;

        float f = 1.0f / a;
        glm::vec3 s = Origin - v0;
        u = f * glm::dot(s, h);
        if (u < 0.0f || u > 1.0f)
            return false;

        glm::vec3 q = glm::cross(s, edge1);
        v = f * glm::dot(Direction, q);
        if (v < 0.0f || u + v > 1.0f)
            return false;

        t = f * glm::dot(edge2, q);
        return t > 1e-6f && t <= MaxDistance;
    }
};

// ═══════════════════════════════════════════════════════════════
// Frustum — View frustum for culling
// ═══════════════════════════════════════════════════════════════
struct Plane {
    glm::vec3 Normal{0.0f, 1.0f, 0.0f};
    float Distance = 0.0f;

    Plane() = default;
    Plane(const glm::vec3& normal, float d) : Normal(glm::normalize(normal)), Distance(d) {}
    Plane(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
        Normal = glm::normalize(glm::cross(b - a, c - a));
        Distance = -glm::dot(Normal, a);
    }

    float DistanceToPoint(const glm::vec3& point) const { return glm::dot(Normal, point) + Distance; }
};

struct Frustum {
    Plane Planes[6];  // Near, Far, Left, Right, Top, Bottom

    void Extract(const glm::mat4& viewProjection) {
        // Left
        Planes[2].Normal.x = viewProjection[0][3] + viewProjection[0][0];
        Planes[2].Normal.y = viewProjection[1][3] + viewProjection[1][0];
        Planes[2].Normal.z = viewProjection[2][3] + viewProjection[2][0];
        Planes[2].Distance = viewProjection[3][3] + viewProjection[3][0];

        // Right
        Planes[3].Normal.x = viewProjection[0][3] - viewProjection[0][0];
        Planes[3].Normal.y = viewProjection[1][3] - viewProjection[1][0];
        Planes[3].Normal.z = viewProjection[2][3] - viewProjection[2][0];
        Planes[3].Distance = viewProjection[3][3] - viewProjection[3][0];

        // Bottom
        Planes[5].Normal.x = viewProjection[0][3] + viewProjection[0][1];
        Planes[5].Normal.y = viewProjection[1][3] + viewProjection[1][1];
        Planes[5].Normal.z = viewProjection[2][3] + viewProjection[2][1];
        Planes[5].Distance = viewProjection[3][3] + viewProjection[3][1];

        // Top
        Planes[4].Normal.x = viewProjection[0][3] - viewProjection[0][1];
        Planes[4].Normal.y = viewProjection[1][3] - viewProjection[1][1];
        Planes[4].Normal.z = viewProjection[2][3] - viewProjection[2][1];
        Planes[4].Distance = viewProjection[3][3] - viewProjection[3][1];

        // Near
        Planes[0].Normal.x = viewProjection[0][3] + viewProjection[0][2];
        Planes[0].Normal.y = viewProjection[1][3] + viewProjection[1][2];
        Planes[0].Normal.z = viewProjection[2][3] + viewProjection[2][2];
        Planes[0].Distance = viewProjection[3][3] + viewProjection[3][2];

        // Far
        Planes[1].Normal.x = viewProjection[0][3] - viewProjection[0][2];
        Planes[1].Normal.y = viewProjection[1][3] - viewProjection[1][2];
        Planes[1].Normal.z = viewProjection[2][3] - viewProjection[2][2];
        Planes[1].Distance = viewProjection[3][3] - viewProjection[3][2];

        // Normalize
        for (auto& plane : Planes) {
            float length = glm::length(plane.Normal);
            plane.Normal /= length;
            plane.Distance /= length;
        }
    }

    bool ContainsPoint(const glm::vec3& point) const {
        for (const auto& plane : Planes) {
            if (plane.DistanceToPoint(point) < 0.0f)
                return false;
        }
        return true;
    }

    bool IntersectsAABB(const AABB& box) const {
        for (const auto& plane : Planes) {
            glm::vec3 positiveVertex = box.Min;
            if (plane.Normal.x >= 0)
                positiveVertex.x = box.Max.x;
            if (plane.Normal.y >= 0)
                positiveVertex.y = box.Max.y;
            if (plane.Normal.z >= 0)
                positiveVertex.z = box.Max.z;

            if (plane.DistanceToPoint(positiveVertex) < 0.0f)
                return false;
        }
        return true;
    }

    bool IntersectsSphere(const glm::vec3& center, float radius) const {
        for (const auto& plane : Planes) {
            if (plane.DistanceToPoint(center) < -radius)
                return false;
        }
        return true;
    }
};

// ═══════════════════════════════════════════════════════════════
// RaycastHit — Result of a raycast
// ═══════════════════════════════════════════════════════════════
struct RaycastHit {
    glm::vec3 Point{0.0f};
    glm::vec3 Normal{0.0f, 1.0f, 0.0f};
    float Distance = 0.0f;
    uint32_t EntityID = 0;
    bool Hit = false;

    operator bool() const { return Hit; }
};

// ═══════════════════════════════════════════════════════════════
// ColliderShape — Collision shape types
// ═══════════════════════════════════════════════════════════════
enum class ColliderType { Box, Sphere, Capsule, Mesh };

struct CollisionInfo {
    glm::vec3 ContactPoint{0.0f};
    glm::vec3 Normal{0.0f};
    float Penetration = 0.0f;
    uint32_t EntityA = 0;
    uint32_t EntityB = 0;
    bool IsColliding = false;
};

// ═══════════════════════════════════════════════════════════════
// RigidBody — Physics body simulation
// ═══════════════════════════════════════════════════════════════
struct RigidBodyData {
    glm::vec3 Position{0.0f};
    glm::vec3 Velocity{0.0f};
    glm::vec3 Acceleration{0.0f};
    glm::vec3 Force{0.0f};
    glm::vec3 Torque{0.0f};
    glm::vec3 AngularVelocity{0.0f};
    glm::quat Rotation{1.0f, 0.0f, 0.0f, 0.0f};

    float Mass = 1.0f;
    float InverseMass = 1.0f;
    float Restitution = 0.3f;
    float StaticFriction = 0.5f;
    float DynamicFriction = 0.3f;
    float LinearDamping = 0.01f;
    float AngularDamping = 0.05f;

    bool IsStatic = false;
    bool IsKinematic = false;
    bool UseGravity = true;
    bool IsSleeping = false;

    float SleepThreshold = 0.005f;
    float SleepTimer = 0.0f;
    float SleepTimeout = 2.0f;

    uint32_t EntityID = 0;
    ColliderType Shape = ColliderType::Box;
    glm::vec3 ColliderSize{1.0f};
    float ColliderRadius = 0.5f;

    void SetMass(float mass) {
        Mass = mass;
        InverseMass = (mass > 0.0f && !IsStatic) ? (1.0f / mass) : 0.0f;
    }

    void ApplyForce(const glm::vec3& force) { Force += force; }
    void ApplyImpulse(const glm::vec3& impulse) { Velocity += impulse * InverseMass; }
    void ApplyTorque(const glm::vec3& torque) { Torque += torque; }

    void ClearForces() {
        Force = glm::vec3(0.0f);
        Torque = glm::vec3(0.0f);
    }
};

// ═══════════════════════════════════════════════════════════════
// Collision Detection Functions
// ═══════════════════════════════════════════════════════════════
namespace CollisionDetection {

inline bool SphereVsSphere(const glm::vec3& posA, float radiusA, const glm::vec3& posB, float radiusB,
                           CollisionInfo& info) {
    glm::vec3 diff = posB - posA;
    float dist = glm::length(diff);
    float sumRadius = radiusA + radiusB;

    if (dist >= sumRadius)
        return false;

    info.IsColliding = true;
    info.Normal = (dist > 1e-6f) ? diff / dist : glm::vec3(0.0f, 1.0f, 0.0f);
    info.Penetration = sumRadius - dist;
    info.ContactPoint = posA + info.Normal * radiusA;
    return true;
}

inline bool AABBvsAABB(const AABB& a, const AABB& b, CollisionInfo& info) {
    if (!a.Intersects(b))
        return false;

    glm::vec3 overlap;
    overlap.x = std::min(a.Max.x - b.Min.x, b.Max.x - a.Min.x);
    overlap.y = std::min(a.Max.y - b.Min.y, b.Max.y - a.Min.y);
    overlap.z = std::min(a.Max.z - b.Min.z, b.Max.z - a.Min.z);

    info.IsColliding = true;

    // Find axis of minimum penetration
    if (overlap.x <= overlap.y && overlap.x <= overlap.z) {
        info.Normal = (a.GetCenter().x < b.GetCenter().x) ? glm::vec3(-1, 0, 0) : glm::vec3(1, 0, 0);
        info.Penetration = overlap.x;
    } else if (overlap.y <= overlap.x && overlap.y <= overlap.z) {
        info.Normal = (a.GetCenter().y < b.GetCenter().y) ? glm::vec3(0, -1, 0) : glm::vec3(0, 1, 0);
        info.Penetration = overlap.y;
    } else {
        info.Normal = (a.GetCenter().z < b.GetCenter().z) ? glm::vec3(0, 0, -1) : glm::vec3(0, 0, 1);
        info.Penetration = overlap.z;
    }

    info.ContactPoint = (a.GetCenter() + b.GetCenter()) * 0.5f;
    return true;
}

inline bool SphereVsAABB(const glm::vec3& spherePos, float radius, const AABB& box, CollisionInfo& info) {
    glm::vec3 closest = glm::clamp(spherePos, box.Min, box.Max);
    glm::vec3 diff = spherePos - closest;
    float distSq = glm::dot(diff, diff);

    if (distSq >= radius * radius)
        return false;

    float dist = std::sqrt(distSq);
    info.IsColliding = true;
    info.Normal = (dist > 1e-6f) ? diff / dist : glm::vec3(0.0f, 1.0f, 0.0f);
    info.Penetration = radius - dist;
    info.ContactPoint = closest;
    return true;
}
}  // namespace CollisionDetection

// ═══════════════════════════════════════════════════════════════
// PhysicsWorld — Main physics simulation
// ═══════════════════════════════════════════════════════════════
class PhysicsWorld {
public:
    PhysicsWorld();
    ~PhysicsWorld() = default;

    void SetGravity(const glm::vec3& gravity) { m_Gravity = gravity; }
    glm::vec3 GetGravity() const { return m_Gravity; }

    void SetTimeStep(float step) { m_FixedTimeStep = step; }
    float GetTimeStep() const { return m_FixedTimeStep; }

    void SetIterations(int iterations) { m_SolverIterations = iterations; }

    // Body management
    uint32_t AddBody(const RigidBodyData& body);
    void RemoveBody(uint32_t id);
    RigidBodyData* GetBody(uint32_t id);
    const std::vector<RigidBodyData>& GetBodies() const { return m_Bodies; }

    // Simulation
    void Step(float deltaTime);
    void Clear();

    // Queries
    RaycastHit Raycast(const Ray& ray) const;
    std::vector<RaycastHit> RaycastAll(const Ray& ray) const;
    std::vector<uint32_t> OverlapSphere(const glm::vec3& center, float radius) const;
    std::vector<uint32_t> OverlapBox(const AABB& box) const;

    // Collision callbacks
    using CollisionCallback = std::function<void(const CollisionInfo&)>;
    void SetCollisionCallback(CollisionCallback callback) { m_CollisionCallback = callback; }

    // Statistics
    struct Stats {
        uint32_t BodyCount = 0;
        uint32_t ActiveBodies = 0;
        uint32_t CollisionChecks = 0;
        uint32_t CollisionsDetected = 0;
        float StepTime = 0.0f;
    };
    const Stats& GetStats() const { return m_Stats; }

private:
    void IntegrateBodies(float dt);
    void DetectCollisions();
    void ResolveCollisions();
    void UpdateSleepState(float dt);
    AABB GetBodyAABB(const RigidBodyData& body) const;

private:
    std::vector<RigidBodyData> m_Bodies;
    std::vector<CollisionInfo> m_Collisions;

    glm::vec3 m_Gravity{0.0f, -9.81f, 0.0f};
    float m_FixedTimeStep = 1.0f / 60.0f;
    float m_Accumulator = 0.0f;
    int m_SolverIterations = 8;

    CollisionCallback m_CollisionCallback;
    Stats m_Stats;
};

}  // namespace PyEngine
