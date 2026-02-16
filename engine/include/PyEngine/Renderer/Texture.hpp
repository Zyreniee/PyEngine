#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory>
#include <string>

namespace PyEngine {

class VulkanContext;

class Texture2D {
public:
    Texture2D(VulkanContext& context, VmaAllocator allocator, const std::string& filepath);
    Texture2D(VulkanContext& context, VmaAllocator allocator, uint32_t width, uint32_t height,
              const void* data = nullptr);
    ~Texture2D();

    void Bind(uint32_t slot = 0) const;

    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    VkImageView GetImageView() const { return m_ImageView; }
    VkSampler GetSampler() const { return m_Sampler; }
    const std::string& GetPath() const { return m_FilePath; }

    bool IsLoaded() const { return m_Loaded; }

    static std::shared_ptr<Texture2D> CreateWhiteTexture(VulkanContext& context, VmaAllocator allocator);

private:
    void CreateImage(uint32_t width, uint32_t height, const void* pixels);
    void CreateImageView();
    void CreateSampler();

private:
    VulkanContext& m_Context;
    VmaAllocator m_Allocator;

    VkImage m_Image = VK_NULL_HANDLE;
    VmaAllocation m_ImageAllocation = VK_NULL_HANDLE;
    VkImageView m_ImageView = VK_NULL_HANDLE;
    VkSampler m_Sampler = VK_NULL_HANDLE;

    uint32_t m_Width = 0;
    uint32_t m_Height = 0;
    std::string m_FilePath;
    bool m_Loaded = false;
};

}  // namespace PyEngine
