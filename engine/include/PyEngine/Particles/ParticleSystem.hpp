#pragma once

#include <algorithm>
#include <cmath>
#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <random>
#include <vector>

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// Particle — Single particle data
// ═══════════════════════════════════════════════════════════════
struct Particle {
    glm::vec3 Position{0.0f};
    glm::vec3 Velocity{0.0f};
    glm::vec3 Acceleration{0.0f};
    glm::vec4 Color{1.0f};
    glm::vec4 StartColor{1.0f};
    glm::vec4 EndColor{1.0f, 1.0f, 1.0f, 0.0f};
    float Size = 1.0f;
    float StartSize = 1.0f;
    float EndSize = 0.0f;
    float Rotation = 0.0f;
    float AngularVelocity = 0.0f;
    float Lifetime = 1.0f;
    float RemainingLife = 1.0f;
    bool Active = false;

    float GetAge() const { return 1.0f - (RemainingLife / Lifetime); }
};

// ═══════════════════════════════════════════════════════════════
// RandomRange — Min/Max random value generator
// ═══════════════════════════════════════════════════════════════
struct RandomRange {
    float Min = 0.0f;
    float Max = 1.0f;

    RandomRange() = default;
    RandomRange(float value) : Min(value), Max(value) {}
    RandomRange(float min, float max) : Min(min), Max(max) {}

    float Evaluate(std::mt19937& rng) const {
        std::uniform_real_distribution<float> dist(Min, Max);
        return dist(rng);
    }
};

struct RandomRangeVec3 {
    glm::vec3 Min{0.0f};
    glm::vec3 Max{0.0f};

    RandomRangeVec3() = default;
    RandomRangeVec3(const glm::vec3& value) : Min(value), Max(value) {}
    RandomRangeVec3(const glm::vec3& min, const glm::vec3& max) : Min(min), Max(max) {}

    glm::vec3 Evaluate(std::mt19937& rng) const {
        std::uniform_real_distribution<float> distX(Min.x, Max.x);
        std::uniform_real_distribution<float> distY(Min.y, Max.y);
        std::uniform_real_distribution<float> distZ(Min.z, Max.z);
        return {distX(rng), distY(rng), distZ(rng)};
    }
};

// ═══════════════════════════════════════════════════════════════
// ColorGradient — Color over lifetime
// ═══════════════════════════════════════════════════════════════
struct ColorGradientKey {
    float Time = 0.0f;
    glm::vec4 Color{1.0f};
};

struct ColorGradient {
    std::vector<ColorGradientKey> Keys;

    ColorGradient() {
        Keys.push_back({0.0f, {1.0f, 1.0f, 1.0f, 1.0f}});
        Keys.push_back({1.0f, {1.0f, 1.0f, 1.0f, 0.0f}});
    }

    glm::vec4 Evaluate(float t) const {
        if (Keys.empty())
            return glm::vec4(1.0f);
        if (Keys.size() == 1)
            return Keys[0].Color;

        t = std::clamp(t, 0.0f, 1.0f);

        if (t <= Keys.front().Time)
            return Keys.front().Color;
        if (t >= Keys.back().Time)
            return Keys.back().Color;

        for (size_t i = 0; i < Keys.size() - 1; i++) {
            if (t >= Keys[i].Time && t <= Keys[i + 1].Time) {
                float factor = (t - Keys[i].Time) / (Keys[i + 1].Time - Keys[i].Time);
                return glm::mix(Keys[i].Color, Keys[i + 1].Color, factor);
            }
        }
        return Keys.back().Color;
    }
};

// ═══════════════════════════════════════════════════════════════
// EmitterShape
// ═══════════════════════════════════════════════════════════════
enum class EmitterShape { Point, Sphere, Hemisphere, Cone, Box, Circle, Edge };

struct EmitterShapeConfig {
    EmitterShape Shape = EmitterShape::Cone;
    float Radius = 1.0f;
    float Angle = 25.0f;  // Cone angle in degrees
    glm::vec3 BoxSize{1.0f};
    float ArcDegrees = 360.0f;
    bool EmitFromEdge = false;

    void GeneratePositionAndDirection(std::mt19937& rng, glm::vec3& outPos, glm::vec3& outDir) const {
        std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
        std::uniform_real_distribution<float> distAngle(0.0f, glm::radians(ArcDegrees));
        std::uniform_real_distribution<float> distFull(-1.0f, 1.0f);

        switch (Shape) {
            case EmitterShape::Point:
                outPos = glm::vec3(0.0f);
                outDir = glm::vec3(0.0f, 1.0f, 0.0f);
                break;
            case EmitterShape::Sphere: {
                float theta = distAngle(rng);
                float phi = std::acos(distFull(rng));
                float r = EmitFromEdge ? Radius : Radius * std::cbrt(dist01(rng));
                outPos = glm::vec3(r * std::sin(phi) * std::cos(theta), r * std::cos(phi),
                                   r * std::sin(phi) * std::sin(theta));
                outDir = glm::normalize(outPos);
                if (glm::length(outDir) < 0.001f)
                    outDir = glm::vec3(0.0f, 1.0f, 0.0f);
                break;
            }
            case EmitterShape::Hemisphere: {
                float theta = distAngle(rng);
                float phi = std::acos(dist01(rng));
                float r = EmitFromEdge ? Radius : Radius * std::cbrt(dist01(rng));
                outPos = glm::vec3(r * std::sin(phi) * std::cos(theta), r * std::cos(phi),
                                   r * std::sin(phi) * std::sin(theta));
                outDir = glm::normalize(outPos);
                break;
            }
            case EmitterShape::Cone: {
                float theta = distAngle(rng);
                float coneAngle = glm::radians(Angle) * dist01(rng);
                float r = EmitFromEdge ? Radius : Radius * dist01(rng);
                outPos = glm::vec3(r * std::cos(theta), 0.0f, r * std::sin(theta));
                outDir = glm::normalize(glm::vec3(std::sin(coneAngle) * std::cos(theta), std::cos(coneAngle),
                                                  std::sin(coneAngle) * std::sin(theta)));
                break;
            }
            case EmitterShape::Box: {
                std::uniform_real_distribution<float> dx(-BoxSize.x * 0.5f, BoxSize.x * 0.5f);
                std::uniform_real_distribution<float> dy(-BoxSize.y * 0.5f, BoxSize.y * 0.5f);
                std::uniform_real_distribution<float> dz(-BoxSize.z * 0.5f, BoxSize.z * 0.5f);
                outPos = glm::vec3(dx(rng), dy(rng), dz(rng));
                outDir = glm::vec3(0.0f, 1.0f, 0.0f);
                break;
            }
            case EmitterShape::Circle: {
                float theta = distAngle(rng);
                float r = EmitFromEdge ? Radius : Radius * std::sqrt(dist01(rng));
                outPos = glm::vec3(r * std::cos(theta), 0.0f, r * std::sin(theta));
                outDir = glm::vec3(0.0f, 1.0f, 0.0f);
                break;
            }
            case EmitterShape::Edge: {
                std::uniform_real_distribution<float> dx(-Radius, Radius);
                outPos = glm::vec3(dx(rng), 0.0f, 0.0f);
                outDir = glm::vec3(0.0f, 1.0f, 0.0f);
                break;
            }
        }
    }
};

// ═══════════════════════════════════════════════════════════════
// ParticleForce — Forces applied to particles
// ═══════════════════════════════════════════════════════════════
struct ParticleForce {
    enum class Type { Gravity, Wind, Turbulence, Vortex, Attraction, Drag };

    Type ForceType = Type::Gravity;
    glm::vec3 Direction{0.0f, -9.81f, 0.0f};
    float Strength = 1.0f;
    glm::vec3 Position{0.0f};  // For attraction/vortex
    float Radius = 5.0f;
    float Frequency = 1.0f;  // For turbulence

    glm::vec3 Evaluate(const Particle& particle, float time) const {
        switch (ForceType) {
            case Type::Gravity:
                return Direction * Strength;
            case Type::Wind:
                return Direction * Strength;
            case Type::Turbulence: {
                float noise =
                    std::sin(particle.Position.x * Frequency + time) * std::cos(particle.Position.z * Frequency + time);
                return glm::vec3(noise, 0.0f, noise) * Strength;
            }
            case Type::Vortex: {
                glm::vec3 toCenter = Position - particle.Position;
                float dist = glm::length(toCenter);
                if (dist < 0.01f || dist > Radius)
                    return glm::vec3(0.0f);
                glm::vec3 tangent = glm::normalize(glm::cross(toCenter, glm::vec3(0, 1, 0)));
                return tangent * (Strength / dist);
            }
            case Type::Attraction: {
                glm::vec3 toCenter = Position - particle.Position;
                float dist = glm::length(toCenter);
                if (dist < 0.01f)
                    return glm::vec3(0.0f);
                return glm::normalize(toCenter) * (Strength / (dist * dist));
            }
            case Type::Drag:
                return -particle.Velocity * Strength;
        }
        return glm::vec3(0.0f);
    }
};

// ═══════════════════════════════════════════════════════════════
// ParticleEmitter — Main particle emitter
// ═══════════════════════════════════════════════════════════════
class ParticleEmitter {
public:
    // Configuration
    std::string Name = "Particle System";
    uint32_t MaxParticles = 1000;
    float EmissionRate = 50.0f;
    int BurstCount = 0;
    float BurstInterval = 0.0f;

    RandomRange Lifetime{1.0f, 3.0f};
    RandomRange StartSpeed{2.0f, 5.0f};
    RandomRange StartSize{0.1f, 0.5f};
    RandomRange EndSizeMultiplier{0.0f, 0.2f};
    RandomRange StartRotation{0.0f, 360.0f};
    RandomRange AngularVelocity{-90.0f, 90.0f};

    glm::vec4 StartColor{1.0f, 0.8f, 0.2f, 1.0f};
    glm::vec4 EndColor{1.0f, 0.2f, 0.0f, 0.0f};
    ColorGradient ColorOverLifetime;
    bool UseColorGradient = false;

    EmitterShapeConfig ShapeConfig;
    std::vector<ParticleForce> Forces;

    glm::vec3 WorldPosition{0.0f};
    bool IsPlaying = true;
    bool IsLooping = true;
    float Duration = 5.0f;
    float SimulationSpeed = 1.0f;

    // Space
    enum class SimulationSpace { Local, World } Space = SimulationSpace::World;

    ParticleEmitter();

    void Play() {
        IsPlaying = true;
        m_ElapsedTime = 0.0f;
    }
    void Stop() { IsPlaying = false; }
    void Pause() { IsPlaying = false; }
    void Clear();
    void Burst(int count);

    void Update(float deltaTime);

    // Stats
    uint32_t GetActiveParticleCount() const { return m_ActiveCount; }
    const std::vector<Particle>& GetParticles() const { return m_Particles; }

    struct Stats {
        uint32_t ActiveParticles = 0;
        uint32_t TotalEmitted = 0;
        float SimulationTime = 0.0f;
    };
    const Stats& GetStats() const { return m_Stats; }

private:
    void EmitParticle();
    void UpdateParticle(Particle& p, float dt);

private:
    std::vector<Particle> m_Particles;
    uint32_t m_ActiveCount = 0;
    float m_EmitAccumulator = 0.0f;
    float m_ElapsedTime = 0.0f;
    float m_BurstTimer = 0.0f;
    std::mt19937 m_Rng{42};
    Stats m_Stats;
};

// ═══════════════════════════════════════════════════════════════
// ParticleSystem — Manages multiple emitters
// ═══════════════════════════════════════════════════════════════
class ParticleSystem {
public:
    ParticleSystem() = default;

    ParticleEmitter& AddEmitter(const std::string& name = "Emitter") {
        auto emitter = std::make_unique<ParticleEmitter>();
        emitter->Name = name;
        m_Emitters.push_back(std::move(emitter));
        return *m_Emitters.back();
    }

    void RemoveEmitter(size_t index) {
        if (index < m_Emitters.size()) {
            m_Emitters.erase(m_Emitters.begin() + static_cast<long>(index));
        }
    }

    ParticleEmitter* GetEmitter(size_t index) {
        return (index < m_Emitters.size()) ? m_Emitters[index].get() : nullptr;
    }

    size_t GetEmitterCount() const { return m_Emitters.size(); }

    void Update(float deltaTime) {
        for (auto& emitter : m_Emitters) {
            emitter->Update(deltaTime);
        }
    }

    void PlayAll() {
        for (auto& e : m_Emitters)
            e->Play();
    }
    void StopAll() {
        for (auto& e : m_Emitters)
            e->Stop();
    }
    void ClearAll() {
        for (auto& e : m_Emitters)
            e->Clear();
    }

    uint32_t GetTotalActiveParticles() const {
        uint32_t total = 0;
        for (const auto& e : m_Emitters)
            total += e->GetActiveParticleCount();
        return total;
    }

private:
    std::vector<std::unique_ptr<ParticleEmitter>> m_Emitters;
};

}  // namespace PyEngine
