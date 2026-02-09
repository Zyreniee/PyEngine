#include "PyEngine/Renderer/Buffer.hpp"
#include "PyEngine/Core/Assert.hpp"
#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Renderer/VulkanContext.hpp"
#include <cstring>

namespace PyEngine {

Buffer::Buffer(VulkanContext &context, VmaAllocator allocator,
               VkDeviceSize size, VkBufferUsageFlags usage,
               VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags)
    : m_Context(context), m_Allocator(allocator), m_Size(size) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo allocInfo{};
  allocInfo.usage = memoryUsage;
  allocInfo.flags = flags;

  VkResult result = vmaCreateBuffer(m_Allocator, &bufferInfo, &allocInfo,
                                    &m_Buffer, &m_Allocation, nullptr);
  PYENGINE_CORE_ASSERT(result == VK_SUCCESS, "Failed to create buffer!");
}

Buffer::~Buffer() {
  if (m_MappedData) {
    Unmap();
  }
  if (m_Buffer) {
    vmaDestroyBuffer(m_Allocator, m_Buffer, m_Allocation);
  }
}

void *Buffer::Map() {
  if (!m_MappedData) {
    vmaMapMemory(m_Allocator, m_Allocation, &m_MappedData);
  }
  return m_MappedData;
}

void Buffer::Unmap() {
  if (m_MappedData) {
    vmaUnmapMemory(m_Allocator, m_Allocation);
    m_MappedData = nullptr;
  }
}

void Buffer::CopyData(const void *data, size_t size) {
  PYENGINE_CORE_ASSERT(size <= m_Size, "Data size exceeds buffer size!");
  void *mappedData = Map();
  std::memcpy(mappedData, data, size);
  Unmap();
}

void Buffer::CopyBuffer(VulkanContext &context, VkCommandPool commandPool,
                        VkBuffer srcBuffer, VkBuffer dstBuffer,
                        VkDeviceSize size) {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(context.GetDevice(), &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  VkBufferCopy copyRegion{};
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(context.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(context.GetGraphicsQueue());

  vkFreeCommandBuffers(context.GetDevice(), commandPool, 1, &commandBuffer);
}

} // namespace PyEngine
