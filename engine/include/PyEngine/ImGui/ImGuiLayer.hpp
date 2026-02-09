#pragma once

#include <vulkan/vulkan.h>

namespace PyEngine {

class Window;
class VulkanContext;
class Renderer;

class ImGuiLayer {
public:
  ImGuiLayer(Window &window, VulkanContext &context, Renderer &renderer);
  ~ImGuiLayer();

  void Begin();
  void End();

  void RenderDrawData(VkCommandBuffer commandBuffer);

private:
  void SetupImGuiStyle();

private:
  Window &m_Window;
  VulkanContext &m_Context;
  Renderer &m_Renderer;

  VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
};

} // namespace PyEngine
