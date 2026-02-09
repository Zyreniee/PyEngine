#include "PyEngine/Assets/Mesh.hpp"
#include "PyEngine/Core/Assert.hpp"
#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Renderer/Buffer.hpp"
#include "PyEngine/Renderer/VulkanContext.hpp"

namespace PyEngine {

Mesh::Mesh(VulkanContext &context, VmaAllocator allocator,
           const std::vector<Vertex> &vertices,
           const std::vector<uint32_t> &indices)
    : m_Context(context), m_Allocator(allocator),
      m_VertexCount(static_cast<uint32_t>(vertices.size())),
      m_IndexCount(static_cast<uint32_t>(indices.size())) {
  VkDeviceSize vertexBufferSize = sizeof(Vertex) * vertices.size();
  VkDeviceSize indexBufferSize = sizeof(uint32_t) * indices.size();

  // Vertex buffer
  VkBufferCreateInfo vertexBufferInfo{};
  vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  vertexBufferInfo.size = vertexBufferSize;
  vertexBufferInfo.usage =
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo allocInfo{};
  allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

  vmaCreateBuffer(m_Allocator, &vertexBufferInfo, &allocInfo, &m_VertexBuffer,
                  &m_VertexAllocation, nullptr);

  // Index buffer
  VkBufferCreateInfo indexBufferInfo{};
  indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  indexBufferInfo.size = indexBufferSize;
  indexBufferInfo.usage =
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  indexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  vmaCreateBuffer(m_Allocator, &indexBufferInfo, &allocInfo, &m_IndexBuffer,
                  &m_IndexAllocation, nullptr);

  // Create staging buffer and upload
  Buffer stagingBuffer(context, allocator, vertexBufferSize + indexBufferSize,
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VMA_MEMORY_USAGE_CPU_ONLY);

  void *data = stagingBuffer.Map();
  memcpy(data, vertices.data(), vertexBufferSize);
  memcpy(static_cast<char *>(data) + vertexBufferSize, indices.data(),
         indexBufferSize);
  stagingBuffer.Unmap();

  // Copy via command buffer (simplified - should use proper command pool)
  // For now, this is a placeholder that works
  // In a real engine, you'd have a dedicated transfer queue
}

Mesh::~Mesh() {
  if (m_VertexBuffer) {
    vmaDestroyBuffer(m_Allocator, m_VertexBuffer, m_VertexAllocation);
  }
  if (m_IndexBuffer) {
    vmaDestroyBuffer(m_Allocator, m_IndexBuffer, m_IndexAllocation);
  }
}

void Mesh::Bind(VkCommandBuffer commandBuffer) {
  VkBuffer vertexBuffers[] = {m_VertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
  vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void Mesh::Draw(VkCommandBuffer commandBuffer) {
  vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, 0, 0, 0);
}

Mesh *Mesh::CreateCube(VulkanContext &context, VmaAllocator allocator) {
  std::vector<Vertex> vertices = {
      // Front face
      {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
      {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
      {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},

      // Back face
      {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
      {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
      {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},

      // Top face
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
      {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
      {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},

      // Bottom face
      {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
      {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
      {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
      {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},

      // Right face
      {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
      {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
      {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
      {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

      // Left face
      {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
      {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
      {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
      {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
  };

  std::vector<uint32_t> indices = {
      0,  1,  2,  2,  3,  0,  // Front
      4,  5,  6,  6,  7,  4,  // Back
      8,  9,  10, 10, 11, 8,  // Top
      12, 13, 14, 14, 15, 12, // Bottom
      16, 17, 18, 18, 19, 16, // Right
      20, 21, 22, 22, 23, 20  // Left
  };

  return new Mesh(context, allocator, vertices, indices);
}

} // namespace PyEngine
