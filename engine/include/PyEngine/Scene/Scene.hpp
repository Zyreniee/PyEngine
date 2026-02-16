#pragma once

#include <entt/entt.hpp>
#include <memory>
#include <string>
#include <vector>

#include "PyEngine/AI/NavMeshSystem.hpp"
#include "PyEngine/Scene/TerrainSystem.hpp"

namespace PyEngine {

class Entity;
class UUID;

class Scene {
public:
    Scene(const std::string& name = "Untitled");
    ~Scene();

    Entity CreateEntity(const std::string& name = "Entity");
    Entity CreateEntityWithUUID(UUID uuid, const std::string& name = "Entity");
    void DestroyEntity(Entity entity);

    void OnUpdate(float deltaTime);
    void OnUpdateRuntime(float deltaTime);
    void OnUpdateEditor(float deltaTime, class EditorCamera& camera);
    void OnViewportResize(uint32_t width, uint32_t height);
    void Render();

    // Query
    Entity FindEntityByName(const std::string& name);
    Entity FindEntityByUUID(UUID uuid);
    Entity GetPrimaryCameraEntity();
    std::vector<Entity> GetAllEntities();
    size_t GetEntityCount() const;

    // Scene properties
    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }

    entt::registry& GetRegistry() { return m_Registry; }
    const entt::registry& GetRegistry() const { return m_Registry; }

    NavMeshSystem& GetNavMeshSystem() { return m_NavMeshSystem; }

    // Scene lifecycle
    void OnRuntimeStart();
    void OnRuntimeStop();

private:
    std::string m_Name;
    entt::registry m_Registry;

    TerrainSystem m_TerrainSystem;
    NavMeshSystem m_NavMeshSystem;

    uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

    friend class Entity;
    friend class SceneSerializer;
};

}  // namespace PyEngine
