#include "PyEngine/Scene/SceneManager.hpp"

#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Scene/Scene.hpp"
#include "PyEngine/Scene/SceneSerializer.hpp"

namespace PyEngine {

SceneManager& SceneManager::Get() {
    static SceneManager instance;
    return instance;
}

std::shared_ptr<Scene> SceneManager::NewScene(const std::string& name) {
    m_ActiveScene = std::make_shared<Scene>(name);
    m_CurrentScenePath.clear();
    PYENGINE_CORE_INFO("New scene created: {}", name);
    return m_ActiveScene;
}

bool SceneManager::SaveScene(const std::string& filepath) {
    if (!m_ActiveScene) {
        PYENGINE_CORE_ERROR("No active scene to save");
        return false;
    }

    SceneSerializer serializer(m_ActiveScene);
    serializer.Serialize(filepath);
    m_CurrentScenePath = filepath;
    return true;
}

bool SceneManager::LoadScene(const std::string& filepath) {
    auto newScene = std::make_shared<Scene>();
    SceneSerializer serializer(newScene);

    if (serializer.Deserialize(filepath)) {
        m_ActiveScene = newScene;
        m_CurrentScenePath = filepath;
        return true;
    }

    return false;
}

}  // namespace PyEngine
