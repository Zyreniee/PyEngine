#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// Material Definition
// ═══════════════════════════════════════════════════════════════

enum class ShaderType { StandardPBR, Unlit, Transparent, Particles, Skybox, UI };

struct TextureMap {
    std::string Path;
    uint32_t ID = 0;
    bool Srgb = true;
};

struct MaterialInstance {
    std::string Name = "New Material";
    uint32_t ID = 0;
    ShaderType Type = ShaderType::StandardPBR;

    // PBR Parameters
    glm::vec4 AlbedoColor{1.0f};
    float Metallic = 0.0f;
    float Roughness = 0.5f;
    float Emission = 0.0f;
    glm::vec3 EmissiveColor{0.0f};

    // Texture Maps
    TextureMap AlbedoMap;
    TextureMap NormalMap;
    TextureMap MetallicMap;
    TextureMap RoughnessMap;
    TextureMap AmbientOcclusionMap;
    TextureMap EmissiveMap;

    // Tiling & Offset
    glm::vec2 Tiling{1.0f};
    glm::vec2 Offset{0.0f};

    // Render State
    bool TwoSided = false;
    bool Transparent = false;
    bool CastShadows = true;

    // Shader Properties (Uniforms)
    std::unordered_map<std::string, float> FloatProps;
    std::unordered_map<std::string, glm::vec4> VectorProps;
    std::unordered_map<std::string, int> IntProps;
};

// ═══════════════════════════════════════════════════════════════
// Material Library — Manages materials
// ═══════════════════════════════════════════════════════════════

class MaterialLibrary {
public:
    static MaterialLibrary& Get() {
        static MaterialLibrary instance;
        return instance;
    }

    std::shared_ptr<MaterialInstance> CreateMaterial(const std::string& name);
    std::shared_ptr<MaterialInstance> GetMaterial(const std::string& name);
    std::shared_ptr<MaterialInstance> GetMaterial(uint32_t id);

    void AddMaterial(std::shared_ptr<MaterialInstance> material);
    void RemoveMaterial(const std::string& name);

    // Defaults
    std::shared_ptr<MaterialInstance> GetDefaultMaterial();
    std::shared_ptr<MaterialInstance> GetErrorMaterial();

    const std::unordered_map<std::string, std::shared_ptr<MaterialInstance>>& GetAllMaterials() const {
        return m_Materials;
    }

private:
    MaterialLibrary();

private:
    std::unordered_map<std::string, std::shared_ptr<MaterialInstance>> m_Materials;
    std::unordered_map<uint32_t, std::shared_ptr<MaterialInstance>> m_MaterialsByID;

    std::shared_ptr<MaterialInstance> m_DefaultMaterial;
    std::shared_ptr<MaterialInstance> m_ErrorMaterial;
    uint32_t m_NextID = 1;
};

}  // namespace PyEngine
