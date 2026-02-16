#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <glm/glm.hpp>
#include <vector>

namespace PyEngine {

class VulkanContext;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoord;
};

class Mesh {
public:
    Mesh(VulkanContext& context, VmaAllocator allocator, const std::vector<Vertex>& vertices,
         const std::vector<uint32_t>& indices);
    ~Mesh();

    void Bind(VkCommandBuffer commandBuffer);
    void Draw(VkCommandBuffer commandBuffer);

    uint32_t GetIndexCount() const { return m_IndexCount; }

    static Mesh* CreateCube(VulkanContext& context, VmaAllocator allocator);
    static Mesh* CreateSphere(VulkanContext& context, VmaAllocator allocator, int segments = 32, int rings = 16);
    static Mesh* CreatePlane(VulkanContext& context, VmaAllocator allocator, float size = 10.0f);
    static Mesh* CreateCylinder(VulkanContext& context, VmaAllocator allocator, int segments = 32);
    static Mesh* CreateCapsule(VulkanContext& context, VmaAllocator allocator, int segments = 32, int rings = 8);

    const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
    const std::vector<uint32_t>& GetIndices() const { return m_Indices; }

private:
    VulkanContext& m_Context;
    VmaAllocator m_Allocator;

    VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
    VmaAllocation m_VertexAllocation = VK_NULL_HANDLE;

    VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
    VmaAllocation m_IndexAllocation = VK_NULL_HANDLE;

    uint32_t m_VertexCount;
    uint32_t m_IndexCount;

    // CPU-side copy for physics/navmesh/picking
    std::vector<Vertex> m_Vertices;
    std::vector<uint32_t> m_Indices;
};

}  // namespace PyEngine
