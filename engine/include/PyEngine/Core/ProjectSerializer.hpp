#pragma once

#include <filesystem>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace PyEngine {

struct ProjectConfig {
    std::string Name = "Untitled Project";
    std::filesystem::path ProjectDirectory;
    std::filesystem::path AssetDirectory;
    std::filesystem::path StartupScene;

    // Render Settings
    bool EnableShadows = true;
    bool EnableBloom = true;
    bool EnableToneMapping = true;

    // Physics Settings
    glm::vec3 Gravity = {0.0f, -9.81f, 0.0f};
    int VelocityIterations = 6;
    int PositionIterations = 2;
};

class Project {
public:
    static const std::shared_ptr<ProjectConfig>& GetConfig() { return s_ActiveProject->m_Config; }

    static std::shared_ptr<Project> New();
    static std::shared_ptr<Project> Load(const std::filesystem::path& path);
    static bool SaveActive(const std::filesystem::path& path);

    static std::shared_ptr<Project> GetActive() { return s_ActiveProject; }

private:
    std::shared_ptr<ProjectConfig> m_Config;
    static std::shared_ptr<Project> s_ActiveProject;

    friend class ProjectSerializer;
};

class ProjectSerializer {
public:
    ProjectSerializer(std::shared_ptr<Project> project);

    bool Serialize(const std::filesystem::path& filepath);
    bool Deserialize(const std::filesystem::path& filepath);

private:
    std::shared_ptr<Project> m_Project;
};

}  // namespace PyEngine
