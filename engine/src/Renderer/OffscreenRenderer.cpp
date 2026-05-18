#include "PyEngine/Renderer/OffscreenRenderer.hpp"
#include "PyEngine/Renderer/VulkanContext.hpp"
#include "PyEngine/Core/Assert.hpp"
#include "PyEngine/Core/Log.hpp"
#include <backends/imgui_impl_vulkan.h>
#include <array>

namespace PyEngine {

OffscreenRenderer::OffscreenRenderer(VulkanContext& context, VmaAllocator allocator,
                                     uint32_t width, uint32_t height, VkFormat colorFormat)
    : m_Context(context), m_Allocator(allocator), m_Width(width), m_Height(height), m_ColorFormat(colorFormat) {
    Create();
}

OffscreenRenderer::~OffscreenRenderer() {
    Destroy();
}

void OffscreenRenderer::Resize(uint32_t width, uint32_t height) {
    // Prevent tiny framebuffers that cause rendering artifacts
    if (width < 64 || height < 64)
        return;
    if (m_Width == width && m_Height == height)
        return;

    vkDeviceWaitIdle(m_Context.GetDevice());
    Destroy();
    m_Width  = width;
    m_Height = height;
    Create();
}

void OffscreenRenderer::Create() {
    CreateRenderPass();
    CreateColorImage();
    CreateDepthImage();
    CreateFramebuffer();
    CreateCommandPool();
    CreateCommandBuffer();
    CreateSyncObjects();
    CreateSampler();
    RegisterWithImGui();
    m_Initialized = true;
    PYENGINE_CORE_INFO("OffscreenRenderer created: {}x{}", m_Width, m_Height);
}

void OffscreenRenderer::Destroy() {
    if (!m_Initialized) return;
    VkDevice dev = m_Context.GetDevice();

    vkWaitForFences(dev, 1, &m_Fence, VK_TRUE, UINT64_MAX);

    // ImGui doesn't have a RemoveTexture API in older versions; the descriptor
    // is owned by the ImGui descriptor pool and will be freed with it.

    vkDestroySampler(dev, m_Sampler, nullptr);
    vkDestroyFence(dev, m_Fence, nullptr);
    vkFreeCommandBuffers(dev, m_CommandPool, 1, &m_CommandBuffer);
    vkDestroyCommandPool(dev, m_CommandPool, nullptr);
    vkDestroyFramebuffer(dev, m_Framebuffer, nullptr);

    vkDestroyImageView(dev, m_DepthView, nullptr);
    vmaDestroyImage(m_Allocator, m_DepthImage, m_DepthAllocation);

    vkDestroyImageView(dev, m_ColorView, nullptr);
    vmaDestroyImage(m_Allocator, m_ColorImage, m_ColorAllocation);

    vkDestroyRenderPass(dev, m_RenderPass, nullptr);

    m_Initialized = false;
}

VkFormat OffscreenRenderer::FindDepthFormat() {
    std::vector<VkFormat> candidates = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                                        VK_FORMAT_D24_UNORM_S8_UINT};
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_Context.GetPhysicalDevice(), format, &props);
        if ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) ==
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return format;
        }
    }
    return VK_FORMAT_D32_SFLOAT;
}

// ────────────────────────────────────────────────────────────────────────────
void OffscreenRenderer::CreateRenderPass() {
    VkAttachmentDescription colorAttach{};
    colorAttach.format         = m_ColorFormat;  // matches swapchain pipeline
    colorAttach.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAttach.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttach.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttach.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttach.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentDescription depthAttach{};
    depthAttach.format         = FindDepthFormat();
    depthAttach.samples        = VK_SAMPLE_COUNT_1_BIT;
    depthAttach.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttach.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttach.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttach.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depthRef{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorRef;
    subpass.pDepthStencilAttachment = &depthRef;

    // Subpass dependency: match the main Renderer exactly to ensure pipeline compatibility
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttach, depthAttach};
    VkRenderPassCreateInfo rpInfo{};
    rpInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.attachmentCount = (uint32_t)attachments.size();
    rpInfo.pAttachments    = attachments.data();
    rpInfo.subpassCount    = 1;
    rpInfo.pSubpasses      = &subpass;
    rpInfo.dependencyCount = 1;
    rpInfo.pDependencies   = &dependency;

    VkResult r = vkCreateRenderPass(m_Context.GetDevice(), &rpInfo, nullptr, &m_RenderPass);
    PYENGINE_CORE_ASSERT(r == VK_SUCCESS, "OffscreenRenderer: Failed to create render pass");
}

void OffscreenRenderer::CreateColorImage() {
    VkImageCreateInfo imgInfo{};
    imgInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgInfo.imageType     = VK_IMAGE_TYPE_2D;
    imgInfo.format        = m_ColorFormat;  // must match render pass
    imgInfo.extent        = {m_Width, m_Height, 1};
    imgInfo.mipLevels     = 1;
    imgInfo.arrayLayers   = 1;
    imgInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imgInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imgInfo.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    vmaCreateImage(m_Allocator, &imgInfo, &allocInfo, &m_ColorImage, &m_ColorAllocation, nullptr);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image                           = m_ColorImage;
    viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format                          = m_ColorFormat;
    viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.layerCount     = 1;
    vkCreateImageView(m_Context.GetDevice(), &viewInfo, nullptr, &m_ColorView);
}

void OffscreenRenderer::CreateDepthImage() {
    VkImageCreateInfo imgInfo{};
    imgInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgInfo.imageType     = VK_IMAGE_TYPE_2D;
    imgInfo.format        = FindDepthFormat();
    imgInfo.extent        = {m_Width, m_Height, 1};
    imgInfo.mipLevels     = 1;
    imgInfo.arrayLayers   = 1;
    imgInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imgInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imgInfo.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    vmaCreateImage(m_Allocator, &imgInfo, &allocInfo, &m_DepthImage, &m_DepthAllocation, nullptr);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image                           = m_DepthImage;
    viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format                          = FindDepthFormat();
    viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.layerCount     = 1;
    vkCreateImageView(m_Context.GetDevice(), &viewInfo, nullptr, &m_DepthView);
}

void OffscreenRenderer::CreateFramebuffer() {
    std::array<VkImageView, 2> attachments = {m_ColorView, m_DepthView};

    VkFramebufferCreateInfo fbInfo{};
    fbInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.renderPass      = m_RenderPass;
    fbInfo.attachmentCount = (uint32_t)attachments.size();
    fbInfo.pAttachments    = attachments.data();
    fbInfo.width           = m_Width;
    fbInfo.height          = m_Height;
    fbInfo.layers          = 1;
    vkCreateFramebuffer(m_Context.GetDevice(), &fbInfo, nullptr, &m_Framebuffer);
}

void OffscreenRenderer::CreateCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_Context.GetQueueFamilies().GraphicsFamily.value();
    vkCreateCommandPool(m_Context.GetDevice(), &poolInfo, nullptr, &m_CommandPool);
}

void OffscreenRenderer::CreateCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = m_CommandPool;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(m_Context.GetDevice(), &allocInfo, &m_CommandBuffer);
}

void OffscreenRenderer::CreateSyncObjects() {
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(m_Context.GetDevice(), &fenceInfo, nullptr, &m_Fence);
}

void OffscreenRenderer::CreateSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter    = VK_FILTER_LINEAR;
    samplerInfo.minFilter    = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode   = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    vkCreateSampler(m_Context.GetDevice(), &samplerInfo, nullptr, &m_Sampler);
}

void OffscreenRenderer::RegisterWithImGui() {
    m_DescriptorSet = ImGui_ImplVulkan_AddTexture(m_Sampler, m_ColorView,
                                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

// ── Per-frame API ─────────────────────────────────────────────────────────

bool OffscreenRenderer::BeginFrame() {
    VkDevice dev = m_Context.GetDevice();
    vkWaitForFences(dev, 1, &m_Fence, VK_TRUE, UINT64_MAX);
    vkResetFences(dev, 1, &m_Fence);

    vkResetCommandBuffer(m_CommandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(m_CommandBuffer, &beginInfo);

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color        = {{0.08f, 0.08f, 0.08f, 1.0f}};  // Very dark bg
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo rpBegin{};
    rpBegin.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBegin.renderPass               = m_RenderPass;
    rpBegin.framebuffer              = m_Framebuffer;
    rpBegin.renderArea.offset        = {0, 0};
    rpBegin.renderArea.extent        = {m_Width, m_Height};
    rpBegin.clearValueCount          = (uint32_t)clearValues.size();
    rpBegin.pClearValues             = clearValues.data();

    vkCmdBeginRenderPass(m_CommandBuffer, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{0, 0, (float)m_Width, (float)m_Height, 0.0f, 1.0f};
    VkRect2D   scissor{{0, 0}, {m_Width, m_Height}};
    vkCmdSetViewport(m_CommandBuffer, 0, 1, &viewport);
    vkCmdSetScissor (m_CommandBuffer, 0, 1, &scissor);

    return true;
}

void OffscreenRenderer::EndFrame(VkCommandBuffer /*unused*/) {
    vkCmdEndRenderPass(m_CommandBuffer);
    vkEndCommandBuffer(m_CommandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &m_CommandBuffer;

    vkQueueSubmit(m_Context.GetGraphicsQueue(), 1, &submitInfo, m_Fence);
}

} // namespace PyEngine
