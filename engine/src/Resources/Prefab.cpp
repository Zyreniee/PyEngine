#include "PyEngine/Resources/Prefab.hpp"

#include <fstream>
#include <sstream>

#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Scene/SceneSerializer.hpp"

namespace PyEngine {

std::shared_ptr<Prefab> Prefab::Create(Entity entity) {
    auto prefab = std::make_shared<Prefab>();
    if (entity.HasComponent<TagComponent>())
        prefab->m_Name = entity.GetComponent<TagComponent>().Tag;
    else
        prefab->m_Name = "Prefab";

    // Serialize entity to string
    // We reuse SceneSerializer logic but for a single entity tree
    // Ideally SceneSerializer would have a public method for this,
    // but here we mock it by "serializing" to a string buffer
    std::stringstream ss;
    // Mock serialization of entity components
    ss << "PrefabData: " << prefab->m_Name << "\n";
    // Real implementation would call SceneSerializer::SerializeEntity(ss, entity);
    prefab->m_SerializedData = ss.str();

    return prefab;
}

std::shared_ptr<Prefab> Prefab::Load(const std::filesystem::path& filepath) {
    auto prefab = std::make_shared<Prefab>();
    if (PrefabSerializer::Deserialize(prefab, filepath)) {
        return prefab;
    }
    return nullptr;
}

bool Prefab::Save(const std::filesystem::path& filepath) {
    return PrefabSerializer::Serialize(shared_from_this(), filepath);
}

Entity Prefab::Instantiate(Scene* scene) {
    if (!scene)
        return {};

    // Create new entity
    Entity entity = scene->CreateEntity(m_Name);

    // Deserialize components from m_SerializedData onto 'entity'
    // Parsing logic would go here

    return entity;
}

// ═══════════════════════════════════════════════════════════════
// PrefabSerializer
// ═══════════════════════════════════════════════════════════════

bool PrefabSerializer::Serialize(std::shared_ptr<Prefab> prefab, const std::filesystem::path& filepath) {
    std::ofstream out(filepath);
    if (!out.is_open())
        return false;

    out << prefab->m_SerializedData;
    out.close();
    return true;
}

bool PrefabSerializer::Deserialize(std::shared_ptr<Prefab> prefab, const std::filesystem::path& filepath) {
    std::ifstream in(filepath);
    if (!in.is_open())
        return false;

    std::stringstream buffer;
    buffer << in.rdbuf();
    prefab->m_SerializedData = buffer.str();

    // Extract name
    std::string line;
    std::stringstream ss(prefab->m_SerializedData);
    if (std::getline(ss, line)) {
        if (line.find("PrefabData: ") == 0) {
            prefab->m_Name = line.substr(12);
        }
    }

    return true;
}

}  // namespace PyEngine
