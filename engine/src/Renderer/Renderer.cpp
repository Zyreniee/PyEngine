#include "PyEngine/Renderer/Renderer.hpp"

#include <array>
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>

#include "PyEngine/Assets/Mesh.hpp"
#include "PyEngine/Core/Assert.hpp"
#include "PyEngine/Core/FileSystem.hpp"
#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Platform/Window.hpp"
#include "PyEngine/Renderer/Pipeline.hpp"
#include "PyEngine/Renderer/Swapchain.hpp"
#include "PyEngine/Renderer/VulkanContext.hpp"

namespace PyEngine {

Renderer::Renderer(Window& window) : m_Window(window) {
    m_Context = std::make_unique<VulkanContext>(window);

    // Create VMA allocator
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = m_Context->GetPhysicalDevice();
    allocatorInfo.device = m_Context->GetDevice();
    allocatorInfo.instance = m_Context->GetInstance();
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

    vmaCreateAllocator(&allocatorInfo, &m_Allocator);

    m_Swapchain = std::make_unique<Swapchain>(*m_Context, window.GetWidth(), window.GetHeight());

    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreatePipelineLayout();

    m_Swapchain->CreateFramebuffers(m_RenderPass);

    CreatePipeline();
    CreateCommandPool();
    CreateCommandBuffers();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateSyncObjects();

    PYENGINE_CORE_INFO("Renderer initialized successfully");
}

Renderer::~Renderer() {
    vkDeviceWaitIdle(m_Context->GetDevice());

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(m_Context->GetDevice(), m_ImageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(m_Context->GetDevice(), m_RenderFinishedSemaphores[i], nullptr);
        vkDestroyFence(m_Context->GetDevice(), m_InFlightFences[i], nullptr);
        vmaDestroyBuffer(m_Allocator, m_UniformBuffers[i], m_UniformBufferAllocations[i]);
    }

    if (m_DescriptorPool)
        vkDestroyDescriptorPool(m_Context->GetDevice(), m_DescriptorPool, nullptr);
    if (m_CommandPool)
        vkDestroyCommandPool(m_Context->GetDevice(), m_CommandPool, nullptr);
    if (m_PipelineLayout)
        vkDestroyPipelineLayout(m_Context->GetDevice(), m_PipelineLayout, nullptr);
    if (m_DescriptorSetLayout)
        vkDestroyDescriptorSetLayout(m_Context->GetDevice(), m_DescriptorSetLayout, nullptr);
    if (m_RenderPass)
        vkDestroyRenderPass(m_Context->GetDevice(), m_RenderPass, nullptr);

    if (m_Allocator)
        vmaDestroyAllocator(m_Allocator);
}

void Renderer::BeginFrame() {
    PYENGINE_CORE_ASSERT(!m_IsFrameStarted, "Cannot call BeginFrame while already in progress");

    vkWaitForFences(m_Context->GetDevice(), 1, &m_InFlightFences[m_CurrentFrameIndex], VK_TRUE, UINT64_MAX);

    VkResult result =
        vkAcquireNextImageKHR(m_Context->GetDevice(), m_Swapchain->GetSwapchain(), UINT64_MAX,
                              m_ImageAvailableSemaphores[m_CurrentFrameIndex], VK_NULL_HANDLE, &m_CurrentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapchain();
        return;
    }
    PYENGINE_CORE_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Failed to acquire swap chain image!");

    vkResetFences(m_Context->GetDevice(), 1, &m_InFlightFences[m_CurrentFrameIndex]);

    vkResetCommandBuffer(m_CommandBuffers[m_CurrentImageIndex], 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(m_CommandBuffers[m_CurrentImageIndex], &beginInfo);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_RenderPass;
    renderPassInfo.framebuffer = m_Swapchain->GetFramebuffers()[m_CurrentImageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_Swapchain->GetExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.01f, 0.01f, 0.01f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(m_CommandBuffers[m_CurrentImageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_Swapchain->GetExtent().width);
    viewport.height = static_cast<float>(m_Swapchain->GetExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_CommandBuffers[m_CurrentImageIndex], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_Swapchain->GetExtent();
    vkCmdSetScissor(m_CommandBuffers[m_CurrentImageIndex], 0, 1, &scissor);

    m_IsFrameStarted = true;
}

void Renderer::EndFrame() {
    PYENGINE_CORE_ASSERT(m_IsFrameStarted, "Cannot call EndFrame if frame is not in progress");

    vkCmdEndRenderPass(m_CommandBuffers[m_CurrentImageIndex]);
    vkEndCommandBuffer(m_CommandBuffers[m_CurrentImageIndex]);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_ImageAvailableSemaphores[m_CurrentFrameIndex]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentImageIndex];

    VkSemaphore signalSemaphores[] = {m_RenderFinishedSemaphores[m_CurrentFrameIndex]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkResult result =
        vkQueueSubmit(m_Context->GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrameIndex]);
    PYENGINE_CORE_ASSERT(result == VK_SUCCESS, "Failed to submit draw command buffer!");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_Swapchain->GetSwapchain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &m_CurrentImageIndex;

    result = vkQueuePresentKHR(m_Context->GetPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window.WasResized()) {
        m_Window.ResetResizedFlag();
        RecreateSwapchain();
    } else {
        PYENGINE_CORE_ASSERT(result == VK_SUCCESS, "Failed to present swap chain image!");
    }

    m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    m_IsFrameStarted = false;
}

void Renderer::DrawMesh(Mesh* mesh, const glm::mat4& transform) {
    if (!m_IsFrameStarted || !m_Pipeline)
        return;

    UpdateUniformBuffer(m_CurrentFrameIndex);

    m_Pipeline->Bind(m_CommandBuffers[m_CurrentImageIndex]);
    vkCmdBindDescriptorSets(m_CommandBuffers[m_CurrentImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0,
                            1, &m_DescriptorSets[m_CurrentFrameIndex], 0, nullptr);

    mesh->Bind(m_CommandBuffers[m_CurrentImageIndex]);
    mesh->Draw(m_CommandBuffers[m_CurrentImageIndex]);
}

void Renderer::SetCamera(const glm::mat4& view, const glm::mat4& projection) {
    m_ViewMatrix = view;
    m_ProjectionMatrix = projection;
}

void Renderer::CreateRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_Swapchain->GetImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = m_Swapchain->GetDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkResult result = vkCreateRenderPass(m_Context->GetDevice(), &renderPassInfo, nullptr, &m_RenderPass);
    PYENGINE_CORE_ASSERT(result == VK_SUCCESS, "Failed to create render pass!");
}

void Renderer::CreateDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    VkResult result = vkCreateDescriptorSetLayout(m_Context->GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout);
    PYENGINE_CORE_ASSERT(result == VK_SUCCESS, "Failed to create descriptor set layout!");
}

void Renderer::CreatePipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;

    VkResult result = vkCreatePipelineLayout(m_Context->GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout);
    PYENGINE_CORE_ASSERT(result == VK_SUCCESS, "Failed to create pipeline layout!");
}

void Renderer::CreatePipeline() {
    auto exeDir = FileSystem::GetExecutableDirectory();
    std::string vertPath = (exeDir / "shaders" / "basic.vert.spv").string();
    std::string fragPath = (exeDir / "shaders" / "basic.frag.spv").string();

    // Check if shader files exist before trying to create pipeline
    if (!std::filesystem::exists(vertPath) || !std::filesystem::exists(fragPath)) {
        PYENGINE_CORE_WARN("Shader files not found, skipping pipeline creation");
        PYENGINE_CORE_WARN("  Expected: {} and {}", vertPath, fragPath);
        return;
    }

    PipelineConfig config;
    config.RenderPass = m_RenderPass;

    m_Pipeline = std::make_unique<Pipeline>(*m_Context, vertPath, fragPath, m_PipelineLayout, config);
}

void Renderer::CreateCommandPool() {
    QueueFamilyIndices queueFamilyIndices = m_Context->GetQueueFamilies();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();

    VkResult result = vkCreateCommandPool(m_Context->GetDevice(), &poolInfo, nullptr, &m_CommandPool);
    PYENGINE_CORE_ASSERT(result == VK_SUCCESS, "Failed to create command pool!");
}

void Renderer::CreateCommandBuffers() {
    m_CommandBuffers.resize(m_Swapchain->GetImageCount());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

    VkResult result = vkAllocateCommandBuffers(m_Context->GetDevice(), &allocInfo, m_CommandBuffers.data());
    PYENGINE_CORE_ASSERT(result == VK_SUCCESS, "Failed to allocate command buffers!");
}

void Renderer::CreateSyncObjects() {
    m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkCreateSemaphore(m_Context->GetDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]);
        vkCreateSemaphore(m_Context->GetDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]);
        vkCreateFence(m_Context->GetDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]);
    }
}

void Renderer::CreateUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    m_UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    m_UniformBufferAllocations.resize(MAX_FRAMES_IN_FLIGHT);
    m_UniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VmaAllocationInfo allocationInfo;
        vmaCreateBuffer(m_Allocator, &bufferInfo, &allocInfo, &m_UniformBuffers[i], &m_UniformBufferAllocations[i],
                        &allocationInfo);

        m_UniformBuffersMapped[i] = allocationInfo.pMappedData;
    }
}

void Renderer::CreateDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkResult result = vkCreateDescriptorPool(m_Context->GetDevice(), &poolInfo, nullptr, &m_DescriptorPool);
    PYENGINE_CORE_ASSERT(result == VK_SUCCESS, "Failed to create descriptor pool!");
}

void Renderer::CreateDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    m_DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    VkResult result = vkAllocateDescriptorSets(m_Context->GetDevice(), &allocInfo, m_DescriptorSets.data());
    PYENGINE_CORE_ASSERT(result == VK_SUCCESS, "Failed to allocate descriptor sets!");

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_UniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_DescriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(m_Context->GetDevice(), 1, &descriptorWrite, 0, nullptr);
    }
}

void Renderer::RecreateSwapchain() {
    vkDeviceWaitIdle(m_Context->GetDevice());

    m_Swapchain->Recreate(m_Window.GetWidth(), m_Window.GetHeight());
    m_Swapchain->CreateFramebuffers(m_RenderPass);

    CreateCommandBuffers();
}

void Renderer::UpdateUniformBuffer(uint32_t currentImage) {
    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.view = m_ViewMatrix;
    ubo.proj = m_ProjectionMatrix;

    memcpy(m_UniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

}  // namespace PyEngine
