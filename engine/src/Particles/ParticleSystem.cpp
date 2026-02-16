#include "PyEngine/Particles/ParticleSystem.hpp"

namespace PyEngine {

ParticleEmitter::ParticleEmitter() {
    m_Particles.resize(MaxParticles);

    // Default force: gravity
    ParticleForce gravity;
    gravity.ForceType = ParticleForce::Type::Gravity;
    gravity.Direction = glm::vec3(0.0f, -9.81f, 0.0f);
    gravity.Strength = 1.0f;
    Forces.push_back(gravity);
}

void ParticleEmitter::Clear() {
    for (auto& p : m_Particles) {
        p.Active = false;
    }
    m_ActiveCount = 0;
    m_EmitAccumulator = 0.0f;
    m_ElapsedTime = 0.0f;
    m_Stats = {};
}

void ParticleEmitter::Burst(int count) {
    for (int i = 0; i < count; i++) {
        EmitParticle();
    }
}

void ParticleEmitter::Update(float deltaTime) {
    float dt = deltaTime * SimulationSpeed;
    m_ElapsedTime += dt;

    // Check duration
    if (!IsLooping && m_ElapsedTime >= Duration) {
        IsPlaying = false;
    }

    // Emit new particles
    if (IsPlaying) {
        m_EmitAccumulator += EmissionRate * dt;
        while (m_EmitAccumulator >= 1.0f) {
            EmitParticle();
            m_EmitAccumulator -= 1.0f;
        }

        // Burst emission
        if (BurstCount > 0 && BurstInterval > 0.0f) {
            m_BurstTimer += dt;
            if (m_BurstTimer >= BurstInterval) {
                Burst(BurstCount);
                m_BurstTimer = 0.0f;
            }
        }
    }

    // Update existing particles
    m_ActiveCount = 0;
    for (auto& particle : m_Particles) {
        if (!particle.Active)
            continue;

        particle.RemainingLife -= dt;
        if (particle.RemainingLife <= 0.0f) {
            particle.Active = false;
            continue;
        }

        UpdateParticle(particle, dt);
        m_ActiveCount++;
    }

    m_Stats.ActiveParticles = m_ActiveCount;
    m_Stats.SimulationTime = m_ElapsedTime;
}

void ParticleEmitter::EmitParticle() {
    // Find inactive particle
    Particle* particle = nullptr;
    for (auto& p : m_Particles) {
        if (!p.Active) {
            particle = &p;
            break;
        }
    }
    if (!particle)
        return;  // Pool full

    // Initialize particle
    particle->Active = true;
    particle->Lifetime = Lifetime.Evaluate(m_Rng);
    particle->RemainingLife = particle->Lifetime;

    // Position and direction from shape
    glm::vec3 pos, dir;
    ShapeConfig.GeneratePositionAndDirection(m_Rng, pos, dir);
    particle->Position = WorldPosition + pos;

    // Velocity
    float speed = StartSpeed.Evaluate(m_Rng);
    particle->Velocity = dir * speed;
    particle->Acceleration = glm::vec3(0.0f);

    // Visual properties
    particle->StartSize = StartSize.Evaluate(m_Rng);
    particle->EndSize = particle->StartSize * EndSizeMultiplier.Evaluate(m_Rng);
    particle->Size = particle->StartSize;

    particle->Rotation = StartRotation.Evaluate(m_Rng);
    particle->AngularVelocity = AngularVelocity.Evaluate(m_Rng);

    particle->StartColor = StartColor;
    particle->EndColor = EndColor;
    particle->Color = StartColor;

    m_Stats.TotalEmitted++;
}

void ParticleEmitter::UpdateParticle(Particle& p, float dt) {
    float age = p.GetAge();

    // Apply forces
    glm::vec3 totalForce(0.0f);
    for (const auto& force : Forces) {
        totalForce += force.Evaluate(p, m_ElapsedTime);
    }
    p.Acceleration = totalForce;

    // Integration (Verlet)
    p.Velocity += p.Acceleration * dt;
    p.Position += p.Velocity * dt;

    // Rotation
    p.Rotation += p.AngularVelocity * dt;

    // Size over lifetime
    p.Size = glm::mix(p.StartSize, p.EndSize, age);

    // Color over lifetime
    if (UseColorGradient) {
        p.Color = ColorOverLifetime.Evaluate(age);
    } else {
        p.Color = glm::mix(p.StartColor, p.EndColor, age);
    }
}

}  // namespace PyEngine
