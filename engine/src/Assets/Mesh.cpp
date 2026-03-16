#include "PyEngine/Assets/Mesh.hpp"

#include <cstring>

#include "PyEngine/Renderer/Buffer.hpp"
#include "PyEngine/Renderer/VulkanContext.hpp"

namespace PyEngine {

Mesh::Mesh(VulkanContext& context, VmaAllocator allocator, const std::vector<Vertex>& vertices,
           const std::vector<uint32_t>& indices)
    : m_Context(context),
      m_Allocator(allocator),
      m_VertexCount(static_cast<uint32_t>(vertices.size())),
      m_IndexCount(static_cast<uint32_t>(indices.size())),
      m_Vertices(vertices),
      m_Indices(indices) {
    VkDeviceSize vertexBufferSize = sizeof(Vertex) * vertices.size();
    VkDeviceSize indexBufferSize = sizeof(uint32_t) * indices.size();

    // Vertex buffer
    VkBufferCreateInfo vertexBufferInfo{};
    vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferInfo.size = vertexBufferSize;
    vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    vmaCreateBuffer(m_Allocator, &vertexBufferInfo, &allocInfo, &m_VertexBuffer, &m_VertexAllocation, nullptr);

    // Index buffer
    VkBufferCreateInfo indexBufferInfo{};
    indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    indexBufferInfo.size = indexBufferSize;
    indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    indexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vmaCreateBuffer(m_Allocator, &indexBufferInfo, &allocInfo, &m_IndexBuffer, &m_IndexAllocation, nullptr);

    // Create staging buffers and upload
    Buffer stagingVertexBuffer(context, allocator, vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VMA_MEMORY_USAGE_CPU_ONLY);
    void* vData = stagingVertexBuffer.Map();
    memcpy(vData, vertices.data(), vertexBufferSize);
    stagingVertexBuffer.Unmap();

    Buffer stagingIndexBuffer(context, allocator, indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VMA_MEMORY_USAGE_CPU_ONLY);
    void* iData = stagingIndexBuffer.Map();
    memcpy(iData, indices.data(), indexBufferSize);
    stagingIndexBuffer.Unmap();

    // Copy via temporary command pool
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = context.GetQueueFamilies().GraphicsFamily.value();
    VkCommandPool commandPool;
    vkCreateCommandPool(context.GetDevice(), &poolInfo, nullptr, &commandPool);

    Buffer::CopyBuffer(context, commandPool, stagingVertexBuffer.GetBuffer(), m_VertexBuffer, vertexBufferSize);
    Buffer::CopyBuffer(context, commandPool, stagingIndexBuffer.GetBuffer(), m_IndexBuffer, indexBufferSize);

    vkDestroyCommandPool(context.GetDevice(), commandPool, nullptr);
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

Mesh* Mesh::CreateCube(VulkanContext& context, VmaAllocator allocator) {
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
        0,  1,  2,  2,  3,  0,   // Front
        4,  5,  6,  6,  7,  4,   // Back
        8,  9,  10, 10, 11, 8,   // Top
        12, 13, 14, 14, 15, 12,  // Bottom
        16, 17, 18, 18, 19, 16,  // Right
        20, 21, 22, 22, 23, 20   // Left
    };

    return new Mesh(context, allocator, vertices, indices);
}

Mesh* Mesh::CreateSphere(VulkanContext& context, VmaAllocator allocator, int segments, int rings) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (int y = 0; y <= rings; y++) {
        for (int x = 0; x <= segments; x++) {
            float xSeg = (float)x / (float)segments;
            float ySeg = (float)y / (float)rings;
            float xPos = std::cos(xSeg * 2.0f * 3.14159265f) * std::sin(ySeg * 3.14159265f);
            float yPos = std::cos(ySeg * 3.14159265f);
            float zPos = std::sin(xSeg * 2.0f * 3.14159265f) * std::sin(ySeg * 3.14159265f);

            glm::vec3 pos(xPos * 0.5f, yPos * 0.5f, zPos * 0.5f);
            glm::vec3 norm(xPos, yPos, zPos);
            glm::vec2 uv(xSeg, ySeg);
            vertices.push_back({pos, glm::normalize(norm), uv});
        }
    }

    for (int y = 0; y < rings; y++) {
        for (int x = 0; x < segments; x++) {
            uint32_t i0 = y * (segments + 1) + x;
            uint32_t i1 = i0 + 1;
            uint32_t i2 = (y + 1) * (segments + 1) + x;
            uint32_t i3 = i2 + 1;
            indices.insert(indices.end(), {i0, i2, i1, i1, i2, i3});
        }
    }

    return new Mesh(context, allocator, vertices, indices);
}

Mesh* Mesh::CreatePlane(VulkanContext& context, VmaAllocator allocator, float size) {
    float h = size * 0.5f;
    std::vector<Vertex> vertices = {
        {{-h, 0.0f, h}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{h, 0.0f, h}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{h, 0.0f, -h}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{-h, 0.0f, -h}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
    };
    std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};
    return new Mesh(context, allocator, vertices, indices);
}

Mesh* Mesh::CreateCylinder(VulkanContext& context, VmaAllocator allocator, int segments) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    float PI = 3.14159265f;

    // Side
    for (int i = 0; i <= segments; i++) {
        float angle = (float)i / (float)segments * 2.0f * PI;
        float x = std::cos(angle) * 0.5f;
        float z = std::sin(angle) * 0.5f;
        glm::vec3 norm = glm::normalize(glm::vec3(std::cos(angle), 0.0f, std::sin(angle)));
        float u = (float)i / (float)segments;
        vertices.push_back({{x, -0.5f, z}, norm, {u, 0.0f}});
        vertices.push_back({{x, 0.5f, z}, norm, {u, 1.0f}});
    }
    for (int i = 0; i < segments; i++) {
        uint32_t b = i * 2;
        indices.insert(indices.end(), {b, b + 2, b + 1, b + 1, b + 2, b + 3});
    }

    // Top cap center
    uint32_t topCenter = (uint32_t)vertices.size();
    vertices.push_back({{0.0f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 0.5f}});
    for (int i = 0; i <= segments; i++) {
        float angle = (float)i / (float)segments * 2.0f * PI;
        float x = std::cos(angle) * 0.5f;
        float z = std::sin(angle) * 0.5f;
        vertices.push_back({{x, 0.5f, z}, {0.0f, 1.0f, 0.0f}, {x + 0.5f, z + 0.5f}});
    }
    for (int i = 0; i < segments; i++) {
        indices.insert(indices.end(), {topCenter, topCenter + 1 + i + 1, topCenter + 1 + i});
    }

    // Bottom cap center
    uint32_t botCenter = (uint32_t)vertices.size();
    vertices.push_back({{0.0f, -0.5f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 0.5f}});
    for (int i = 0; i <= segments; i++) {
        float angle = (float)i / (float)segments * 2.0f * PI;
        float x = std::cos(angle) * 0.5f;
        float z = std::sin(angle) * 0.5f;
        vertices.push_back({{x, -0.5f, z}, {0.0f, -1.0f, 0.0f}, {x + 0.5f, z + 0.5f}});
    }
    for (int i = 0; i < segments; i++) {
        indices.insert(indices.end(), {botCenter, botCenter + 1 + i, botCenter + 1 + i + 1});
    }

    return new Mesh(context, allocator, vertices, indices);
}

Mesh* Mesh::CreateCapsule(VulkanContext& context, VmaAllocator allocator, int segments, int rings) {
    // Simplified capsule: sphere top + cylinder mid + sphere bottom
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    float PI = 3.14159265f;
    float radius = 0.5f;
    float halfHeight = 0.5f;

    // Top hemisphere
    for (int y = 0; y <= rings; y++) {
        for (int x = 0; x <= segments; x++) {
            float xSeg = (float)x / (float)segments;
            float ySeg = (float)y / (float)rings * 0.5f;  // Only top half
            float xPos = std::cos(xSeg * 2.0f * PI) * std::sin(ySeg * PI);
            float yPos = std::cos(ySeg * PI);
            float zPos = std::sin(xSeg * 2.0f * PI) * std::sin(ySeg * PI);
            glm::vec3 pos(xPos * radius, yPos * radius + halfHeight, zPos * radius);
            glm::vec3 norm = glm::normalize(glm::vec3(xPos, yPos, zPos));
            vertices.push_back({pos, norm, {xSeg, ySeg}});
        }
    }

    for (int y = 0; y < rings; y++) {
        for (int x = 0; x < segments; x++) {
            uint32_t i0 = y * (segments + 1) + x;
            uint32_t i1 = i0 + 1;
            uint32_t i2 = (y + 1) * (segments + 1) + x;
            uint32_t i3 = i2 + 1;
            indices.insert(indices.end(), {i0, i2, i1, i1, i2, i3});
        }
    }

    // Bottom hemisphere (mirrored)
    uint32_t offset = (uint32_t)vertices.size();
    for (int y = 0; y <= rings; y++) {
        for (int x = 0; x <= segments; x++) {
            float xSeg = (float)x / (float)segments;
            float ySeg = 0.5f + (float)y / (float)rings * 0.5f;
            float xPos = std::cos(xSeg * 2.0f * PI) * std::sin(ySeg * PI);
            float yPos = std::cos(ySeg * PI);
            float zPos = std::sin(xSeg * 2.0f * PI) * std::sin(ySeg * PI);
            glm::vec3 pos(xPos * radius, yPos * radius - halfHeight, zPos * radius);
            glm::vec3 norm = glm::normalize(glm::vec3(xPos, yPos, zPos));
            vertices.push_back({pos, norm, {xSeg, ySeg}});
        }
    }

    for (int y = 0; y < rings; y++) {
        for (int x = 0; x < segments; x++) {
            uint32_t i0 = offset + y * (segments + 1) + x;
            uint32_t i1 = i0 + 1;
            uint32_t i2 = offset + (y + 1) * (segments + 1) + x;
            uint32_t i3 = i2 + 1;
            indices.insert(indices.end(), {i0, i2, i1, i1, i2, i3});
        }
    }

    return new Mesh(context, allocator, vertices, indices);
}

}  // namespace PyEngine
