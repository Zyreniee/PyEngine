#pragma once

#include <entt.hpp>
#include <utility>

#include "PyEngine/Scene/Components.hpp"

namespace PyEngine {

class Entity {
public:
    Entity() = default;
    Entity(entt::entity handle, entt::registry* registry) : m_EntityHandle(handle), m_Registry(registry) {}

    template <typename T, typename... Args>
    T& AddComponent(Args&&... args) {
        return m_Registry->emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
    }

    template <typename T>
    T& GetComponent() {
        return m_Registry->get<T>(m_EntityHandle);
    }

    template <typename T>
    bool HasComponent() {
        return m_Registry->all_of<T>(m_EntityHandle);
    }

    template <typename T>
    void RemoveComponent() {
        m_Registry->remove<T>(m_EntityHandle);
    }

    operator bool() const { return m_EntityHandle != entt::null; }
    operator entt::entity() const { return m_EntityHandle; }
    operator uint32_t() const { return static_cast<uint32_t>(m_EntityHandle); }

    bool operator==(const Entity& other) const {
        return m_EntityHandle == other.m_EntityHandle && m_Registry == other.m_Registry;
    }

    bool operator!=(const Entity& other) const { return !(*this == other); }

private:
    entt::entity m_EntityHandle = entt::null;
    entt::registry* m_Registry = nullptr;
};

}  // namespace PyEngine
