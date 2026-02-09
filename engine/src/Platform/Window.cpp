#include "PyEngine/Platform/Window.hpp"
#include "PyEngine/Core/Assert.hpp"
#include "PyEngine/Core/Log.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace PyEngine {

static bool s_GLFWInitialized = false;

static void GLFWErrorCallback(int error, const char *description) {
  PYENGINE_CORE_ERROR("GLFW Error ({}): {}", error, description);
}

Window::Window(const WindowProps &props)
    : m_Title(props.Title), m_Width(props.Width), m_Height(props.Height),
      m_VSync(props.VSync) {
  if (!s_GLFWInitialized) {
    int success = glfwInit();
    PYENGINE_CORE_ASSERT(success, "Failed to initialize GLFW!");
    glfwSetErrorCallback(GLFWErrorCallback);
    s_GLFWInitialized = true;
  }

  // Don't create OpenGL context
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  m_Window =
      glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
  PYENGINE_CORE_ASSERT(m_Window, "Failed to create GLFW window!");

  glfwSetWindowUserPointer(m_Window, this);
  glfwSetFramebufferSizeCallback(m_Window, FramebufferResizeCallback);

  PYENGINE_CORE_INFO("Window created: {} ({}x{})", m_Title, m_Width, m_Height);
}

Window::~Window() {
  if (m_Window) {
    glfwDestroyWindow(m_Window);
  }
  // Note: We don't terminate GLFW here as there might be multiple windows
}

void Window::PollEvents() { glfwPollEvents(); }

bool Window::ShouldClose() const { return glfwWindowShouldClose(m_Window); }

void Window::Close() { glfwSetWindowShouldClose(m_Window, GLFW_TRUE); }

VkSurfaceKHR Window::CreateVulkanSurface(VkInstance instance) {
  VkSurfaceKHR surface;
  VkResult result =
      glfwCreateWindowSurface(instance, m_Window, nullptr, &surface);
  PYENGINE_CORE_ASSERT(result == VK_SUCCESS,
                       "Failed to create Vulkan surface!");
  return surface;
}

void Window::GetRequiredInstanceExtensions(uint32_t *count,
                                           const char ***extensions) {
  *extensions = glfwGetRequiredInstanceExtensions(count);
}

void Window::FramebufferResizeCallback(GLFWwindow *window, int width,
                                       int height) {
  auto self = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  self->m_FramebufferResized = true;
  self->m_Width = static_cast<uint32_t>(width);
  self->m_Height = static_cast<uint32_t>(height);

  if (self->m_ResizeCallback) {
    self->m_ResizeCallback(self->m_Width, self->m_Height);
  }
}

void Window::KeyCallback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  // Handle ESC to close
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

} // namespace PyEngine
