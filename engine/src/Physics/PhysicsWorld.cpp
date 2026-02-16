#include "PyEngine/Physics/PhysicsWorld.hpp"

#include <chrono>
#include <glm/gtc/quaternion.hpp>

namespace PyEngine {

PhysicsWorld::PhysicsWorld() {
    m_Bodies.reserve(1024);
    m_Collisions.reserve(256);
}

uint32_t PhysicsWorld::AddBody(const RigidBodyData& body) {
    RigidBodyData newBody = body;
    if (newBody.IsStatic)
        newBody.InverseMass = 0.0f;
    m_Bodies.push_back(newBody);
    return static_cast<uint32_t>(m_Bodies.size() - 1);
}

void PhysicsWorld::RemoveBody(uint32_t id) {
    if (id < m_Bodies.size()) {
        m_Bodies.erase(m_Bodies.begin() + id);
    }
}

RigidBodyData* PhysicsWorld::GetBody(uint32_t id) {
    if (id < m_Bodies.size())
        return &m_Bodies[id];
    return nullptr;
}

void PhysicsWorld::Step(float deltaTime) {
    auto start = std::chrono::high_resolution_clock::now();

    m_Accumulator += deltaTime;

    while (m_Accumulator >= m_FixedTimeStep) {
        IntegrateBodies(m_FixedTimeStep);
        DetectCollisions();
        ResolveCollisions();
        UpdateSleepState(m_FixedTimeStep);
        m_Accumulator -= m_FixedTimeStep;
    }

    auto end = std::chrono::high_resolution_clock::now();
    m_Stats.StepTime = std::chrono::duration<float, std::milli>(end - start).count();
    m_Stats.BodyCount = static_cast<uint32_t>(m_Bodies.size());
}

void PhysicsWorld::Clear() {
    m_Bodies.clear();
    m_Collisions.clear();
    m_Accumulator = 0.0f;
    m_Stats = {};
}

void PhysicsWorld::IntegrateBodies(float dt) {
    m_Stats.ActiveBodies = 0;

    for (auto& body : m_Bodies) {
        if (body.IsStatic || body.IsKinematic || body.IsSleeping)
            continue;

        m_Stats.ActiveBodies++;

        // Apply gravity
        if (body.UseGravity) {
            body.Force += m_Gravity * body.Mass;
        }

        // Semi-implicit Euler integration
        body.Acceleration = body.Force * body.InverseMass;
        body.Velocity += body.Acceleration * dt;
        body.AngularVelocity += body.Torque * body.InverseMass * dt;  // Simplified

        // Apply damping
        body.Velocity *= (1.0f - body.LinearDamping * dt);
        body.AngularVelocity *= (1.0f - body.AngularDamping * dt);

        // Integrate position
        body.Position += body.Velocity * dt;

        // Integrate rotation (simplified quaternion integration)
        glm::quat spin(0.0f, body.AngularVelocity.x * 0.5f * dt, body.AngularVelocity.y * 0.5f * dt,
                       body.AngularVelocity.z * 0.5f * dt);
        body.Rotation = glm::normalize(body.Rotation + spin * body.Rotation);

        body.ClearForces();
    }
}

void PhysicsWorld::DetectCollisions() {
    m_Collisions.clear();
    m_Stats.CollisionChecks = 0;
    m_Stats.CollisionsDetected = 0;

    // Broad phase: AABB overlap check
    for (size_t i = 0; i < m_Bodies.size(); i++) {
        if (m_Bodies[i].IsSleeping)
            continue;

        for (size_t j = i + 1; j < m_Bodies.size(); j++) {
            if (m_Bodies[j].IsSleeping && m_Bodies[i].IsStatic)
                continue;
            if (m_Bodies[i].IsStatic && m_Bodies[j].IsStatic)
                continue;

            m_Stats.CollisionChecks++;

            AABB aabbA = GetBodyAABB(m_Bodies[i]);
            AABB aabbB = GetBodyAABB(m_Bodies[j]);

            if (!aabbA.Intersects(aabbB))
                continue;

            // Narrow phase
            CollisionInfo info;
            info.EntityA = m_Bodies[i].EntityID;
            info.EntityB = m_Bodies[j].EntityID;

            bool collided = false;

            if (m_Bodies[i].Shape == ColliderType::Sphere && m_Bodies[j].Shape == ColliderType::Sphere) {
                collided = CollisionDetection::SphereVsSphere(m_Bodies[i].Position, m_Bodies[i].ColliderRadius,
                                                              m_Bodies[j].Position, m_Bodies[j].ColliderRadius, info);
            } else if (m_Bodies[i].Shape == ColliderType::Box && m_Bodies[j].Shape == ColliderType::Box) {
                collided = CollisionDetection::AABBvsAABB(aabbA, aabbB, info);
            } else if (m_Bodies[i].Shape == ColliderType::Sphere && m_Bodies[j].Shape == ColliderType::Box) {
                collided =
                    CollisionDetection::SphereVsAABB(m_Bodies[i].Position, m_Bodies[i].ColliderRadius, aabbB, info);
            } else if (m_Bodies[i].Shape == ColliderType::Box && m_Bodies[j].Shape == ColliderType::Sphere) {
                collided =
                    CollisionDetection::SphereVsAABB(m_Bodies[j].Position, m_Bodies[j].ColliderRadius, aabbA, info);
                info.Normal = -info.Normal;
            }

            if (collided) {
                m_Collisions.push_back(info);
                m_Stats.CollisionsDetected++;

                if (m_CollisionCallback) {
                    m_CollisionCallback(info);
                }
            }
        }
    }
}

void PhysicsWorld::ResolveCollisions() {
    for (int iter = 0; iter < m_SolverIterations; iter++) {
        for (auto& collision : m_Collisions) {
            // Find bodies
            RigidBodyData* bodyA = nullptr;
            RigidBodyData* bodyB = nullptr;

            for (auto& b : m_Bodies) {
                if (b.EntityID == collision.EntityA)
                    bodyA = &b;
                if (b.EntityID == collision.EntityB)
                    bodyB = &b;
            }

            if (!bodyA || !bodyB)
                continue;

            float totalInvMass = bodyA->InverseMass + bodyB->InverseMass;
            if (totalInvMass <= 0.0f)
                continue;

            // Relative velocity
            glm::vec3 relVel = bodyB->Velocity - bodyA->Velocity;
            float contactVel = glm::dot(relVel, collision.Normal);

            // Don't resolve if separating
            if (contactVel > 0.0f)
                continue;

            // Restitution
            float restitution = std::min(bodyA->Restitution, bodyB->Restitution);

            // Impulse magnitude
            float j = -(1.0f + restitution) * contactVel / totalInvMass;

            // Apply impulse
            glm::vec3 impulse = collision.Normal * j;
            bodyA->Velocity -= impulse * bodyA->InverseMass;
            bodyB->Velocity += impulse * bodyB->InverseMass;

            // Friction
            glm::vec3 tangent = relVel - collision.Normal * contactVel;
            if (glm::length(tangent) > 1e-6f) {
                tangent = glm::normalize(tangent);
                float jt = -glm::dot(relVel, tangent) / totalInvMass;

                float mu = std::sqrt(bodyA->StaticFriction * bodyA->StaticFriction +
                                     bodyB->StaticFriction * bodyB->StaticFriction);

                glm::vec3 frictionImpulse;
                if (std::abs(jt) < j * mu) {
                    frictionImpulse = tangent * jt;
                } else {
                    float dynamicFriction = std::sqrt(bodyA->DynamicFriction * bodyA->DynamicFriction +
                                                      bodyB->DynamicFriction * bodyB->DynamicFriction);
                    frictionImpulse = tangent * (-j * dynamicFriction);
                }

                bodyA->Velocity -= frictionImpulse * bodyA->InverseMass;
                bodyB->Velocity += frictionImpulse * bodyB->InverseMass;
            }

            // Positional correction (prevents sinking)
            const float correctionPercent = 0.8f;
            const float slop = 0.01f;
            float correction = std::max(collision.Penetration - slop, 0.0f) / totalInvMass * correctionPercent;
            glm::vec3 correctionVec = collision.Normal * correction;

            bodyA->Position -= correctionVec * bodyA->InverseMass;
            bodyB->Position += correctionVec * bodyB->InverseMass;
        }
    }
}

void PhysicsWorld::UpdateSleepState(float dt) {
    for (auto& body : m_Bodies) {
        if (body.IsStatic || body.IsKinematic)
            continue;

        float energy = glm::dot(body.Velocity, body.Velocity) + glm::dot(body.AngularVelocity, body.AngularVelocity);

        if (energy < body.SleepThreshold) {
            body.SleepTimer += dt;
            if (body.SleepTimer >= body.SleepTimeout) {
                body.IsSleeping = true;
                body.Velocity = glm::vec3(0.0f);
                body.AngularVelocity = glm::vec3(0.0f);
            }
        } else {
            body.SleepTimer = 0.0f;
            body.IsSleeping = false;
        }
    }
}

AABB PhysicsWorld::GetBodyAABB(const RigidBodyData& body) const {
    AABB aabb;
    if (body.Shape == ColliderType::Sphere) {
        float r = body.ColliderRadius;
        aabb.Min = body.Position - glm::vec3(r);
        aabb.Max = body.Position + glm::vec3(r);
    } else {
        glm::vec3 half = body.ColliderSize * 0.5f;
        aabb.Min = body.Position - half;
        aabb.Max = body.Position + half;
    }
    return aabb;
}

RaycastHit PhysicsWorld::Raycast(const Ray& ray) const {
    RaycastHit closest;
    closest.Distance = std::numeric_limits<float>::max();

    for (const auto& body : m_Bodies) {
        float t = 0.0f;
        bool hit = false;

        if (body.Shape == ColliderType::Sphere) {
            hit = ray.IntersectsSphere(body.Position, body.ColliderRadius, t);
        } else {
            AABB aabb = GetBodyAABB(body);
            hit = ray.IntersectsAABB(aabb, t);
        }

        if (hit && t < closest.Distance) {
            closest.Hit = true;
            closest.Distance = t;
            closest.Point = ray.GetPoint(t);
            closest.EntityID = body.EntityID;
            closest.Normal = glm::normalize(closest.Point - body.Position);
        }
    }

    return closest;
}

std::vector<RaycastHit> PhysicsWorld::RaycastAll(const Ray& ray) const {
    std::vector<RaycastHit> hits;

    for (const auto& body : m_Bodies) {
        float t = 0.0f;
        bool hit = false;

        if (body.Shape == ColliderType::Sphere) {
            hit = ray.IntersectsSphere(body.Position, body.ColliderRadius, t);
        } else {
            AABB aabb = GetBodyAABB(body);
            hit = ray.IntersectsAABB(aabb, t);
        }

        if (hit) {
            RaycastHit h;
            h.Hit = true;
            h.Distance = t;
            h.Point = ray.GetPoint(t);
            h.EntityID = body.EntityID;
            h.Normal = glm::normalize(h.Point - body.Position);
            hits.push_back(h);
        }
    }

    std::sort(hits.begin(), hits.end(),
              [](const RaycastHit& a, const RaycastHit& b) { return a.Distance < b.Distance; });

    return hits;
}

std::vector<uint32_t> PhysicsWorld::OverlapSphere(const glm::vec3& center, float radius) const {
    std::vector<uint32_t> result;
    for (const auto& body : m_Bodies) {
        AABB aabb = GetBodyAABB(body);
        glm::vec3 closest = glm::clamp(center, aabb.Min, aabb.Max);
        float dist = glm::distance(center, closest);
        if (dist <= radius) {
            result.push_back(body.EntityID);
        }
    }
    return result;
}

std::vector<uint32_t> PhysicsWorld::OverlapBox(const AABB& box) const {
    std::vector<uint32_t> result;
    for (const auto& body : m_Bodies) {
        AABB aabb = GetBodyAABB(body);
        if (box.Intersects(aabb)) {
            result.push_back(body.EntityID);
        }
    }
    return result;
}

}  // namespace PyEngine
