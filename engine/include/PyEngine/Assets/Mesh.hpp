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

private:
    VulkanContext& m_Context;
    VmaAllocator m_Allocator;

    VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
    VmaAllocation m_VertexAllocation = VK_NULL_HANDLE;

    VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
    VmaAllocation m_IndexAllocation = VK_NULL_HANDLE;

    uint32_t m_VertexCount;
    uint32_t m_IndexCount;
};

}  // namespace PyEngine
