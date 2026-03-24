#include "PyEngine/Scene/Scene.hpp"

#include "PyEngine/Assets/Mesh.hpp"
#include "PyEngine/Core/Application.hpp"
#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Core/UUID.hpp"
#include "PyEngine/Renderer/Renderer.hpp"
#include "PyEngine/Scene/Components.hpp"
#include "PyEngine/Scene/EditorCamera.hpp"
#include "PyEngine/Scene/Entity.hpp"

namespace PyEngine {

Scene::Scene(const std::string& name) : m_Name(name) {
    PYENGINE_CORE_INFO("Scene '{}' created", name);
    m_TerrainSystem.Init();
    m_NavMeshSystem.Init();
}

Scene::~Scene() {
    PYENGINE_CORE_INFO("Scene '{}' destroyed", m_Name);
}

Entity Scene::CreateEntity(const std::string& name) {
    return CreateEntityWithUUID(UUID(), name);
}

Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name) {
    Entity entity = {m_Registry.create(), this};
    entity.AddComponent<IDComponent>(uuid);
    entity.AddComponent<TransformComponent>();
    auto& tag = entity.AddComponent<TagComponent>();
    tag.Tag = name.empty() ? "Entity" : name;
    return entity;
}

void Scene::DestroyEntity(Entity entity) {
    m_Registry.destroy(entity);
}

void Scene::OnUpdate(float deltaTime) {
    // Generic update if needed
}

void Scene::OnUpdateRuntime(float deltaTime) {
    // Update scripts
    {
        auto view = m_Registry.view<ScriptComponent>();
        for (auto entity : view) {
            // Script update logic would go here
        }
    }

    // Physics update
    // ...

    // Find Primary Camera
    CameraComponent* mainCamera = nullptr;
    glm::mat4 cameraTransform;

    auto view = m_Registry.view<TransformComponent, CameraComponent>();
    for (auto entity : view) {
        auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
        if (camera.Primary) {
            mainCamera = &camera;
            cameraTransform = transform.GetTransformMatrix();
            break;
        }
    }

    if (mainCamera) {
        auto& app = Application::Get();
        auto& renderer = app.GetRenderer();

        float aspectRatio = (float)m_ViewportWidth / (float)m_ViewportHeight;
        if (std::isnan(aspectRatio) || std::isinf(aspectRatio))
            aspectRatio = 16.0f / 9.0f;

        renderer.SetCamera(glm::inverse(cameraTransform), mainCamera->GetProjectionMatrix(aspectRatio));
    }

    m_TerrainSystem.Update(deltaTime, this);
    Render();
}

void Scene::OnUpdateEditor(float deltaTime, EditorCamera& camera) {
    m_TerrainSystem.Update(deltaTime, this);
    Render();
}

void Scene::OnViewportResize(uint32_t width, uint32_t height) {
    m_ViewportWidth = width;
    m_ViewportHeight = height;
}

void Scene::Render() {
    auto& app = Application::Get();
    auto& renderer = app.GetRenderer();

    // Collect lights from scene entities for PBR lighting
    renderer.CollectLights(this);

    // Lazy initialize default meshes
    static std::shared_ptr<Mesh> cubeMesh;
    static std::shared_ptr<Mesh> sphereMesh;
    static std::shared_ptr<Mesh> planeMesh;
    static std::shared_ptr<Mesh> cylinderMesh;
    static std::shared_ptr<Mesh> capsuleMesh;

    if (!cubeMesh) {
        cubeMesh.reset(Mesh::CreateCube(renderer.GetContext(), renderer.GetAllocator()));
        sphereMesh.reset(Mesh::CreateSphere(renderer.GetContext(), renderer.GetAllocator()));
        planeMesh.reset(Mesh::CreatePlane(renderer.GetContext(), renderer.GetAllocator()));
        cylinderMesh.reset(Mesh::CreateCylinder(renderer.GetContext(), renderer.GetAllocator()));
        capsuleMesh.reset(Mesh::CreateCapsule(renderer.GetContext(), renderer.GetAllocator()));
    }

    auto group = m_Registry.group<TransformComponent>(entt::get<MeshRendererComponent>);
    for (auto entity : group) {
        auto [transform, meshComp] = group.get<TransformComponent, MeshRendererComponent>(entity);

        if (!meshComp.Visible)
            continue;

        Mesh* meshToDraw = nullptr;

        if (meshComp.CustomMesh) {
            meshToDraw = meshComp.CustomMesh.get();
        } else {
            switch (meshComp.MeshID) {
                case 0:
                    meshToDraw = cubeMesh.get();
                    break;
                case 1:
                    meshToDraw = sphereMesh.get();
                    break;
                case 2:
                    meshToDraw = planeMesh.get();
                    break;
                case 3:
                    meshToDraw = cylinderMesh.get();
                    break;
                case 4:
                    meshToDraw = capsuleMesh.get();
                    break;
                default:
                    meshToDraw = cubeMesh.get();
                    break;
            }
        }

        if (meshToDraw) {
            // Pass per-entity color tint and material properties to PBR renderer
            renderer.DrawMesh(meshToDraw, transform.GetTransformMatrix(),
                              meshComp.ColorTint, meshComp.Metallic,
                              meshComp.Roughness, meshComp.AO);
        }
    }
}

Entity Scene::FindEntityByName(const std::string& name) {
    auto view = m_Registry.view<TagComponent>();
    for (auto entity : view) {
        const auto& tag = view.get<TagComponent>(entity);
        if (tag.Tag == name) {
            return Entity(entity, this);
        }
    }
    return Entity();
}

Entity Scene::FindEntityByUUID(UUID uuid) {
    auto view = m_Registry.view<IDComponent>();
    for (auto entity : view) {
        const auto& id = view.get<IDComponent>(entity);
        if (id.ID == uuid) {
            return Entity(entity, this);
        }
    }
    return Entity();
}

Entity Scene::GetPrimaryCameraEntity() {
    auto view = m_Registry.view<CameraComponent>();
    for (auto entity : view) {
        const auto& camera = view.get<CameraComponent>(entity);
        if (camera.Primary) {
            return Entity(entity, this);
        }
    }
    return Entity();
}

std::vector<Entity> Scene::GetAllEntities() {
    std::vector<Entity> entities;
    auto view = m_Registry.view<IDComponent>();
    for (auto entity : view) {
        entities.emplace_back(entity, this);
    }
    return entities;
}

size_t Scene::GetEntityCount() const {
    return m_Registry.view<IDComponent>().size();
}

void Scene::OnRuntimeStart() {
    PYENGINE_CORE_INFO("Scene '{}' runtime started", m_Name);
}

void Scene::OnRuntimeStop() {
    PYENGINE_CORE_INFO("Scene '{}' runtime stopped", m_Name);
}

}  // namespace PyEngine
