#include "PyEngine/Renderer/RenderQueue.hpp"

namespace PyEngine {

void RenderQueue::Sort(SortMode mode, const glm::vec3& cameraPosition) {
    // Calculate distance for each command
    if (mode == SortMode::FrontToBack || mode == SortMode::BackToFront || mode == SortMode::ByDistance) {
        for (auto& cmd : m_Commands) {
            glm::vec3 pos(cmd.ModelMatrix[3]);
            cmd.DistanceToCamera = glm::distance(cameraPosition, pos);
        }
    }

    switch (mode) {
        case SortMode::FrontToBack:
            std::sort(m_Commands.begin(), m_Commands.end(), [](const RenderCommand& a, const RenderCommand& b) {
                if (a.RenderQueue != b.RenderQueue)
                    return a.RenderQueue < b.RenderQueue;
                return a.DistanceToCamera < b.DistanceToCamera;
            });
            break;

        case SortMode::BackToFront:
            std::sort(m_Commands.begin(), m_Commands.end(), [](const RenderCommand& a, const RenderCommand& b) {
                if (a.RenderQueue != b.RenderQueue)
                    return a.RenderQueue < b.RenderQueue;
                return a.DistanceToCamera > b.DistanceToCamera;
            });
            break;

        case SortMode::ByMaterial:
            std::sort(m_Commands.begin(), m_Commands.end(), [](const RenderCommand& a, const RenderCommand& b) {
                if (a.RenderQueue != b.RenderQueue)
                    return a.RenderQueue < b.RenderQueue;
                if (a.MaterialID != b.MaterialID)
                    return a.MaterialID < b.MaterialID;
                return a.MeshID < b.MeshID;
            });
            break;

        case SortMode::ByDistance:
            std::sort(m_Commands.begin(), m_Commands.end(), [](const RenderCommand& a, const RenderCommand& b) {
                return a.DistanceToCamera < b.DistanceToCamera;
            });
            break;

        case SortMode::Unsorted:
            break;
    }
}

void RenderQueue::Execute() {
    m_Stats.DrawCalls = 0;
    m_Stats.Batches = 0;
    m_Stats.OpaqueCount = 0;
    m_Stats.TransparentCount = 0;
    m_Stats.ShadowCasterCount = 0;

    uint32_t lastMaterialID = UINT32_MAX;

    for (const auto& cmd : m_Commands) {
        if (cmd.CommandType == RenderCommand::Type::DrawMesh || cmd.CommandType == RenderCommand::Type::DrawInstanced) {
            m_Stats.DrawCalls++;

            if (cmd.MaterialID != lastMaterialID) {
                m_Stats.Batches++;
                lastMaterialID = cmd.MaterialID;
            }

            if (cmd.IsTransparent)
                m_Stats.TransparentCount++;
            else
                m_Stats.OpaqueCount++;

            if (cmd.CastShadows)
                m_Stats.ShadowCasterCount++;
        }
    }
}

std::vector<const RenderCommand*> RenderQueue::GetOpaqueCommands() const {
    std::vector<const RenderCommand*> result;
    for (const auto& cmd : m_Commands) {
        if (!cmd.IsTransparent && cmd.RenderQueue < 3000) {
            result.push_back(&cmd);
        }
    }
    return result;
}

std::vector<const RenderCommand*> RenderQueue::GetTransparentCommands() const {
    std::vector<const RenderCommand*> result;
    for (const auto& cmd : m_Commands) {
        if (cmd.IsTransparent || (cmd.RenderQueue >= 3000 && cmd.RenderQueue < 4000)) {
            result.push_back(&cmd);
        }
    }
    return result;
}

std::vector<const RenderCommand*> RenderQueue::GetShadowCasters() const {
    std::vector<const RenderCommand*> result;
    for (const auto& cmd : m_Commands) {
        if (cmd.CastShadows && !cmd.IsTransparent) {
            result.push_back(&cmd);
        }
    }
    return result;
}

std::vector<const RenderCommand*> RenderQueue::GetOverlayCommands() const {
    std::vector<const RenderCommand*> result;
    for (const auto& cmd : m_Commands) {
        if (cmd.RenderQueue >= 4000) {
            result.push_back(&cmd);
        }
    }
    return result;
}

// ── PostProcessStack ─────────────────────────────────────────

glm::vec3 PostProcessStack::ApplyToneMapping(const glm::vec3& color) const {
    glm::vec3 result = color * m_Settings.Exposure;

    switch (m_Settings.ToneMapping) {
        case PostProcessSettings::ToneMapper::None:
            return result;

        case PostProcessSettings::ToneMapper::Reinhard:
            return result / (result + glm::vec3(1.0f));

        case PostProcessSettings::ToneMapper::ACES: {
            const float a = 2.51f;
            const float b = 0.03f;
            const float c = 2.43f;
            const float d = 0.59f;
            const float e = 0.14f;
            return glm::clamp((result * (a * result + b)) / (result * (c * result + d) + e), glm::vec3(0.0f),
                              glm::vec3(1.0f));
        }

        case PostProcessSettings::ToneMapper::Filmic: {
            glm::vec3 x = glm::max(glm::vec3(0.0f), result - glm::vec3(0.004f));
            return (x * (6.2f * x + glm::vec3(0.5f))) / (x * (6.2f * x + glm::vec3(1.7f)) + glm::vec3(0.06f));
        }

        case PostProcessSettings::ToneMapper::Uncharted2: {
            auto tonemap = [](const glm::vec3& x) {
                const float A = 0.15f, B = 0.50f, C = 0.10f, D = 0.20f, E = 0.02f, F = 0.30f;
                return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
            };
            glm::vec3 curr = tonemap(result * 2.0f);
            glm::vec3 whiteScale = glm::vec3(1.0f) / tonemap(glm::vec3(11.2f));
            return curr * whiteScale;
        }
    }
    return result;
}

float PostProcessStack::ApplyVignette(const glm::vec2& uv) const {
    if (!m_Settings.VignetteEnabled)
        return 1.0f;

    glm::vec2 centered = uv - glm::vec2(0.5f);
    float dist = glm::length(centered) * 2.0f;
    float vignette = 1.0f - glm::smoothstep(1.0f - m_Settings.VignetteIntensity, 1.0f, dist);
    return glm::mix(1.0f, vignette, m_Settings.VignetteIntensity);
}

glm::vec3 PostProcessStack::ApplyColorGrading(const glm::vec3& color) const {
    glm::vec3 result = color;

    // Contrast
    result = (result - 0.5f) * m_Settings.Contrast + 0.5f;

    // Saturation
    float luminance = glm::dot(result, glm::vec3(0.2126f, 0.7152f, 0.0722f));
    result = glm::mix(glm::vec3(luminance), result, m_Settings.Saturation);

    // Temperature (warm/cool shift)
    if (std::abs(m_Settings.Temperature) > 0.001f) {
        result.r += m_Settings.Temperature * 0.1f;
        result.b -= m_Settings.Temperature * 0.1f;
    }

    // Color filter
    result *= m_Settings.ColorFilter;

    // Gamma correction
    result = glm::pow(glm::max(result, glm::vec3(0.0f)), glm::vec3(1.0f / m_Settings.Gamma));

    return glm::clamp(result, glm::vec3(0.0f), glm::vec3(1.0f));
}

float PostProcessStack::CalculateFog(float distance) const {
    if (!m_Settings.FogEnabled)
        return 0.0f;

    switch (m_Settings.FogType) {
        case PostProcessSettings::FogMode::Linear:
            return std::clamp((distance - m_Settings.FogStart) / (m_Settings.FogEnd - m_Settings.FogStart), 0.0f, 1.0f);
        case PostProcessSettings::FogMode::Exponential:
            return 1.0f - std::exp(-m_Settings.FogDensity * distance);
        case PostProcessSettings::FogMode::ExponentialSquared:
            return 1.0f - std::exp(-m_Settings.FogDensity * m_Settings.FogDensity * distance * distance);
    }
    return 0.0f;
}

void PostProcessStack::SetCinematicPreset() {
    m_Settings.ToneMapping = PostProcessSettings::ToneMapper::ACES;
    m_Settings.Exposure = 1.2f;
    m_Settings.BloomEnabled = true;
    m_Settings.BloomIntensity = 0.8f;
    m_Settings.BloomThreshold = 1.5f;
    m_Settings.VignetteEnabled = true;
    m_Settings.VignetteIntensity = 0.4f;
    m_Settings.FilmGrainEnabled = true;
    m_Settings.FilmGrainIntensity = 0.05f;
    m_Settings.DOFEnabled = true;
    m_Settings.DOFFocusDistance = 5.0f;
    m_Settings.Contrast = 1.1f;
    m_Settings.Saturation = 0.95f;
    m_Settings.AntiAliasing = PostProcessSettings::AAMode::TAA;
}

void PostProcessStack::SetGamePreset() {
    m_Settings.ToneMapping = PostProcessSettings::ToneMapper::ACES;
    m_Settings.Exposure = 1.0f;
    m_Settings.BloomEnabled = true;
    m_Settings.BloomIntensity = 0.5f;
    m_Settings.VignetteEnabled = false;
    m_Settings.FilmGrainEnabled = false;
    m_Settings.DOFEnabled = false;
    m_Settings.MotionBlurEnabled = false;
    m_Settings.AntiAliasing = PostProcessSettings::AAMode::FXAA;
}

void PostProcessStack::SetRealisticPreset() {
    m_Settings.ToneMapping = PostProcessSettings::ToneMapper::Filmic;
    m_Settings.Exposure = 1.0f;
    m_Settings.SSAOEnabled = true;
    m_Settings.SSAORadius = 0.5f;
    m_Settings.BloomEnabled = true;
    m_Settings.BloomIntensity = 0.3f;
    m_Settings.FogEnabled = true;
    m_Settings.FogDensity = 0.01f;
    m_Settings.Contrast = 1.05f;
    m_Settings.AntiAliasing = PostProcessSettings::AAMode::TAA;
}

void PostProcessStack::ResetToDefaults() {
    m_Settings = PostProcessSettings();
}

}  // namespace PyEngine
