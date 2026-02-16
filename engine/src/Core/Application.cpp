#include "PyEngine/Core/Application.hpp"

#include "PyEngine/Core/Assert.hpp"
#include "PyEngine/Core/Layer.hpp"
#include "PyEngine/Core/Log.hpp"
#include "PyEngine/ImGui/ImGuiLayer.hpp"
#include "PyEngine/Platform/Input.hpp"
#include "PyEngine/Platform/Window.hpp"
#include "PyEngine/Renderer/Renderer.hpp"
#include "PyEngine/Renderer/VulkanContext.hpp"

namespace PyEngine {

Application* Application::s_Instance = nullptr;

Application::Application(const ApplicationSpecification& spec) : m_Specification(spec) {
    PYENGINE_CORE_ASSERT(!s_Instance, "Application already exists!");
    s_Instance = this;

    // Initialize logging
    Log::Init();
    PYENGINE_CORE_INFO("PyEngine {} initializing...", spec.Name);

    // Create window
    WindowProps windowProps(spec.Name, spec.WindowWidth, spec.WindowHeight);
    m_Window = std::make_unique<Window>(windowProps);

    // Initialize input
    Input::Init(m_Window->GetNativeWindow());

    // Create renderer
    m_Renderer = std::make_unique<Renderer>(*m_Window);

    // Create ImGui layer
    m_ImGuiLayer = std::make_unique<ImGuiLayer>(*m_Window, m_Renderer->GetContext(), *m_Renderer);

    PYENGINE_CORE_INFO("PyEngine initialized successfully");
    PYENGINE_CORE_INFO("GPU: {}", m_Renderer->GetContext().GetGPUName());
}

Application::~Application() {
    PYENGINE_CORE_INFO("PyEngine shutting down...");
    Log::Shutdown();
    s_Instance = nullptr;
}

void Application::Run() {
    PYENGINE_CORE_INFO("Entering main loop");

    while (m_Running) {
        float deltaTime = m_Timer.GetDeltaTime();
        m_Timer.Reset();
        m_DeltaTime = deltaTime;

        // FPS tracking
        m_FrameCount++;
        m_FPSTimer += deltaTime;
        if (m_FPSTimer >= 1.0f) {
            m_FPS = m_FrameCount / m_FPSTimer;
            m_FrameTime = m_FPSTimer / m_FrameCount * 1000.0f;
            m_FrameCount = 0;
            m_FPSTimer = 0.0f;
        }

        // Poll events
        m_Window->PollEvents();
        Input::Update();

        // Check window close
        if (m_Window->ShouldClose()) {
            m_Running = false;
            break;
        }

        // Update layers
        if (!m_Minimized) {
            for (Layer* layer : m_LayerStack) {
                layer->OnUpdate(deltaTime);
            }

            // Render
            m_Renderer->BeginFrame();

            if (m_Renderer->IsFrameInProgress()) {
                // ImGui rendering
                m_ImGuiLayer->Begin();

                for (Layer* layer : m_LayerStack) {
                    layer->OnImGuiRender();
                }

                m_ImGuiLayer->End();
                m_ImGuiLayer->RenderDrawData(m_Renderer->GetCurrentCommandBuffer());
            }

            m_Renderer->EndFrame();
        }
    }
}

void Application::Close() {
    m_Running = false;
}

void Application::OnEvent(Event& event) {
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) { return OnWindowClose(e); });
    dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { return OnWindowResize(e); });

    // Dispatch to layers in reverse order
    for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
        if (event.Handled)
            break;
        (*it)->OnEvent(event);
    }
}

void Application::PushLayer(Layer* layer) {
    m_LayerStack.PushLayer(layer);
}

void Application::PushOverlay(Layer* overlay) {
    m_LayerStack.PushOverlay(overlay);
}

bool Application::OnWindowClose(WindowCloseEvent& e) {
    m_Running = false;
    return true;
}

bool Application::OnWindowResize(WindowResizeEvent& e) {
    if (e.GetWidth() == 0 || e.GetHeight() == 0) {
        m_Minimized = true;
        return false;
    }
    m_Minimized = false;
    return false;
}

}  // namespace PyEngine
