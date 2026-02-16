#pragma once

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>
#include <vector>

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// RenderCommand — A single draw command
// ═══════════════════════════════════════════════════════════════
struct RenderCommand {
    enum class Type {
        DrawMesh,
        DrawInstanced,
        DrawSkybox,
        DrawParticles,
        DrawGizmo,
        DrawDebugLine,
        DrawDebugBox,
        DrawDebugSphere,
        DrawOverlay,
        SetViewport,
        SetScissor,
        BindPipeline,
        BindDescriptorSet,
        PushConstants,
        BeginRenderPass,
        EndRenderPass
    };

    Type CommandType = Type::DrawMesh;

    // Mesh draw data
    uint32_t MeshID = 0;
    uint32_t MaterialID = 0;
    uint32_t InstanceCount = 1;

    // Transform
    glm::mat4 ModelMatrix{1.0f};
    glm::mat4 NormalMatrix{1.0f};

    // Sorting
    float DistanceToCamera = 0.0f;
    int RenderQueue = 2000;  // Opaque: 2000, Transparent: 3000, Overlay: 4000
    int SortingLayer = 0;
    int OrderInLayer = 0;

    // Material override
    glm::vec4 Color{1.0f};
    float Metallic = 0.0f;
    float Roughness = 0.5f;

    // Flags
    bool CastShadows = true;
    bool ReceiveShadows = true;
    bool IsTransparent = false;
    bool IsVisible = true;

    // Entity reference
    uint32_t EntityID = 0;

    // For debug draw
    glm::vec3 DebugStart{0.0f};
    glm::vec3 DebugEnd{0.0f};
    float DebugSize = 0.1f;
};

// ═══════════════════════════════════════════════════════════════
// RenderQueue — Sorts and batches render commands
// ═══════════════════════════════════════════════════════════════
class RenderQueue {
public:
    enum SortMode {
        FrontToBack,  // Opaque objects from camera
        BackToFront,  // Transparent objects
        ByMaterial,   // Minimize state changes
        ByDistance,   // Pure distance sort
        Unsorted      // Draw order as submitted
    };

    RenderQueue() { m_Commands.reserve(4096); }

    void Clear() {
        m_Commands.clear();
        m_Stats = {};
    }

    void Submit(const RenderCommand& cmd) {
        if (!cmd.IsVisible)
            return;
        m_Commands.push_back(cmd);
        m_Stats.SubmittedCommands++;
    }

    void Submit(RenderCommand::Type type, const glm::mat4& modelMatrix, uint32_t meshID, uint32_t materialID,
                uint32_t entityID = 0) {
        RenderCommand cmd;
        cmd.CommandType = type;
        cmd.ModelMatrix = modelMatrix;
        cmd.MeshID = meshID;
        cmd.MaterialID = materialID;
        cmd.EntityID = entityID;
        Submit(cmd);
    }

    void Sort(SortMode mode, const glm::vec3& cameraPosition = glm::vec3(0.0f));

    // Execute all commands
    void Execute();

    // Iterate commands
    const std::vector<RenderCommand>& GetCommands() const { return m_Commands; }

    // Filtering
    std::vector<const RenderCommand*> GetOpaqueCommands() const;
    std::vector<const RenderCommand*> GetTransparentCommands() const;
    std::vector<const RenderCommand*> GetShadowCasters() const;
    std::vector<const RenderCommand*> GetOverlayCommands() const;

    // Stats
    struct Stats {
        uint32_t SubmittedCommands = 0;
        uint32_t DrawCalls = 0;
        uint32_t Batches = 0;
        uint32_t OpaqueCount = 0;
        uint32_t TransparentCount = 0;
        uint32_t ShadowCasterCount = 0;
    };
    const Stats& GetStats() const { return m_Stats; }

private:
    std::vector<RenderCommand> m_Commands;
    Stats m_Stats;
};

// ═══════════════════════════════════════════════════════════════
// Material — PBR material data
// ═══════════════════════════════════════════════════════════════
struct MaterialData {
    std::string Name = "Default";
    uint32_t ID = 0;

    // PBR properties
    glm::vec4 AlbedoColor{1.0f};
    float Metallic = 0.0f;
    float Roughness = 0.5f;
    float AmbientOcclusion = 1.0f;
    glm::vec3 EmissiveColor{0.0f};
    float EmissiveStrength = 0.0f;
    float NormalStrength = 1.0f;

    // Texture IDs
    uint32_t AlbedoTexture = 0;
    uint32_t NormalTexture = 0;
    uint32_t MetallicTexture = 0;
    uint32_t RoughnessTexture = 0;
    uint32_t AOTexture = 0;
    uint32_t EmissiveTexture = 0;
    uint32_t HeightTexture = 0;

    // Rendering
    enum class BlendMode { Opaque, AlphaTest, AlphaBlend, Additive, Multiply } Blend = BlendMode::Opaque;
    enum class CullMode { None, Front, Back } Cull = CullMode::Back;
    bool DepthWrite = true;
    bool DepthTest = true;
    float AlphaCutoff = 0.5f;

    // Tiling
    glm::vec2 Tiling{1.0f};
    glm::vec2 Offset{0.0f};

    // Shader
    uint32_t ShaderID = 0;
    int RenderQueue = 2000;

    bool IsTransparent() const { return Blend != BlendMode::Opaque && Blend != BlendMode::AlphaTest; }
};

// ═══════════════════════════════════════════════════════════════
// Light — Light source data
// ═══════════════════════════════════════════════════════════════
struct LightData {
    enum class Type { Directional, Point, Spot, Area } LightType = Type::Directional;

    glm::vec3 Position{0.0f, 5.0f, 0.0f};
    glm::vec3 Direction{0.0f, -1.0f, 0.0f};
    glm::vec3 Color{1.0f};
    float Intensity = 1.0f;

    // Point/spot
    float Range = 10.0f;
    float InnerConeAngle = 15.0f;
    float OuterConeAngle = 30.0f;

    // Area
    glm::vec2 AreaSize{1.0f};

    // Shadows
    bool CastShadows = true;
    int ShadowMapResolution = 2048;
    float ShadowBias = 0.005f;
    float ShadowNormalBias = 0.02f;
    float ShadowNearPlane = 0.1f;
    float ShadowFarPlane = 50.0f;

    // Culling
    uint32_t CullingMask = 0xFFFFFFFF;

    // Cookie/Projection
    uint32_t CookieTexture = 0;

    float GetAttenuation(float distance) const {
        if (distance >= Range)
            return 0.0f;
        float att = 1.0f / (1.0f + distance * 0.09f + distance * distance * 0.032f);
        float rangeAtt = 1.0f - std::pow(std::clamp(distance / Range, 0.0f, 1.0f), 2.0f);
        return att * rangeAtt * Intensity;
    }

    float GetSpotAttenuation(const glm::vec3& spotDir, const glm::vec3& lightToPoint) const {
        float theta = glm::dot(glm::normalize(lightToPoint), glm::normalize(spotDir));
        float innerCos = std::cos(glm::radians(InnerConeAngle));
        float outerCos = std::cos(glm::radians(OuterConeAngle));
        float epsilon = innerCos - outerCos;
        return std::clamp((theta - outerCos) / epsilon, 0.0f, 1.0f);
    }

    glm::mat4 GetViewMatrix() const { return glm::lookAt(Position, Position + Direction, glm::vec3(0, 1, 0)); }

    glm::mat4 GetProjectionMatrix() const {
        if (LightType == Type::Directional) {
            return glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, ShadowNearPlane, ShadowFarPlane);
        }
        float fov = glm::radians(OuterConeAngle * 2.0f);
        return glm::perspective(fov, 1.0f, ShadowNearPlane, ShadowFarPlane);
    }

    glm::mat4 GetLightSpaceMatrix() const { return GetProjectionMatrix() * GetViewMatrix(); }
};

// ═══════════════════════════════════════════════════════════════
// PostProcessSettings — Post-processing pipeline config
// ═══════════════════════════════════════════════════════════════
struct PostProcessSettings {
    // Bloom
    bool BloomEnabled = false;
    float BloomThreshold = 1.0f;
    float BloomIntensity = 1.0f;
    float BloomRadius = 4.0f;
    int BloomIterations = 5;

    // Tone mapping
    enum class ToneMapper { None, Reinhard, ACES, Filmic, Uncharted2 } ToneMapping = ToneMapper::ACES;
    float Exposure = 1.0f;
    float Gamma = 2.2f;

    // Ambient Occlusion
    bool SSAOEnabled = false;
    float SSAORadius = 0.5f;
    float SSAOBias = 0.025f;
    float SSAOIntensity = 1.0f;
    int SSAOKernelSize = 64;

    // Color grading
    float Contrast = 1.0f;
    float Saturation = 1.0f;
    float Temperature = 0.0f;
    float Tint = 0.0f;
    glm::vec3 ColorFilter{1.0f};

    // Vignette
    bool VignetteEnabled = false;
    float VignetteIntensity = 0.3f;
    float VignetteSmoothness = 0.5f;
    glm::vec3 VignetteColor{0.0f};

    // Film grain
    bool FilmGrainEnabled = false;
    float FilmGrainIntensity = 0.1f;
    float FilmGrainResponse = 0.8f;

    // Chromatic aberration
    bool ChromaticAberrationEnabled = false;
    float ChromaticAberrationIntensity = 0.1f;

    // Depth of Field
    bool DOFEnabled = false;
    float DOFFocusDistance = 10.0f;
    float DOFAperture = 5.6f;
    float DOFFocalLength = 50.0f;

    // Motion Blur
    bool MotionBlurEnabled = false;
    float MotionBlurIntensity = 0.5f;
    int MotionBlurSamples = 8;

    // Fog
    bool FogEnabled = false;
    enum class FogMode { Linear, Exponential, ExponentialSquared } FogType = FogMode::ExponentialSquared;
    glm::vec3 FogColor{0.5f, 0.6f, 0.7f};
    float FogDensity = 0.02f;
    float FogStart = 10.0f;
    float FogEnd = 100.0f;

    // Anti-aliasing
    enum class AAMode { None, FXAA, SMAA, TAA, MSAA2x, MSAA4x, MSAA8x } AntiAliasing = AAMode::FXAA;

    // Outline
    bool OutlineEnabled = false;
    float OutlineWidth = 1.0f;
    glm::vec4 OutlineColor{1.0f, 0.5f, 0.0f, 1.0f};
};

// ═══════════════════════════════════════════════════════════════
// PostProcessStack — Configurable post-processing
// ═══════════════════════════════════════════════════════════════
class PostProcessStack {
public:
    PostProcessStack() = default;

    PostProcessSettings& GetSettings() { return m_Settings; }
    const PostProcessSettings& GetSettings() const { return m_Settings; }

    // Apply individual effects
    glm::vec3 ApplyToneMapping(const glm::vec3& color) const;
    float ApplyVignette(const glm::vec2& uv) const;
    glm::vec3 ApplyColorGrading(const glm::vec3& color) const;
    float CalculateFog(float distance) const;

    // Presets
    void SetCinematicPreset();
    void SetGamePreset();
    void SetRealisticPreset();
    void ResetToDefaults();

private:
    PostProcessSettings m_Settings;
};

}  // namespace PyEngine
