#include "PyEngine/Scene/Scene.hpp"
#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Scene/Components.hpp"
#include "PyEngine/Scene/Entity.hpp"

namespace PyEngine {

Scene::Scene() { PYENGINE_CORE_INFO("Scene created"); }

Scene::~Scene() { PYENGINE_CORE_INFO("Scene destroyed"); }

Entity Scene::CreateEntity(const std::string &name) {
  Entity entity = {m_Registry.create(), &m_Registry};
  entity.AddComponent<TransformComponent>();
  return entity;
}

void Scene::DestroyEntity(Entity entity) { m_Registry.destroy(entity); }

void Scene::OnUpdate(float deltaTime) {
  // Update systems here
}

} // namespace PyEngine
