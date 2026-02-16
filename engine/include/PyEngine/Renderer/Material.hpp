#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace PyEngine {

class Texture2D;

// Like Unity's Material — shader + textures + properties
class Material {
public:
    Material(const std::string& name = "Default Material");
    ~Material() = default;

    // Properties (Unity Standard Shader style)
    glm::vec4 AlbedoColor{1.0f, 1.0f, 1.0f, 1.0f};
    float Metallic = 0.0f;
    float Roughness = 0.5f;
    float AO = 1.0f;  // Ambient Occlusion
    glm::vec3 EmissionColor{0.0f, 0.0f, 0.0f};
    float EmissionIntensity = 0.0f;

    // Textures
    std::shared_ptr<Texture2D> AlbedoMap;
    std::shared_ptr<Texture2D> NormalMap;
    std::shared_ptr<Texture2D> MetallicMap;
    std::shared_ptr<Texture2D> RoughnessMap;
    std::shared_ptr<Texture2D> AOMap;
    std::shared_ptr<Texture2D> EmissionMap;

    // Rendering
    enum class RenderMode : int { Opaque = 0, Cutout = 1, Transparent = 2 };
    RenderMode Mode = RenderMode::Opaque;
    float AlphaCutoff = 0.5f;
    bool DoubleSided = false;

    // Tiling
    glm::vec2 Tiling{1.0f, 1.0f};
    glm::vec2 Offset{0.0f, 0.0f};

    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }

private:
    std::string m_Name;
};

}  // namespace PyEngine
