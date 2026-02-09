#pragma once

#include <entt.hpp>
#include <memory>
#include <string>

namespace PyEngine {

class Entity;

class Scene {
public:
    Scene();
    ~Scene();

    Entity CreateEntity(const std::string& name = "Entity");
    void DestroyEntity(Entity entity);

    void OnUpdate(float deltaTime);

    entt::registry& GetRegistry() { return m_Registry; }

private:
    entt::registry m_Registry;
};

}  // namespace PyEngine
