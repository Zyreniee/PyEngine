#include "PyEngine/Scene/Entity.hpp"

#include "PyEngine/Scene/Scene.hpp"

namespace PyEngine {

Entity::Entity(entt::entity handle, Scene* scene) : m_EntityHandle(handle), m_Scene(scene) {}

const std::string& Entity::GetName() const {
    return GetComponent<TagComponent>().Tag;
}

void Entity::SetName(const std::string& name) {
    GetComponent<TagComponent>().Tag = name;
}

UUID Entity::GetUUID() const {
    return GetComponent<IDComponent>().ID;
}

TransformComponent& Entity::GetTransform() {
    return GetComponent<TransformComponent>();
}

const TransformComponent& Entity::GetTransform() const {
    return GetComponent<TransformComponent>();
}

}  // namespace PyEngine
