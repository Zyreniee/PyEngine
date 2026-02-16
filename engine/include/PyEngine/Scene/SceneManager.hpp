#pragma once

#include <memory>
#include <string>

namespace PyEngine {

class Scene;

// Like Unity's SceneManager — manages active scene, loading/saving/switching
class SceneManager {
public:
    static SceneManager& Get();

    std::shared_ptr<Scene> NewScene(const std::string& name = "Untitled");
    std::shared_ptr<Scene> GetActiveScene() { return m_ActiveScene; }
    void SetActiveScene(const std::shared_ptr<Scene>& scene) { m_ActiveScene = scene; }

    bool SaveScene(const std::string& filepath);
    bool LoadScene(const std::string& filepath);

    const std::string& GetCurrentScenePath() const { return m_CurrentScenePath; }

private:
    SceneManager() = default;

    std::shared_ptr<Scene> m_ActiveScene;
    std::string m_CurrentScenePath;
};

}  // namespace PyEngine
