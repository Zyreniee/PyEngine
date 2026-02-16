#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace PyEngine {

class Mesh;
class Texture2D;
class Material;
class VulkanContext;

// Like Unity's AssetDatabase — central registry for all engine assets
class AssetManager {
public:
    static AssetManager& Get();

    void Init(VulkanContext* context, void* allocator);
    void Shutdown();

    // Mesh primitives (like Unity's built-in primitives)
    enum class PrimitiveType { Cube, Sphere, Plane, Cylinder, Capsule, Quad };

    Mesh* GetPrimitiveMesh(PrimitiveType type);
    Mesh* LoadMesh(const std::string& filepath);

    // Materials
    uint32_t CreateMaterial(const std::string& name = "New Material");
    Material* GetMaterial(uint32_t id);
    Material* GetDefaultMaterial();

    // Textures
    std::shared_ptr<Texture2D> LoadTexture(const std::string& filepath);
    std::shared_ptr<Texture2D> GetWhiteTexture();

    // Asset listing
    const std::vector<std::string>& GetMaterialNames() const { return m_MaterialNames; }

private:
    AssetManager() = default;

    VulkanContext* m_Context = nullptr;
    void* m_Allocator = nullptr;  // VmaAllocator

    // Meshes
    std::unordered_map<PrimitiveType, std::unique_ptr<Mesh>> m_PrimitiveMeshes;
    std::unordered_map<std::string, std::unique_ptr<Mesh>> m_LoadedMeshes;

    // Materials
    std::vector<std::unique_ptr<Material>> m_Materials;
    std::vector<std::string> m_MaterialNames;

    // Textures
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_Textures;
    std::shared_ptr<Texture2D> m_WhiteTexture;
};

}  // namespace PyEngine
