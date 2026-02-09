#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <vector>

namespace PyEngine {

class VulkanContext;

class Buffer {
public:
    Buffer(VulkanContext& context, VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage,
           VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags = 0);
    ~Buffer();

    void* Map();
    void Unmap();
    void CopyData(const void* data, size_t size);

    VkBuffer GetBuffer() const { return m_Buffer; }
    VkDeviceSize GetSize() const { return m_Size; }

    static void CopyBuffer(VulkanContext& context, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer,
                           VkDeviceSize size);

private:
    VulkanContext& m_Context;
    VmaAllocator m_Allocator;

    VkBuffer m_Buffer = VK_NULL_HANDLE;
    VmaAllocation m_Allocation = VK_NULL_HANDLE;
    VkDeviceSize m_Size;
    void* m_MappedData = nullptr;
};

}  // namespace PyEngine
