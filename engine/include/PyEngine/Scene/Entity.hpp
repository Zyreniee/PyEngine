#pragma once

#include <entt/entt.hpp>
#include <utility>

#include "PyEngine/Core/UUID.hpp"
#include "PyEngine/Scene/Components.hpp"
#include "PyEngine/Scene/Scene.hpp"

namespace PyEngine {

class Scene;

class Entity {
public:
    Entity() = default;
    Entity(entt::entity handle, Scene* scene);

    template <typename T, typename... Args>
    T& AddComponent(Args&&... args) {
        return m_Scene->GetRegistry().emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
    }

    template <typename T>
    T& GetComponent() {
        return m_Scene->GetRegistry().get<T>(m_EntityHandle);
    }

    template <typename T>
    const T& GetComponent() const {
        return m_Scene->GetRegistry().get<T>(m_EntityHandle);
    }

    template <typename T>
    bool HasComponent() const {
        return m_Scene->GetRegistry().all_of<T>(m_EntityHandle);
    }

    template <typename T>
    void RemoveComponent() {
        m_Scene->GetRegistry().remove<T>(m_EntityHandle);
    }

    // Convenience accessors (Unity-like)
    const std::string& GetName() const;
    void SetName(const std::string& name);
    UUID GetUUID() const;
    TransformComponent& GetTransform();
    const TransformComponent& GetTransform() const;

    Scene* GetScene() { return m_Scene; }

    operator bool() const { return m_EntityHandle != entt::null && m_Scene != nullptr; }
    operator entt::entity() const { return m_EntityHandle; }
    operator uint32_t() const { return static_cast<uint32_t>(m_EntityHandle); }

    bool operator==(const Entity& other) const {
        return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
    }

    bool operator!=(const Entity& other) const { return !(*this == other); }

private:
    entt::entity m_EntityHandle = entt::null;
    Scene* m_Scene = nullptr;
};

}  // namespace PyEngine
