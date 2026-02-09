#include "PyEngine/ImGui/ImGuiLayer.hpp"
#include "PyEngine/Core/Assert.hpp"
#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Platform/Window.hpp"
#include "PyEngine/Renderer/Renderer.hpp"
#include "PyEngine/Renderer/VulkanContext.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace PyEngine {

ImGuiLayer::ImGuiLayer(Window &window, VulkanContext &context,
                       Renderer &renderer)
    : m_Window(window), m_Context(context), m_Renderer(renderer) {
  // Create descriptor pool for ImGui
  VkDescriptorPoolSize pool_sizes[] = {
      {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  pool_info.maxSets = 1000;
  pool_info.poolSizeCount = 11;
  pool_info.pPoolSizes = pool_sizes;

  VkResult result = vkCreateDescriptorPool(m_Context.GetDevice(), &pool_info,
                                           nullptr, &m_DescriptorPool);
  PYENGINE_CORE_ASSERT(result == VK_SUCCESS,
                       "Failed to create ImGui descriptor pool!");

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  SetupImGuiStyle();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForVulkan(m_Window.GetNativeWindow(), true);

  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = m_Context.GetInstance();
  init_info.PhysicalDevice = m_Context.GetPhysicalDevice();
  init_info.Device = m_Context.GetDevice();
  init_info.QueueFamily = m_Context.GetQueueFamilies().GraphicsFamily.value();
  init_info.Queue = m_Context.GetGraphicsQueue();
  init_info.DescriptorPool = m_DescriptorPool;
  init_info.MinImageCount = 2;
  init_info.ImageCount = 2;
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

  // Note: This is a simplified version that doesn't create a separate render
  // pass for ImGui In production, you might want a separate render pass
  ImGui_ImplVulkan_Init(&init_info, VK_NULL_HANDLE);

  // Upload Fonts
  VkCommandPool command_pool = VK_NULL_HANDLE;
  VkCommandPoolCreateInfo pool_info_cmd = {};
  pool_info_cmd.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info_cmd.queueFamilyIndex =
      m_Context.GetQueueFamilies().GraphicsFamily.value();
  vkCreateCommandPool(m_Context.GetDevice(), &pool_info_cmd, nullptr,
                      &command_pool);

  VkCommandBuffer command_buffer;
  VkCommandBufferAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = command_pool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = 1;
  vkAllocateCommandBuffers(m_Context.GetDevice(), &alloc_info, &command_buffer);

  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(command_buffer, &begin_info);

  ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

  vkEndCommandBuffer(command_buffer);
  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer;
  vkQueueSubmit(m_Context.GetGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
  vkQueueWaitIdle(m_Context.GetGraphicsQueue());

  ImGui_ImplVulkan_DestroyFontUploadObjects();
  vkFreeCommandBuffers(m_Context.GetDevice(), command_pool, 1, &command_buffer);
  vkDestroyCommandPool(m_Context.GetDevice(), command_pool, nullptr);

  PYENGINE_CORE_INFO("ImGui layer initialized");
}

ImGuiLayer::~ImGuiLayer() {
  vkDeviceWaitIdle(m_Context.GetDevice());

  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  if (m_DescriptorPool) {
    vkDestroyDescriptorPool(m_Context.GetDevice(), m_DescriptorPool, nullptr);
  }
}

void ImGuiLayer::Begin() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ImGuiLayer::End() { ImGui::Render(); }

void ImGuiLayer::RenderDrawData(VkCommandBuffer commandBuffer) {
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void ImGuiLayer::SetupImGuiStyle() {
  ImGuiStyle &style = ImGui::GetStyle();

  // Dark theme
  ImGui::StyleColorsDark();

  // Rounding
  style.WindowRounding = 5.3f;
  style.FrameRounding = 2.3f;
  style.ScrollbarRounding = 0;

  // Colors
  ImVec4 *colors = style.Colors;
  colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.94f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.0f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.54f);
  colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.4f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.0f);
}

} // namespace PyEngine
