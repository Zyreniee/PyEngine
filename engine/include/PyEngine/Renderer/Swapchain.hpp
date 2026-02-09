#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace PyEngine {

class VulkanContext;

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR Capabilities;
    std::vector<VkSurfaceFormatKHR> Formats;
    std::vector<VkPresentModeKHR> PresentModes;
};

class Swapchain {
public:
    Swapchain(VulkanContext& context, uint32_t width, uint32_t height);
    ~Swapchain();

    void Recreate(uint32_t width, uint32_t height);

    VkSwapchainKHR GetSwapchain() const { return m_Swapchain; }
    VkFormat GetImageFormat() const { return m_ImageFormat; }
    VkExtent2D GetExtent() const { return m_Extent; }
    uint32_t GetImageCount() const { return static_cast<uint32_t>(m_Images.size()); }

    const std::vector<VkImage>& GetImages() const { return m_Images; }
    const std::vector<VkImageView>& GetImageViews() const { return m_ImageViews; }
    const std::vector<VkFramebuffer>& GetFramebuffers() const { return m_Framebuffers; }

    VkImage GetDepthImage() const { return m_DepthImage; }
    VkImageView GetDepthImageView() const { return m_DepthImageView; }
    VkFormat GetDepthFormat() const { return m_DepthFormat; }

    void CreateFramebuffers(VkRenderPass renderPass);

    static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

private:
    void CreateSwapchain();
    void CreateImageViews();
    void CreateDepthResources();
    void Cleanup();

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    VkFormat FindDepthFormat();

private:
    VulkanContext& m_Context;
    uint32_t m_Width;
    uint32_t m_Height;

    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_Images;
    std::vector<VkImageView> m_ImageViews;
    std::vector<VkFramebuffer> m_Framebuffers;

    VkFormat m_ImageFormat;
    VkExtent2D m_Extent;

    VkImage m_DepthImage = VK_NULL_HANDLE;
    VkDeviceMemory m_DepthImageMemory = VK_NULL_HANDLE;
    VkImageView m_DepthImageView = VK_NULL_HANDLE;
    VkFormat m_DepthFormat;
};

}  // namespace PyEngine
