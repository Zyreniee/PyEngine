#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <imgui.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace PyEngine {

class VulkanContext;

/// Renders the 3D scene to an off-screen image that can be sampled
/// by ImGui as a texture (for the Scene View panel).
class OffscreenRenderer {
public:
    OffscreenRenderer(VulkanContext& context, VmaAllocator allocator,
                      uint32_t width, uint32_t height, VkFormat colorFormat);
    ~OffscreenRenderer();

    // Call when the viewport is resized
    void Resize(uint32_t width, uint32_t height);

    // Per-frame API (call between BeginFrame / EndFrame)
    bool BeginFrame();
    void EndFrame(VkCommandBuffer cmdBuf);

    // The descriptor set to pass to ImGui::Image(...)
    ImTextureID GetImTextureID() const { return (ImTextureID)m_DescriptorSet; }

    VkRenderPass GetRenderPass() const { return m_RenderPass; }
    VkCommandBuffer GetCommandBuffer() const { return m_CommandBuffer; }

    uint32_t GetWidth()  const { return m_Width;  }
    uint32_t GetHeight() const { return m_Height; }
    VkFormat GetColorFormat() const { return m_ColorFormat; }

private:
    void Create();
    void Destroy();
    void CreateRenderPass();
    void CreateColorImage();
    void CreateDepthImage();
    void CreateFramebuffer();
    void CreateCommandPool();
    void CreateCommandBuffer();
    void CreateSyncObjects();
    void CreateSampler();
    void RegisterWithImGui();

    VkFormat FindDepthFormat();

private:
    VulkanContext& m_Context;
    VmaAllocator   m_Allocator;

    uint32_t m_Width  = 0;
    uint32_t m_Height = 0;

    VkRenderPass   m_RenderPass  = VK_NULL_HANDLE;
    VkFormat       m_ColorFormat = VK_FORMAT_B8G8R8A8_SRGB;  // matches swapchain

    // Color attachment
    VkImage        m_ColorImage      = VK_NULL_HANDLE;
    VmaAllocation  m_ColorAllocation = VK_NULL_HANDLE;
    VkImageView    m_ColorView       = VK_NULL_HANDLE;

    // Depth attachment
    VkImage        m_DepthImage      = VK_NULL_HANDLE;
    VmaAllocation  m_DepthAllocation = VK_NULL_HANDLE;
    VkImageView    m_DepthView       = VK_NULL_HANDLE;

    VkFramebuffer  m_Framebuffer = VK_NULL_HANDLE;

    // Command recording
    VkCommandPool   m_CommandPool   = VK_NULL_HANDLE;
    VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

    // Sync
    VkFence     m_Fence     = VK_NULL_HANDLE;

    // Sampler + ImGui descriptor set
    VkSampler       m_Sampler       = VK_NULL_HANDLE;
    VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;

    bool m_Initialized = false;
};

} // namespace PyEngine
