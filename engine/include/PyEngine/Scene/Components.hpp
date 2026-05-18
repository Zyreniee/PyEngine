#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <string>
#include <vector>

#include "PyEngine/Core/UUID.hpp"

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// ECS Components — All components for the engine
// ═══════════════════════════════════════════════════════════════

class Mesh;  // Forward declare

struct IDComponent {
    UUID ID;

    IDComponent() = default;
    IDComponent(UUID id) : ID(id) {}
};

struct TagComponent {
    std::string Tag = "Entity";
    uint32_t Layer = 0;
    bool IsStatic = false;
    bool IsActive = true;

    TagComponent() = default;
    TagComponent(const std::string& tag) : Tag(tag) {}
};

struct TransformComponent {
    glm::vec3 Position{0.0f};
    glm::vec3 Rotation{0.0f};  // Euler angles in degrees
    glm::vec3 Scale{1.0f};

    glm::mat4 GetTransformMatrix() const {
        glm::mat4 mat = glm::translate(glm::mat4(1.0f), Position);
        mat = glm::rotate(mat, glm::radians(Rotation.x), {1, 0, 0});
        mat = glm::rotate(mat, glm::radians(Rotation.y), {0, 1, 0});
        mat = glm::rotate(mat, glm::radians(Rotation.z), {0, 0, 1});
        mat = glm::scale(mat, Scale);
        return mat;
    }

    glm::vec3 GetForward() const {
        float yaw = glm::radians(Rotation.y);
        float pitch = glm::radians(Rotation.x);
        return glm::normalize(
            glm::vec3(std::cos(pitch) * std::sin(yaw), std::sin(pitch), std::cos(pitch) * std::cos(yaw)));
    }

    glm::vec3 GetRight() const { return glm::normalize(glm::cross(GetForward(), glm::vec3(0, 1, 0))); }

    glm::vec3 GetUp() const { return glm::normalize(glm::cross(GetRight(), GetForward())); }
};

struct MeshRendererComponent {
    uint32_t MeshID = 0;
    uint32_t MaterialID = 0;
    bool Visible = true;
    bool CastShadows = true;
    bool ReceiveShadows = true;
    int RenderQueue = 2000;
    int SortingLayer = 0;
    int OrderInLayer = 0;

    glm::vec4 ColorTint{1.0f};

    // PBR Material Properties
    float Metallic = 0.0f;
    float Roughness = 0.5f;
    float AO = 1.0f;

    // For dynamic meshes (Terrain, Procedural, etc.)
    std::shared_ptr<Mesh> CustomMesh;
};

struct CameraComponent {
    enum class ProjectionType { Perspective, Orthographic } Projection = ProjectionType::Perspective;
    float FieldOfView = 60.0f;
    float NearPlane = 0.1f;
    float FarPlane = 1000.0f;
    float OrthographicSize = 5.0f;
    bool IsPrimary = true;
    bool& Primary = IsPrimary;  // backward compat alias
    glm::vec4 ClearColor{0.1f, 0.1f, 0.12f, 1.0f};
    int Depth = 0;
    uint32_t CullingMask = 0xFFFFFFFF;

    glm::mat4 GetProjectionMatrix(float aspectRatio) const {
        if (Projection == ProjectionType::Perspective) {
            return glm::perspective(glm::radians(FieldOfView), aspectRatio, NearPlane, FarPlane);
        }
        float halfWidth = OrthographicSize * aspectRatio;
        float halfHeight = OrthographicSize;
        return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, NearPlane, FarPlane);
    }
};

struct LightComponent {
    enum class Type { Directional, Point, Spot, Area } LightType = Type::Directional;
    glm::vec3 Color{1.0f};
    float Intensity = 1.0f;
    float Range = 10.0f;
    float InnerConeAngle = 15.0f;
    float OuterConeAngle = 30.0f;
    glm::vec2 AreaSize{1.0f};
    bool CastShadows = true;
    int ShadowResolution = 2048;
    float ShadowBias = 0.005f;
    uint32_t CullingMask = 0xFFFFFFFF;
};

struct RigidBodyComponent {
    enum class BodyType { Static, Dynamic, Kinematic } Type = BodyType::Dynamic;
    float Mass = 1.0f;
    float LinearDamping = 0.01f;
    float AngularDamping = 0.05f;
    bool UseGravity = true;
    bool FreezeRotationX = false;
    bool FreezeRotationY = false;
    bool FreezeRotationZ = false;

    glm::vec3 Velocity{0.0f};
    glm::vec3 AngularVelocity{0.0f};

    float Restitution = 0.3f;
    float StaticFriction = 0.5f;
    float DynamicFriction = 0.3f;
};

struct BoxColliderComponent {
    glm::vec3 Center{0.0f};
    glm::vec3 Size{1.0f};
    bool IsTrigger = false;
    uint32_t PhysicsMaterialID = 0;
};

struct SphereColliderComponent {
    glm::vec3 Center{0.0f};
    float Radius = 0.5f;
    bool IsTrigger = false;
};

struct CapsuleColliderComponent {
    glm::vec3 Center{0.0f};
    float Radius = 0.5f;
    float Height = 2.0f;
    enum class Direction { X, Y, Z } Axis = Direction::Y;
    bool IsTrigger = false;
};

struct AudioSourceComponent {
    uint32_t ClipID = 0;
    std::string ClipName;
    float Volume = 1.0f;
    float Pitch = 1.0f;
    bool Spatial = true;
    bool Loop = false;
    bool PlayOnAwake = true;
    float MinDistance = 1.0f;
    float MaxDistance = 50.0f;
    float DopplerLevel = 1.0f;
    std::string MixerGroup = "SFX";
};

struct AudioListenerComponent {
    bool Active = true;
};

struct ParticleSystemComponent {
    std::string SystemName = "Particles";
    uint32_t MaxParticles = 1000;
    float EmissionRate = 50.0f;
    float Duration = 5.0f;
    bool Looping = true;
    bool PlayOnAwake = true;
    bool SimulateInWorldSpace = true;

    glm::vec4 StartColor{1.0f};
    glm::vec4 EndColor{1.0f, 1.0f, 1.0f, 0.0f};
    float StartSize = 0.5f;
    float EndSize = 0.0f;
    float StartSpeed = 5.0f;
    float StartLifetime = 2.0f;
    float GravityModifier = 1.0f;
};

struct AnimatorComponent {
    uint32_t ControllerID = 0;
    float Speed = 1.0f;
    bool ApplyRootMotion = false;
    std::string CurrentState;
};

struct ScriptComponent {
    std::vector<std::string> ScriptClassNames;
    bool Enabled = true;
};

struct SpriteRendererComponent {
    uint32_t TextureID = 0;
    glm::vec4 Color{1.0f};
    glm::vec2 Tiling{1.0f};
    glm::vec2 Offset{0.0f};
    int SortingOrder = 0;
    std::string SortingLayer = "Default";
    bool FlipX = false;
    bool FlipY = false;
};

struct TextRendererComponent {
    std::string Text = "Text";
    uint32_t FontID = 0;
    float FontSize = 16.0f;
    glm::vec4 Color{1.0f};
    enum class Alignment { Left, Center, Right } Align = Alignment::Left;
    float LineSpacing = 1.0f;
    bool Outline = false;
    float OutlineWidth = 1.0f;
    glm::vec4 OutlineColor{0.0f, 0.0f, 0.0f, 1.0f};
};

struct EnvironmentComponent {
    // Skybox
    uint32_t SkyboxCubemapID = 0;
    glm::vec3 AmbientColor{0.1f, 0.1f, 0.12f};
    float AmbientIntensity = 0.5f;

    // Fog
    bool FogEnabled = false;
    glm::vec3 FogColor{0.5f, 0.6f, 0.7f};
    float FogDensity = 0.02f;
    float FogStart = 10.0f;
    float FogEnd = 100.0f;

    // Reflection
    uint32_t ReflectionProbeID = 0;
    float ReflectionIntensity = 1.0f;
};

struct TerrainComponent {
    uint32_t HeightmapID = 0;
    float Width = 100.0f;
    float Height = 20.0f;
    float Length = 100.0f;
    int Resolution = 256;

    // Layer textures
    std::vector<uint32_t> LayerTextures;
    std::vector<float> LayerScales;

    bool CastShadows = true;
    bool DrawTreesAndGrass = true;
};

struct NavMeshAgentComponent {
    float Speed = 3.5f;
    float AngularSpeed = 120.0f;
    float Acceleration = 8.0f;
    float StoppingDistance = 0.1f;
    float Radius = 0.5f;
    float Height = 2.0f;

    glm::vec3 Destination{0.0f};
    bool HasPath = false;
    bool IsPathPending = false;

    std::vector<glm::vec3> Path;
    int CurrentPathIndex = 0;

    int AreaMask = 0xFFFFFFFF;
};

class PythonScript;  // Forward declare

struct PythonScriptComponent {
    std::string ScriptPath;       // Path to .py file (relative to project)
    bool Enabled = true;
    bool AutoReload = true;       // Hot-reload on file change

    // Runtime state (managed by Scene, not serialized)
    std::shared_ptr<PythonScript> ScriptInstance;
};

}  // namespace PyEngine
