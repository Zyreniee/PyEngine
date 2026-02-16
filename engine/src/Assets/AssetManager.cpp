#include "PyEngine/Assets/AssetManager.hpp"

#include <vk_mem_alloc.h>

#include "PyEngine/Assets/Mesh.hpp"
#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Renderer/Material.hpp"
#include "PyEngine/Renderer/Texture.hpp"
#include "PyEngine/Renderer/VulkanContext.hpp"

namespace PyEngine {

AssetManager& AssetManager::Get() {
    static AssetManager instance;
    return instance;
}

void AssetManager::Init(VulkanContext* context, void* allocator) {
    m_Context = context;
    m_Allocator = allocator;

    // Create default material
    CreateMaterial("Default Material");

    PYENGINE_CORE_INFO("AssetManager initialized");
}

void AssetManager::Shutdown() {
    m_PrimitiveMeshes.clear();
    m_LoadedMeshes.clear();
    m_Materials.clear();
    m_MaterialNames.clear();
    m_Textures.clear();
    m_WhiteTexture.reset();

    PYENGINE_CORE_INFO("AssetManager shut down");
}

Mesh* AssetManager::GetPrimitiveMesh(PrimitiveType type) {
    auto it = m_PrimitiveMeshes.find(type);
    if (it != m_PrimitiveMeshes.end()) {
        return it->second.get();
    }

    VmaAllocator allocator = static_cast<VmaAllocator>(m_Allocator);
    Mesh* mesh = nullptr;

    switch (type) {
        case PrimitiveType::Cube:
            mesh = Mesh::CreateCube(*m_Context, allocator);
            break;
        case PrimitiveType::Sphere:
            mesh = Mesh::CreateSphere(*m_Context, allocator);
            break;
        case PrimitiveType::Plane:
            mesh = Mesh::CreatePlane(*m_Context, allocator);
            break;
        case PrimitiveType::Cylinder:
            mesh = Mesh::CreateCylinder(*m_Context, allocator);
            break;
        case PrimitiveType::Capsule:
            mesh = Mesh::CreateCapsule(*m_Context, allocator);
            break;
        case PrimitiveType::Quad:
            mesh = Mesh::CreatePlane(*m_Context, allocator);
            break;
    }

    if (mesh) {
        m_PrimitiveMeshes[type] = std::unique_ptr<Mesh>(mesh);
    }

    return mesh;
}

Mesh* AssetManager::LoadMesh(const std::string& filepath) {
    auto it = m_LoadedMeshes.find(filepath);
    if (it != m_LoadedMeshes.end()) {
        return it->second.get();
    }

    // TODO: Implement mesh file loading (glTF, OBJ, etc.)
    PYENGINE_CORE_WARN("Mesh loading not yet implemented: {}", filepath);
    return nullptr;
}

uint32_t AssetManager::CreateMaterial(const std::string& name) {
    auto material = std::make_unique<Material>(name);
    uint32_t id = static_cast<uint32_t>(m_Materials.size());
    m_Materials.push_back(std::move(material));
    m_MaterialNames.push_back(name);
    return id;
}

Material* AssetManager::GetMaterial(uint32_t id) {
    if (id < m_Materials.size()) {
        return m_Materials[id].get();
    }
    return nullptr;
}

Material* AssetManager::GetDefaultMaterial() {
    return GetMaterial(0);
}

std::shared_ptr<Texture2D> AssetManager::LoadTexture(const std::string& filepath) {
    auto it = m_Textures.find(filepath);
    if (it != m_Textures.end()) {
        return it->second;
    }

    VmaAllocator allocator = static_cast<VmaAllocator>(m_Allocator);
    auto texture = std::make_shared<Texture2D>(*m_Context, allocator, filepath);
    m_Textures[filepath] = texture;
    return texture;
}

std::shared_ptr<Texture2D> AssetManager::GetWhiteTexture() {
    if (!m_WhiteTexture) {
        VmaAllocator allocator = static_cast<VmaAllocator>(m_Allocator);
        m_WhiteTexture = Texture2D::CreateWhiteTexture(*m_Context, allocator);
    }
    return m_WhiteTexture;
}

}  // namespace PyEngine
