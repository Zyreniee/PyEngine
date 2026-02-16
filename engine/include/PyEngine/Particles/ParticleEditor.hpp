#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "PyEngine/Particles/ParticleSystem.hpp"

namespace PyEngine {

struct GradientColorKey {
    glm::vec4 Color;
    float Time;  // 0.0 to 1.0
};

struct Gradient {
    std::vector<GradientColorKey> Keys;

    glm::vec4 Evaluate(float time) const {
        if (Keys.empty())
            return glm::vec4(1.0f);
        if (Keys.size() == 1)
            return Keys[0].Color;

        // Find keys to interpolate between
        for (size_t i = 0; i < Keys.size() - 1; i++) {
            if (time >= Keys[i].Time && time <= Keys[i + 1].Time) {
                float t = (time - Keys[i].Time) / (Keys[i + 1].Time - Keys[i].Time);
                return glm::mix(Keys[i].Color, Keys[i + 1].Color, t);
            }
        }
        return Keys.back().Color;
    }
};

struct ParticleEditorSettings {
    ParticleProps Props;
    Gradient ColorOverLifetime;
    bool Loop = true;
    float PlaybackSpeed = 1.0f;
};

class ParticleEditor {
public:
    static void Init();
    static void Shutdown();

    static void OnImGuiRender();

    static ParticleEditorSettings& GetSettings() { return s_Settings; }

private:
    static ParticleEditorSettings s_Settings;
};

}  // namespace PyEngine
