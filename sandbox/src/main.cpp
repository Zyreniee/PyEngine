#include <PyEngine/Assets/Mesh.hpp>
#include <PyEngine/Core/Log.hpp>
#include <PyEngine/Core/Timer.hpp>
#include <PyEngine/ImGui/ImGuiLayer.hpp>
#include <PyEngine/Platform/Input.hpp>
#include <PyEngine/Platform/Window.hpp>
#include <PyEngine/Renderer/Renderer.hpp>
#include <PyEngine/Scene/Camera.hpp>
#include <PyEngine/Scene/Scene.hpp>

#include <imgui.h>
#include <memory>

int main() {
  // Initialize logging
  PyEngine::Log::Init();

  PYENGINE_INFO("PyEngine Sandbox starting...");

  // Create window
  PyEngine::WindowProps windowProps("PyEngine Sandbox", 1920, 1080);
  PyEngine::Window window(windowProps);

  // Initialize input
  PyEngine::Input::Init(window.GetNativeWindow());

  // Create renderer
  auto renderer = std::make_unique<PyEngine::Renderer>(window);

  // Create ImGui layer
  auto imguiLayer = std::make_unique<PyEngine::ImGuiLayer>(
      window, renderer->GetContext(), *renderer);

  // Create scene
  auto scene = std::make_unique<PyEngine::Scene>();

  // Create camera
  PyEngine::Camera camera(45.0f, window.GetAspectRatio(), 0.1f, 1000.0f);
  camera.SetPosition(glm::vec3(0.0f, 2.0f, 5.0f));

  // Create cube mesh
  auto cubeMesh = PyEngine::Mesh::CreateCube(renderer->GetContext(),
                                             renderer->GetAllocator());

  // Timer for delta time
  PyEngine::Timer timer;
  float rotation = 0.0f;

  // FPS tracking
  float fps = 0.0f;
  float frameTime = 0.0f;
  int frameCount = 0;
  float fpsTimer = 0.0f;

  PYENGINE_INFO("Entering main loop");

  // Main loop
  while (!window.ShouldClose()) {
    float deltaTime = timer.GetDeltaTime();
    timer.Reset();

    // Update FPS counter
    frameCount++;
    fpsTimer += deltaTime;
    if (fpsTimer >= 1.0f) {
      fps = frameCount / fpsTimer;
      frameTime = fpsTimer / frameCount * 1000.0f;
      frameCount = 0;
      fpsTimer = 0.0f;
    }

    // Poll events
    window.PollEvents();
    PyEngine::Input::Update();

    // Update camera
    camera.OnUpdate(deltaTime);

    // Update scene
    scene->OnUpdate(deltaTime);

    // Rotate cube
    rotation += deltaTime * 45.0f;

    // Render
    renderer->BeginFrame();

    if (renderer->IsFrameInProgress()) {
      // Set camera matrices
      renderer->SetCamera(camera.GetView(), camera.GetProjection());

      // Draw rotating cube
      glm::mat4 transform = glm::rotate(glm::mat4(1.0f), glm::radians(rotation),
                                        glm::vec3(0.5f, 1.0f, 0.2f));
      renderer->DrawMesh(cubeMesh, transform);

      // ImGui
      imguiLayer->Begin();

      // Dockspace
      ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

      // Debug window
      ImGui::Begin("Debug Info");
      ImGui::Text("FPS: %.1f", fps);
      ImGui::Text("Frame Time: %.2f ms", frameTime);
      ImGui::Text("GPU: %s", renderer->GetContext().GetGPUName().c_str());
      ImGui::Separator();
      ImGui::Text("Camera Position: %.2f, %.2f, %.2f", camera.GetPosition().x,
                  camera.GetPosition().y, camera.GetPosition().z);
      ImGui::Text("Camera Forward: %.2f, %.2f, %.2f", camera.GetForward().x,
                  camera.GetForward().y, camera.GetForward().z);
      ImGui::Separator();
      ImGui::Text("Controls:");
      ImGui::BulletText("WASD - Move camera");
      ImGui::BulletText("Space/Ctrl - Up/Down");
      ImGui::BulletText("Right Mouse - Look around");
      ImGui::BulletText("ESC - Exit");
      ImGui::End();

      imguiLayer->End();
      imguiLayer->RenderDrawData(renderer->GetCurrentCommandBuffer());
    }

    renderer->EndFrame();
  }

  // Cleanup
  delete cubeMesh;

  PYENGINE_INFO("PyEngine Sandbox shutting down");
  PyEngine::Log::Shutdown();

  return 0;
}
