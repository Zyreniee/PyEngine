#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <functional>
#include <string>

struct GLFWwindow;

namespace PyEngine {

struct WindowProps {
    std::string Title;
    uint32_t Width;
    uint32_t Height;
    bool VSync;

    WindowProps(const std::string& title = "PyEngine", uint32_t width = 1920, uint32_t height = 1080, bool vsync = true)
        : Title(title), Width(width), Height(height), VSync(vsync) {}
};

class Window {
public:
    using ResizeCallbackFn = std::function<void(uint32_t, uint32_t)>;

    Window(const WindowProps& props = WindowProps());
    ~Window();

    void PollEvents();
    bool ShouldClose() const;
    void Close();

    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    float GetAspectRatio() const { return static_cast<float>(m_Width) / static_cast<float>(m_Height); }

    GLFWwindow* GetNativeWindow() const { return m_Window; }

    VkSurfaceKHR CreateVulkanSurface(VkInstance instance);
    void SetResizeCallback(ResizeCallbackFn callback) { m_ResizeCallback = callback; }

    bool WasResized() const { return m_FramebufferResized; }
    void ResetResizedFlag() { m_FramebufferResized = false; }

    static void GetRequiredInstanceExtensions(uint32_t* count, const char*** extensions);

private:
    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
    GLFWwindow* m_Window;
    std::string m_Title;
    uint32_t m_Width;
    uint32_t m_Height;
    bool m_VSync;
    bool m_FramebufferResized = false;

    ResizeCallbackFn m_ResizeCallback;
};

}  // namespace PyEngine
