#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <glm/glm.hpp>
#include <memory>

namespace PyEngine {

class VulkanContext;
class Swapchain;
class Pipeline;
class Window;
class Mesh;

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class Renderer {
public:
    Renderer(Window& window);
    ~Renderer();

    void BeginFrame();
    void EndFrame();

    void DrawMesh(Mesh* mesh, const glm::mat4& transform);
    void SetCamera(const glm::mat4& view, const glm::mat4& projection);

    VmaAllocator GetAllocator() const { return m_Allocator; }
    VulkanContext& GetContext() { return *m_Context; }

    bool IsFrameInProgress() const { return m_IsFrameStarted; }
    VkCommandBuffer GetCurrentCommandBuffer() const { return m_CommandBuffers[m_CurrentImageIndex]; }
    uint32_t GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }

private:
    void CreateRenderPass();
    void CreateDescriptorSetLayout();
    void CreatePipelineLayout();
    void CreatePipeline();
    void CreateCommandPool();
    void CreateCommandBuffers();
    void CreateSyncObjects();
    void CreateUniformBuffers();
    void CreateDescriptorPool();
    void CreateDescriptorSets();

    void RecreateSwapchain();
    void UpdateUniformBuffer(uint32_t currentImage);

private:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    Window& m_Window;
    std::unique_ptr<VulkanContext> m_Context;
    std::unique_ptr<Swapchain> m_Swapchain;
    std::unique_ptr<Pipeline> m_Pipeline;

    VmaAllocator m_Allocator = VK_NULL_HANDLE;

    VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

    VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_CommandBuffers;

    std::vector<VkSemaphore> m_ImageAvailableSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;
    std::vector<VkFence> m_InFlightFences;

    std::vector<VkBuffer> m_UniformBuffers;
    std::vector<VmaAllocation> m_UniformBufferAllocations;
    std::vector<void*> m_UniformBuffersMapped;

    VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_DescriptorSets;

    uint32_t m_CurrentImageIndex = 0;
    uint32_t m_CurrentFrameIndex = 0;
    bool m_IsFrameStarted = false;

    glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
    glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
};

}  // namespace PyEngine
