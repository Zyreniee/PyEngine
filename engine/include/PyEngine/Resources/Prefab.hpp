#pragma once

#include <filesystem>
#include <string>

#include "PyEngine/Scene/Entity.hpp"

namespace PyEngine {

#include <memory>
class Scene;

class Prefab : public std::enable_shared_from_this<Prefab> {
public:
    static std::shared_ptr<Prefab> Create(Entity entity);
    static std::shared_ptr<Prefab> Load(const std::filesystem::path& filepath);

    Entity Instantiate(Scene* scene);

    bool Save(const std::filesystem::path& filepath);

private:
    std::string m_Name;
    // In a real engine, we'd store a serialized representation of the entity heirarchy here
    // For this implementation, we'll store a raw string of the serialized data
    std::string m_SerializedData;

    friend class PrefabSerializer;
};

class PrefabSerializer {
public:
    static bool Serialize(std::shared_ptr<Prefab> prefab, const std::filesystem::path& filepath);
    static bool Deserialize(std::shared_ptr<Prefab> prefab, const std::filesystem::path& filepath);
};

}  // namespace PyEngine
