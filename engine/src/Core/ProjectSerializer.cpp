#include "PyEngine/Core/ProjectSerializer.hpp"

#include <fstream>
#include <sstream>

#include "PyEngine/Core/Log.hpp"

namespace PyEngine {

std::shared_ptr<Project> Project::s_ActiveProject = nullptr;

std::shared_ptr<Project> Project::New() {
    s_ActiveProject = std::make_shared<Project>();
    s_ActiveProject->m_Config = std::make_shared<ProjectConfig>();
    return s_ActiveProject;
}

std::shared_ptr<Project> Project::Load(const std::filesystem::path& path) {
    std::shared_ptr<Project> project = std::make_shared<Project>();
    project->m_Config = std::make_shared<ProjectConfig>();

    ProjectSerializer serializer(project);
    if (serializer.Deserialize(path)) {
        s_ActiveProject = project;
        return s_ActiveProject;
    }

    return nullptr;
}

bool Project::SaveActive(const std::filesystem::path& path) {
    ProjectSerializer serializer(s_ActiveProject);
    return serializer.Serialize(path);
}

// ═══════════════════════════════════════════════════════════════
// Project Serializer Implementation
// ═══════════════════════════════════════════════════════════════

ProjectSerializer::ProjectSerializer(std::shared_ptr<Project> project) : m_Project(project) {}

bool ProjectSerializer::Serialize(const std::filesystem::path& filepath) {
    const auto& config = m_Project->m_Config;

    std::ofstream out(filepath);
    if (!out.is_open()) {
        PYENGINE_CORE_ERROR("Could not open project file: {}", filepath.string());
        return false;
    }

    out << "Project: " << config->Name << "\n";
    out << "AssetDirectory: " << config->AssetDirectory.string() << "\n";
    out << "StartupScene: " << config->StartupScene.string() << "\n";

    out << "Physics:\n";
    out << "  Gravity: " << config->Gravity.x << " " << config->Gravity.y << " " << config->Gravity.z << "\n";
    out << "  VelocityIterations: " << config->VelocityIterations << "\n";
    out << "  PositionIterations: " << config->PositionIterations << "\n";

    out << "Rendering:\n";
    out << "  EnableShadows: " << config->EnableShadows << "\n";
    out << "  EnableBloom: " << config->EnableBloom << "\n";

    out.close();
    return true;
}

bool ProjectSerializer::Deserialize(const std::filesystem::path& filepath) {
    auto& config = m_Project->m_Config;

    std::ifstream in(filepath);
    if (!in.is_open()) {
        PYENGINE_CORE_ERROR("Could not open project file: {}", filepath.string());
        return false;
    }

    std::string line;
    config->ProjectDirectory = filepath.parent_path();

    while (std::getline(in, line)) {
        std::stringstream ss(line);
        std::string key;
        ss >> key;

        if (key == "Project:") {
            std::string name;
            std::getline(ss, name);
            // trim leading space
            size_t first = name.find_first_not_of(' ');
            if (first != std::string::npos)
                config->Name = name.substr(first);
        } else if (key == "AssetDirectory:") {
            std::string path;
            ss >> path;
            config->AssetDirectory = path;
        } else if (key == "StartupScene:") {
            std::string path;
            ss >> path;
            config->StartupScene = path;
        } else if (key == "Gravity:") {
            ss >> config->Gravity.x >> config->Gravity.y >> config->Gravity.z;
        } else if (key == "VelocityIterations:") {
            ss >> config->VelocityIterations;
        } else if (key == "EnableShadows:") {
            ss >> config->EnableShadows;
        }
        // ... add more parsing logic
    }

    in.close();
    return true;
}

}  // namespace PyEngine
