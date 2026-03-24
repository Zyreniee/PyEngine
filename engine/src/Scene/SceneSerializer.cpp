#include "PyEngine/Scene/SceneSerializer.hpp"

#include <glm/glm.hpp>

#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Scene/Components.hpp"
#include "PyEngine/Scene/Entity.hpp"

namespace PyEngine {

// Helper to format vectors
static std::ostream& operator<<(std::ostream& os, const glm::vec3& v) {
    return os << v.x << " " << v.y << " " << v.z;
}
static std::ostream& operator<<(std::ostream& os, const glm::vec4& v) {
    return os << v.x << " " << v.y << " " << v.z << " " << v.w;
}
static std::ostream& operator<<(std::ostream& os, const glm::vec2& v) {
    return os << v.x << " " << v.y;
}

SceneSerializer::SceneSerializer(const std::shared_ptr<Scene>& scene) : m_Scene(scene) {}

void SceneSerializer::Serialize(const std::string& filepath) {
    std::ofstream out(filepath);
    if (!out.is_open()) {
        PYENGINE_CORE_ERROR("Could not open file for writing: {}", filepath);
        return;
    }

    out << "Scene: " << "Untitled" << "\n";
    out << "Entities: \n";

    // Iterate over all entities with IDComponent (which should be all entities)
    auto view = m_Scene->GetRegistry().view<IDComponent>();
    for (auto entityID : view) {
        Entity entity = {entityID, m_Scene.get()};
        if (!entity)
            continue;

        SerializeEntity(out, entity);
    }

    out.close();
    PYENGINE_CORE_INFO("Scene serialized to: {}", filepath);
}

void SceneSerializer::SerializeRuntime(const std::string& filepath) {
    // Runtime serialization might be binary or compressed,
    // for now just use text
    Serialize(filepath);
}

void SceneSerializer::SerializeEntity(std::ofstream& out, Entity entity) {
    if (!entity.HasComponent<IDComponent>())
        return;

    out << "  Entity: " << (uint64_t)entity.GetUUID() << "\n";

    if (entity.HasComponent<TagComponent>()) {
        auto& tag = entity.GetComponent<TagComponent>();
        out << "    TagComponent:\n";
        out << "      Tag: " << tag.Tag << "\n";
        out << "      Layer: " << tag.Layer << "\n";
        out << "      IsStatic: " << tag.IsStatic << "\n";
        out << "      IsActive: " << tag.IsActive << "\n";
    }

    if (entity.HasComponent<TransformComponent>()) {
        auto& tc = entity.GetComponent<TransformComponent>();
        out << "    TransformComponent:\n";
        out << "      Translation: " << tc.Position << "\n";
        out << "      Rotation: " << tc.Rotation << "\n";
        out << "      Scale: " << tc.Scale << "\n";
    }

    if (entity.HasComponent<CameraComponent>()) {
        auto& cc = entity.GetComponent<CameraComponent>();
        out << "    CameraComponent:\n";
        out << "      ProjectionType: " << (int)cc.Projection << "\n";
        out << "      PerspectiveFOV: " << cc.FieldOfView << "\n";
        out << "      PerspectiveNear: " << cc.NearPlane << "\n";
        out << "      PerspectiveFar: " << cc.FarPlane << "\n";
        out << "      OrthographicSize: " << cc.OrthographicSize << "\n";
        out << "      Primary: " << cc.IsPrimary << "\n";
    }

    if (entity.HasComponent<MeshRendererComponent>()) {
        auto& mrc = entity.GetComponent<MeshRendererComponent>();
        out << "    MeshRendererComponent:\n";
        out << "      MeshID: " << mrc.MeshID << "\n";
        out << "      MaterialID: " << mrc.MaterialID << "\n";
        out << "      ColorTint: " << mrc.ColorTint << "\n";
        out << "      Metallic: " << mrc.Metallic << "\n";
        out << "      Roughness: " << mrc.Roughness << "\n";
        out << "      AO: " << mrc.AO << "\n";
        out << "      CastShadows: " << mrc.CastShadows << "\n";
        out << "      ReceiveShadows: " << mrc.ReceiveShadows << "\n";
    }

    if (entity.HasComponent<LightComponent>()) {
        auto& lc = entity.GetComponent<LightComponent>();
        out << "    LightComponent:\n";
        out << "      Type: " << (int)lc.LightType << "\n";
        out << "      Color: " << lc.Color << "\n";
        out << "      Intensity: " << lc.Intensity << "\n";
        out << "      Range: " << lc.Range << "\n";
        out << "      SpotAngle: " << lc.OuterConeAngle << "\n";
        out << "      CastShadows: " << lc.CastShadows << "\n";
        out << "      ShadowBias: " << lc.ShadowBias << "\n";
    }

    if (entity.HasComponent<RigidBodyComponent>()) {
        auto& rbc = entity.GetComponent<RigidBodyComponent>();
        out << "    RigidBodyComponent:\n";
        out << "      Type: " << (int)rbc.Type << "\n";
        out << "      Mass: " << rbc.Mass << "\n";
        out << "      LinearDamping: " << rbc.LinearDamping << "\n";
        out << "      AngularDamping: " << rbc.AngularDamping << "\n";
        out << "      UseGravity: " << rbc.UseGravity << "\n";
    }

    if (entity.HasComponent<BoxColliderComponent>()) {
        auto& bcc = entity.GetComponent<BoxColliderComponent>();
        out << "    BoxColliderComponent:\n";
        out << "      Offset: " << bcc.Center << "\n";
        out << "      Size: " << bcc.Size << "\n";
        out << "      IsTrigger: " << bcc.IsTrigger << "\n";
        out << "      Material: " << bcc.PhysicsMaterialID << "\n";
    }

    if (entity.HasComponent<PythonScriptComponent>()) {
        auto& psc = entity.GetComponent<PythonScriptComponent>();
        out << "    PythonScriptComponent:\n";
        out << "      ScriptPath: " << psc.ScriptPath << "\n";
        out << "      Enabled: " << psc.Enabled << "\n";
        out << "      AutoReload: " << psc.AutoReload << "\n";
    }

    out << "  EndEntity\n";
}

bool SceneSerializer::Deserialize(const std::string& filepath) {
    std::ifstream in(filepath);
    if (!in.is_open()) {
        PYENGINE_CORE_ERROR("Could not open file for reading: {}", filepath);
        return false;
    }

    std::string line;
    Entity currentEntity;
    bool inEntity = false;
    std::string currentComponent = "";

    while (std::getline(in, line)) {
        // Trim leading whitespace
        size_t first = line.find_first_not_of(" \t");
        if (first == std::string::npos)
            continue;
        std::string content = line.substr(first);
        int indent = first;

        if (content.rfind("Entity:", 0) == 0) {
            uint64_t uuid = std::stoull(content.substr(8));
            currentEntity = m_Scene->CreateEntityWithUUID(UUID(uuid), "Empty Entity");
            inEntity = true;
        } else if (content == "EndEntity") {
            inEntity = false;
            currentEntity = {};
        } else if (inEntity) {
            if (content.back() == ':') {
                currentComponent = content.substr(0, content.length() - 1);

                // Components are usually added by CreateEntity defaults or here
                // We rely on CreateEntity for some, but ensure they exist
                if (currentComponent == "TagComponent" && !currentEntity.HasComponent<TagComponent>())
                    currentEntity.AddComponent<TagComponent>();
                if (currentComponent == "TransformComponent" && !currentEntity.HasComponent<TransformComponent>())
                    currentEntity.AddComponent<TransformComponent>();
                if (currentComponent == "CameraComponent" && !currentEntity.HasComponent<CameraComponent>())
                    currentEntity.AddComponent<CameraComponent>();
                if (currentComponent == "MeshRendererComponent" && !currentEntity.HasComponent<MeshRendererComponent>())
                    currentEntity.AddComponent<MeshRendererComponent>();
                if (currentComponent == "LightComponent" && !currentEntity.HasComponent<LightComponent>())
                    currentEntity.AddComponent<LightComponent>();
                if (currentComponent == "PythonScriptComponent" && !currentEntity.HasComponent<PythonScriptComponent>())
                    currentEntity.AddComponent<PythonScriptComponent>();
                // ...
            } else {
                size_t colonPos = content.find(':');
                if (colonPos != std::string::npos) {
                    std::string key = content.substr(0, colonPos);
                    std::string value = content.substr(colonPos + 2);

                    if (currentComponent == "TagComponent") {
                        auto& tag = currentEntity.GetComponent<TagComponent>();
                        if (key == "Tag")
                            tag.Tag = value;
                        else if (key == "Layer")
                            tag.Layer = std::stoi(value);
                        else if (key == "IsStatic")
                            tag.IsStatic = (value == "1");
                        else if (key == "IsActive")
                            tag.IsActive = (value == "1");
                    } else if (currentComponent == "TransformComponent") {
                        auto& tc = currentEntity.GetComponent<TransformComponent>();
                        std::stringstream ss(value);
                        if (key == "Translation")
                            ss >> tc.Position.x >> tc.Position.y >> tc.Position.z;
                        else if (key == "Rotation")
                            ss >> tc.Rotation.x >> tc.Rotation.y >> tc.Rotation.z;
                        else if (key == "Scale")
                            ss >> tc.Scale.x >> tc.Scale.y >> tc.Scale.z;
                    } else if (currentComponent == "CameraComponent") {
                        auto& cc = currentEntity.GetComponent<CameraComponent>();
                        if (key == "Primary")
                            cc.IsPrimary = (value == "1");
                        else if (key == "PerspectiveFOV")
                            cc.FieldOfView = std::stof(value);
                        else if (key == "ProjectionType")
                            cc.Projection = (CameraComponent::ProjectionType)std::stoi(value);
                    } else if (currentComponent == "MeshRendererComponent") {
                        auto& mrc = currentEntity.GetComponent<MeshRendererComponent>();
                        if (key == "MeshID")
                            mrc.MeshID = std::stoul(value);
                        else if (key == "MaterialID")
                            mrc.MaterialID = std::stoul(value);
                        else if (key == "ColorTint") {
                            std::stringstream ss(value);
                            ss >> mrc.ColorTint.r >> mrc.ColorTint.g >> mrc.ColorTint.b >> mrc.ColorTint.a;
                        }
                        else if (key == "Metallic")
                            mrc.Metallic = std::stof(value);
                        else if (key == "Roughness")
                            mrc.Roughness = std::stof(value);
                        else if (key == "AO")
                            mrc.AO = std::stof(value);
                    } else if (currentComponent == "PythonScriptComponent") {
                        auto& psc = currentEntity.GetComponent<PythonScriptComponent>();
                        if (key == "ScriptPath")
                            psc.ScriptPath = value;
                        else if (key == "Enabled")
                            psc.Enabled = (value == "1");
                        else if (key == "AutoReload")
                            psc.AutoReload = (value == "1");
                    }
                }
            }
        }
    }

    in.close();
    return true;
}

bool SceneSerializer::DeserializeRuntime(const std::string& filepath) {
    return Deserialize(filepath);
}

}  // namespace PyEngine
