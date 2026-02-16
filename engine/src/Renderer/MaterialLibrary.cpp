#include "PyEngine/Renderer/MaterialLibrary.hpp"

#include "PyEngine/Core/Log.hpp"

namespace PyEngine {

MaterialLibrary::MaterialLibrary() {
    // Create default material
    m_DefaultMaterial = std::make_shared<MaterialInstance>();
    m_DefaultMaterial->Name = "Default-Material";
    m_DefaultMaterial->ID = 0;
    m_DefaultMaterial->AlbedoColor = {1.0f, 1.0f, 1.0f, 1.0f};
    m_DefaultMaterial->Metallic = 0.0f;
    m_DefaultMaterial->Roughness = 0.5f;

    // Create error material (magenta)
    m_ErrorMaterial = std::make_shared<MaterialInstance>();
    m_ErrorMaterial->Name = "Error-Material";
    m_ErrorMaterial->ID = UINT32_MAX;
    m_ErrorMaterial->AlbedoColor = {1.0f, 0.0f, 1.0f, 1.0f};
    m_ErrorMaterial->Metallic = 0.0f;
    m_ErrorMaterial->Roughness = 1.0f;

    // Add default to map
    m_Materials[m_DefaultMaterial->Name] = m_DefaultMaterial;
    m_MaterialsByID[0] = m_DefaultMaterial;
}

std::shared_ptr<MaterialInstance> MaterialLibrary::CreateMaterial(const std::string& name) {
    if (m_Materials.find(name) != m_Materials.end()) {
        PYENGINE_CORE_WARN("Material '{}' already exists, returning existing instance.", name);
        return m_Materials[name];
    }

    auto material = std::make_shared<MaterialInstance>();
    material->Name = name;
    material->ID = m_NextID++;

    // Copy defaults
    material->AlbedoColor = m_DefaultMaterial->AlbedoColor;
    material->Metallic = m_DefaultMaterial->Metallic;
    material->Roughness = m_DefaultMaterial->Roughness;

    m_Materials[name] = material;
    m_MaterialsByID[material->ID] = material;

    PYENGINE_CORE_INFO("Created material '{}' (ID: {})", name, material->ID);
    return material;
}

std::shared_ptr<MaterialInstance> MaterialLibrary::GetMaterial(const std::string& name) {
    if (m_Materials.find(name) != m_Materials.end()) {
        return m_Materials[name];
    }
    return m_ErrorMaterial;
}

std::shared_ptr<MaterialInstance> MaterialLibrary::GetMaterial(uint32_t id) {
    if (m_MaterialsByID.find(id) != m_MaterialsByID.end()) {
        return m_MaterialsByID[id];
    }
    return m_ErrorMaterial;
}

void MaterialLibrary::AddMaterial(std::shared_ptr<MaterialInstance> material) {
    if (!material)
        return;

    if (material->ID == 0)
        material->ID = m_NextID++;

    m_Materials[material->Name] = material;
    m_MaterialsByID[material->ID] = material;
}

void MaterialLibrary::RemoveMaterial(const std::string& name) {
    if (m_Materials.find(name) != m_Materials.end()) {
        uint32_t id = m_Materials[name]->ID;
        m_Materials.erase(name);
        m_MaterialsByID.erase(id);
    }
}

std::shared_ptr<MaterialInstance> MaterialLibrary::GetDefaultMaterial() {
    return m_DefaultMaterial;
}

std::shared_ptr<MaterialInstance> MaterialLibrary::GetErrorMaterial() {
    return m_ErrorMaterial;
}

}  // namespace PyEngine
