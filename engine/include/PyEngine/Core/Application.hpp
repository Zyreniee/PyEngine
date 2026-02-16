#pragma once

#include <memory>
#include <string>

#include "PyEngine/Core/Event.hpp"
#include "PyEngine/Core/LayerStack.hpp"
#include "PyEngine/Core/Timer.hpp"

namespace PyEngine {

class Window;
class Renderer;
class ImGuiLayer;
class Scene;

struct ApplicationSpecification {
    std::string Name = "PyEngine Application";
    uint32_t WindowWidth = 1920;
    uint32_t WindowHeight = 1080;
};

class Application {
public:
    explicit Application(const ApplicationSpecification& spec = {});
    virtual ~Application();

    void Run();
    void Close();

    void OnEvent(Event& event);

    void PushLayer(Layer* layer);
    void PushOverlay(Layer* overlay);

    Window& GetWindow() { return *m_Window; }
    Renderer& GetRenderer() { return *m_Renderer; }
    ImGuiLayer& GetImGuiLayer() { return *m_ImGuiLayer; }

    static Application& Get() { return *s_Instance; }

    // Editor state
    enum class RuntimeState { Edit, Play, Pause };
    RuntimeState GetRuntimeState() const { return m_RuntimeState; }
    void SetRuntimeState(RuntimeState state) { m_RuntimeState = state; }

    float GetDeltaTime() const { return m_DeltaTime; }
    float GetFPS() const { return m_FPS; }
    float GetFrameTime() const { return m_FrameTime; }

private:
    bool OnWindowClose(WindowCloseEvent& e);
    bool OnWindowResize(WindowResizeEvent& e);

private:
    static Application* s_Instance;

    ApplicationSpecification m_Specification;
    std::unique_ptr<Window> m_Window;
    std::unique_ptr<Renderer> m_Renderer;
    std::unique_ptr<ImGuiLayer> m_ImGuiLayer;

    LayerStack m_LayerStack;
    Timer m_Timer;

    bool m_Running = true;
    bool m_Minimized = false;
    float m_DeltaTime = 0.0f;

    // FPS tracking
    float m_FPS = 0.0f;
    float m_FrameTime = 0.0f;
    int m_FrameCount = 0;
    float m_FPSTimer = 0.0f;

    RuntimeState m_RuntimeState = RuntimeState::Edit;
};

// Defined by the client (editor or game)
Application* CreateApplication();

}  // namespace PyEngine
