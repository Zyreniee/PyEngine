#pragma once

#include <fstream>
#include <sstream>
#include <string>

#include "PyEngine/Scene/Scene.hpp"

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// Scene Serializer — Save/Load Scenes
// ═══════════════════════════════════════════════════════════════

class SceneSerializer {
public:
    SceneSerializer(const std::shared_ptr<Scene>& scene);

    void Serialize(const std::string& filepath);
    void SerializeRuntime(const std::string& filepath);

    bool Deserialize(const std::string& filepath);
    bool DeserializeRuntime(const std::string& filepath);

private:
    std::shared_ptr<Scene> m_Scene;

    // Helper to write/read values safely
    template <typename T>
    void WriteValue(std::ofstream& out, const std::string& key, const T& value);

    // Component Serializers
    void SerializeEntity(std::ofstream& out, Entity entity);
};

}  // namespace PyEngine
