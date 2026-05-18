#include "EditorLayer.hpp"

#include <imgui.h>
#include <imgui_internal.h>

#include <functional>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

// Core Engine Headers
#include <GLFW/glfw3.h>

#include "PyEngine/Core/Application.hpp"
#include "PyEngine/Core/FileSystem.hpp"
#include "PyEngine/Core/KeyCodes.hpp"
#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Platform/Input.hpp"
#include "PyEngine/Platform/Window.hpp"

// Renderer Headers
#include "PyEngine/Assets/Mesh.hpp"
#include "PyEngine/Renderer/OffscreenRenderer.hpp"
#include "PyEngine/Renderer/Pipeline.hpp"
#include "PyEngine/Renderer/Renderer.hpp"
#include "PyEngine/Renderer/Swapchain.hpp"
#include "PyEngine/Renderer/VulkanContext.hpp"

// Scene Headers
#include "PyEngine/Assets/ModelImporter.hpp"
#include "PyEngine/Scene/Components.hpp"
#include "PyEngine/Scene/SceneManager.hpp"
#include "PyEngine/Scene/SceneSerializer.hpp"

EditorLayer::EditorLayer() : Layer("EditorLayer") {}

void EditorLayer::OnAttach() {
    // 16:9 aspect ratio default
    m_EditorCamera = PyEngine::EditorCamera(60.0f, 1.778f, 0.1f, 1000.0f);
    NewScene();

    // Create the offscreen renderer for the editor scene view
    auto& renderer = PyEngine::Application::Get().GetRenderer();
    m_OffscreenRenderer = std::make_unique<PyEngine::OffscreenRenderer>(
        renderer.GetContext(), renderer.GetAllocator(), 1280, 720, renderer.GetImageFormat());
    m_SceneViewPanel.SetOffscreenRenderer(m_OffscreenRenderer.get());

    InitGrid();
    InitNavMeshDebug();
}

void EditorLayer::OnDetach() {
    if (m_GridPipelineLayout) {
        vkDestroyPipelineLayout(PyEngine::Application::Get().GetRenderer().GetContext().GetDevice(),
                                m_GridPipelineLayout, nullptr);
    }
    if (m_NavMeshPipelineLayout) {
        vkDestroyPipelineLayout(PyEngine::Application::Get().GetRenderer().GetContext().GetDevice(),
                                m_NavMeshPipelineLayout, nullptr);
    }
}

void EditorLayer::OnUpdate(float deltaTime) {
    // Resize
    if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f) {
        m_EditorCamera.SetAspectRatio(m_ViewportSize.x / m_ViewportSize.y);
        m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
    }

    // ── Camera Input Handling: NUCLEAR OPTION ──────────────────
    using namespace PyEngine;

    // Get raw GLFW states for maximum reliability
    auto nativeWindow = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
    
    bool sceneViewActive = m_SceneViewPanel.IsFocused() || m_SceneViewPanel.IsHovered();
    
    bool rmbHeld = sceneViewActive && glfwGetMouseButton(nativeWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    bool mmbHeld = sceneViewActive && glfwGetMouseButton(nativeWindow, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
    bool lmbHeld = sceneViewActive && glfwGetMouseButton(nativeWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    bool altHeld = glfwGetKey(nativeWindow, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(nativeWindow, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS;

    // Ensure speed is very noticeable
    m_EditorCamera.SetMoveSpeed(30.0f);
    m_EditorCamera.SetRightMouseButton(rmbHeld);
    m_EditorCamera.SetMiddleMouseButton(mmbHeld);
    m_EditorCamera.SetLeftMouseButton(lmbHeld);
    m_EditorCamera.SetAltPressed(altHeld);

    if (rmbHeld || mmbHeld) {
        // LOCK CURSOR for reliable delta
        glfwSetInputMode(nativeWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // Movement keys
        m_EditorCamera.SetMoveForward(ImGui::IsKeyDown(ImGuiKey_W) ||
                                      glfwGetKey(nativeWindow, GLFW_KEY_W) == GLFW_PRESS);
        m_EditorCamera.SetMoveBackward(ImGui::IsKeyDown(ImGuiKey_S) ||
                                       glfwGetKey(nativeWindow, GLFW_KEY_S) == GLFW_PRESS);
        m_EditorCamera.SetMoveLeft(ImGui::IsKeyDown(ImGuiKey_A) || glfwGetKey(nativeWindow, GLFW_KEY_A) == GLFW_PRESS);
        m_EditorCamera.SetMoveRight(ImGui::IsKeyDown(ImGuiKey_D) || glfwGetKey(nativeWindow, GLFW_KEY_D) == GLFW_PRESS);
        m_EditorCamera.SetMoveUp(ImGui::IsKeyDown(ImGuiKey_E) || glfwGetKey(nativeWindow, GLFW_KEY_E) == GLFW_PRESS);
        m_EditorCamera.SetMoveDown(ImGui::IsKeyDown(ImGuiKey_Q) || glfwGetKey(nativeWindow, GLFW_KEY_Q) == GLFW_PRESS);
        m_EditorCamera.SetBoost(ImGui::IsKeyDown(ImGuiKey_LeftShift) ||
                                glfwGetKey(nativeWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);

        // Mouse Delta: Using raw GLFW input delta via Input class for maximum precision
        glm::vec2 delta = Input::GetMouseDelta();
        if (glm::length(delta) > 0.0001f) {
            m_EditorCamera.OnMouseMove(delta.x, delta.y);
        }

        m_EditorCamera.OnUpdate(deltaTime);
    } else {
        glfwSetInputMode(nativeWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        // Still call update to catch inertia or state resets
        m_EditorCamera.OnUpdate(deltaTime);
    }

    // Focus Logic (F)
    if (ImGui::IsKeyPressed(ImGuiKey_F) || glfwGetKey(nativeWindow, GLFW_KEY_F) == GLFW_PRESS) {
        auto selectedEntity = m_HierarchyPanel.GetSelectedEntity();
        if (selectedEntity && selectedEntity.HasComponent<PyEngine::TransformComponent>()) {
            const auto& tc = selectedEntity.GetComponent<PyEngine::TransformComponent>();
            m_EditorCamera.Focus(tc.Position);
        }
    }

    // Update renderer camera
    auto& renderer = PyEngine::Application::Get().GetRenderer();
    renderer.SetCamera(m_EditorCamera.GetViewMatrix(), m_EditorCamera.GetProjectionMatrix());

    if (PyEngine::Application::Get().GetRuntimeState() == PyEngine::Application::RuntimeState::Play) {
        m_ActiveScene->OnUpdateRuntime(deltaTime);

        // Optionally draw debug in play mode if requested
        if (m_ShowNavMesh) {
            // We need render pass info... actually DrawGrid/DrawNavMesh requires being inside a render pass.
            // OnUpdateRuntime calls Render() which DOES the render pass.
            // We can't easily inject into OnUpdateRuntime without modifying generic renderer.
            // But for Editor State, we control the render loop more directly?
            // Actually, in Editor, the renderer is global.
            // Let's stick to Editor mode drawing for now.
        }

    } else {
        // ── Editor mode: render into offscreen target ──────────────────
        auto& renderer = PyEngine::Application::Get().GetRenderer();

        if (m_ViewportSize.x > 64.0f && m_ViewportSize.y > 64.0f) {
            m_OffscreenRenderer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        }

        if (m_OffscreenRenderer->BeginFrame()) {
            VkCommandBuffer offCmd = m_OffscreenRenderer->GetCommandBuffer();

            // Intercept scene drawing into offscreen cmd buffer
            renderer.SetExternalCommandBuffer(offCmd);
            
            // This will call DrawMesh, which now goes to offCmd
            m_ActiveScene->OnUpdateEditor(deltaTime, m_EditorCamera);
            
            // Draw overlays over scene
            DrawGrid(offCmd, m_EditorCamera.GetViewMatrix(), m_EditorCamera.GetProjectionMatrix());
            DrawNavMeshDebug(offCmd, m_EditorCamera.GetViewMatrix(), m_EditorCamera.GetProjectionMatrix());
            
            renderer.SetExternalCommandBuffer(VK_NULL_HANDLE);

            m_OffscreenRenderer->EndFrame(offCmd);
        }
    }
}

void EditorLayer::OnImGuiRender() {
    SetupDockspace();

    m_SceneViewPanel.OnImGuiRender();
    m_SceneViewPanel.OnGamePanelRender();
    m_HierarchyPanel.OnImGuiRender();
    m_InspectorPanel.OnImGuiRender();
    m_ProjectPanel.OnImGuiRender();
    m_ConsolePanel.OnImGuiRender();

    // ── ImGuizmo Setup for this frame ──────────────────────────
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::BeginFrame();

    // ── Gizmo in Scene View ──────────────────────────────────
    auto selectedEntity = m_HierarchyPanel.GetSelectedEntity();
    if (selectedEntity && selectedEntity.HasComponent<PyEngine::TransformComponent>() &&
        m_SceneViewPanel.GetGizmoOperation() != GizmoOperation::None) {

        ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
        ImVec2 vpMin = m_SceneViewPanel.GetViewportMin();
        ImVec2 vpSize = m_SceneViewPanel.GetViewportSize();
        ImGuizmo::SetRect(vpMin.x, vpMin.y, vpSize.x, vpSize.y);

        // Camera matrices
        const glm::mat4& viewMat = m_EditorCamera.GetViewMatrix();
        const glm::mat4& projMat = m_EditorCamera.GetProjectionMatrix();

        // Entity transform
        auto& tc = selectedEntity.GetComponent<PyEngine::TransformComponent>();
        glm::mat4 transform = tc.GetTransformMatrix();

        // Map our enum to ImGuizmo
        ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;
        switch (m_SceneViewPanel.GetGizmoOperation()) {
            case GizmoOperation::Rotate: op = ImGuizmo::ROTATE; break;
            case GizmoOperation::Scale:  op = ImGuizmo::SCALE;  break;
            default: break;
        }

        // Snap (hold Ctrl for snap)
        bool snap = PyEngine::Input::IsKeyPressed(PyEngine::Key::LeftControl);
        float snapValue = (op == ImGuizmo::ROTATE) ? 15.0f : 0.5f;
        float snapValues[3] = {snapValue, snapValue, snapValue};

        ImGuizmo::Manipulate(glm::value_ptr(viewMat), glm::value_ptr(projMat),
                             op, ImGuizmo::LOCAL,
                             glm::value_ptr(transform),
                             nullptr,
                             snap ? snapValues : nullptr);

        m_UsingGizmo = ImGuizmo::IsUsing();

        if (ImGuizmo::IsUsing()) {
            glm::vec3 translation, scale, skew;
            glm::quat rotation;
            glm::vec4 perspective;
            glm::decompose(transform, scale, rotation, translation, skew, perspective);

            tc.Position = translation;
            tc.Scale = scale;

            // Convert quaternion to euler angles (degrees) directly
            // This avoids the delta accumulation drift from the previous approach
            glm::vec3 eulerRad = glm::eulerAngles(rotation);
            tc.Rotation = glm::degrees(eulerRad);
        }
    }

    // ── Selection Outline & Collider Debug (2D overlay) ──────────
    DrawSelectionOutline();
    DrawColliderDebug();

    // ── Mouse Picking ────────────────────────────────────────────
    if (m_SceneViewPanel.WasClickedThisFrame() && !m_UsingGizmo) {
        DoMousePicking();
    }

    // Stats
    ImGui::Begin("Stats");
    ImGui::Text("FPS: %.1f", PyEngine::Application::Get().GetFPS());
    ImGui::Text("Frame Time: %.3f ms", PyEngine::Application::Get().GetFrameTime());
    ImGui::End();

    if (m_ShowDemoWindow)
        ImGui::ShowDemoWindow(&m_ShowDemoWindow);

    // Import Modal
    if (m_ShowModelImportModal) {
        ImGui::OpenPopup("Import Model");
    }

    if (ImGui::BeginPopupModal("Import Model", &m_ShowModelImportModal, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter path to GLTF/GLB file:");
        ImGui::InputText("##path", m_ImportPathBuffer, sizeof(m_ImportPathBuffer));

        if (ImGui::Button("Import", ImVec2(120, 0))) {
            if (strlen(m_ImportPathBuffer) > 0) {
                // Determine absolute path if possible, or relative to cwd
                PyEngine::ModelImporter::ImportGLTF(m_ImportPathBuffer, m_ActiveScene.get());
                m_ShowModelImportModal = false;
            }
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            m_ShowModelImportModal = false;
        }
        ImGui::EndPopup();
    }

    // auto stats = PyEngine::Application::Get().GetRenderer().GetStats();
    // ImGui::Text("Draw Calls: %d", stats.DrawCalls);
    // ImGui::Text("Vertices: %d", stats.VertexCount);
    // ImGui::Text("Indices: %d", stats.IndexCount);

    // std::string name = "None";
    // // GetSelectedEntity returns Entity, check if valid
    // if (m_HierarchyPanel.GetSelectedEntity())
    //     name = m_HierarchyPanel.GetSelectedEntity().GetComponent<PyEngine::TagComponent>().Tag;
    // ImGui::Text("Selected Entity: %s", name.c_str());

    // Update Panel Contexts
    // Sync selection
    m_InspectorPanel.SetSelectedEntity(m_HierarchyPanel.GetSelectedEntity());

    // Get viewport size from SceneViewPanel
    ImVec2 panelSize = m_SceneViewPanel.GetViewportSize();
    m_ViewportSize = {panelSize.x, panelSize.y};
}

void EditorLayer::OnEvent(PyEngine::Event& event) {
    if (m_SceneViewPanel.IsFocused()) {
        // m_EditorCamera.OnEvent(event); // EditorCamera doesn't have OnEvent
        // Handle scroll if needed manually
        if (event.GetEventType() == PyEngine::EventType::MouseScrolled) {
            auto& e = (PyEngine::MouseScrolledEvent&)event;
            m_EditorCamera.OnMouseScroll(e.GetYOffset());
        }
    }

    PyEngine::EventDispatcher dispatcher(event);
    dispatcher.Dispatch<PyEngine::KeyPressedEvent>(std::bind(&EditorLayer::OnKeyPressed, this, std::placeholders::_1));
}

bool EditorLayer::OnKeyPressed(PyEngine::KeyPressedEvent& event) {
    // Shortcuts
    bool control = PyEngine::Input::IsKeyPressed(PyEngine::Key::LeftControl) ||
                   PyEngine::Input::IsKeyPressed(PyEngine::Key::RightControl);
    bool shift = PyEngine::Input::IsKeyPressed(PyEngine::Key::LeftShift) ||
                 PyEngine::Input::IsKeyPressed(PyEngine::Key::RightShift);

    switch (event.GetKeyCode()) {
        case PyEngine::Key::N: {
            if (control)
                NewScene();
            break;
        }
        case PyEngine::Key::O: {
            if (control)
                OpenScene();
            break;
        }
        case PyEngine::Key::S: {
            if (control && shift)
                SaveSceneAs();
            else if (control)
                SaveScene();
            break;
        }
        case PyEngine::Key::D: {
            if (control) {
                // Duplicate
            }
            break;
        }
        // Tool shortcuts (only when not typing in text fields)
        case PyEngine::Key::W: {
            if (!control && !shift && !ImGui::GetIO().WantTextInput)
                m_SceneViewPanel.SetGizmoOperation(GizmoOperation::Translate);
            break;
        }
        case PyEngine::Key::E: {
            if (!control && !shift && !ImGui::GetIO().WantTextInput)
                m_SceneViewPanel.SetGizmoOperation(GizmoOperation::Rotate);
            break;
        }
        case PyEngine::Key::R: {
            if (!control && !shift && !ImGui::GetIO().WantTextInput)
                m_SceneViewPanel.SetGizmoOperation(GizmoOperation::Scale);
            break;
        }
        case PyEngine::Key::Escape: {
            // Deselect
            SelectEntity({});
            break;
        }
    }
    return false;
}

void EditorLayer::SetupDockspace() {
    static bool dockspaceOpen = true;
    static bool opt_fullscreen_persistant = true;
    bool opt_fullscreen = opt_fullscreen_persistant;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    float minWinSizeX = style.WindowMinSize.x;
    style.WindowMinSize.x = 370.0f;

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        static bool first_time = true;
        if (first_time) {
            first_time = false;
            BuildDefaultLayout(dockspace_id);
        }
    }

    style.WindowMinSize.x = minWinSizeX;

    DrawMenuBar();

    ImGui::End();
}

void EditorLayer::BuildDefaultLayout(ImGuiID dockspace_id) {
    ImGui::DockBuilderRemoveNode(dockspace_id);  // Clear any existing layout
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

    ImGuiID dock_main_id = dockspace_id;
    ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);
    ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.3f, nullptr, &dock_main_id);
    ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.2f, nullptr, &dock_main_id);

    ImGui::DockBuilderDockWindow("Hierarchy", dock_id_left);
    ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
    ImGui::DockBuilderDockWindow("Project", dock_id_bottom);
    ImGui::DockBuilderDockWindow("Console", dock_id_bottom);
    ImGui::DockBuilderDockWindow("Scene", dock_main_id);
    ImGui::DockBuilderDockWindow("Game", dock_main_id);
    ImGui::DockBuilderDockWindow("Stats", dock_id_right);

    ImGui::DockBuilderFinish(dockspace_id);
}

void EditorLayer::DrawMenuBar() {
    if (ImGui::BeginMenuBar()) {
        // ── PyEngine Logo/Brand ──────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.7f, 1.0f, 1.0f));
        ImGui::TextUnformatted("PyEngine");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        ImGui::TextUnformatted("|");
        ImGui::PopStyleColor();
        ImGui::SameLine();

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) NewScene();
            if (ImGui::MenuItem("Open Scene...", "Ctrl+O")) OpenScene();
            ImGui::Separator();
            if (ImGui::MenuItem("Save", "Ctrl+S")) SaveScene();
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) SaveSceneAs();
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) PyEngine::Application::Get().Close();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            ImGui::MenuItem("Undo", "Ctrl+Z");
            ImGui::MenuItem("Redo", "Ctrl+Y");
            ImGui::Separator();
            ImGui::MenuItem("Cut", "Ctrl+X");
            ImGui::MenuItem("Copy", "Ctrl+C");
            ImGui::MenuItem("Paste", "Ctrl+V");
            ImGui::Separator();
            if (ImGui::MenuItem("Delete", "Del")) {
                if (m_HierarchyPanel.GetSelectedEntity()) {
                    m_ActiveScene->DestroyEntity(m_HierarchyPanel.GetSelectedEntity());
                    m_HierarchyPanel.SetSelectedEntity({});
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Assets")) {
            if (ImGui::MenuItem("Import New Asset...")) {
                m_ShowModelImportModal = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("GameObject")) {
            if (ImGui::MenuItem("Create Empty", "Ctrl+Shift+N")) {
                auto e = m_ActiveScene->CreateEntity("GameObject");
                SelectEntity(e);
            }
            ImGui::Separator();
            
            if (ImGui::BeginMenu("3D Object")) {
                if (ImGui::MenuItem("Cube")) {
                    auto e = m_ActiveScene->CreateEntity("Cube");
                    e.AddComponent<PyEngine::MeshRendererComponent>().MeshID = 0;
                    e.GetComponent<PyEngine::TransformComponent>().Position = {0.0f, 0.5f, 0.0f};
                    SelectEntity(e);
                }
                if (ImGui::MenuItem("Sphere")) {
                    auto e = m_ActiveScene->CreateEntity("Sphere");
                    e.AddComponent<PyEngine::MeshRendererComponent>().MeshID = 1;
                    e.GetComponent<PyEngine::TransformComponent>().Position = {0.0f, 0.5f, 0.0f};
                    SelectEntity(e);
                }
                if (ImGui::MenuItem("Plane")) {
                    auto e = m_ActiveScene->CreateEntity("Plane");
                    e.AddComponent<PyEngine::MeshRendererComponent>().MeshID = 2;
                    SelectEntity(e);
                }
                if (ImGui::MenuItem("Cylinder")) {
                    auto e = m_ActiveScene->CreateEntity("Cylinder");
                    e.AddComponent<PyEngine::MeshRendererComponent>().MeshID = 3;
                    e.GetComponent<PyEngine::TransformComponent>().Position = {0.0f, 0.5f, 0.0f};
                    SelectEntity(e);
                }
                if (ImGui::MenuItem("Capsule")) {
                    auto e = m_ActiveScene->CreateEntity("Capsule");
                    e.AddComponent<PyEngine::MeshRendererComponent>().MeshID = 4;
                    e.GetComponent<PyEngine::TransformComponent>().Position = {0.0f, 0.75f, 0.0f};
                    SelectEntity(e);
                }
                if (ImGui::MenuItem("Terrain")) {
                    auto entity = m_ActiveScene->CreateEntity("Terrain");
                    auto& mrc = entity.AddComponent<PyEngine::MeshRendererComponent>();
                    mrc.MeshID = 2;
                    entity.GetComponent<PyEngine::TransformComponent>().Scale = {10.0f, 1.0f, 10.0f};
                    entity.AddComponent<PyEngine::TerrainComponent>();
                    SelectEntity(entity);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Light")) {
                if (ImGui::MenuItem("Directional Light")) {
                    auto e = m_ActiveScene->CreateEntity("Directional Light");
                    auto& lc = e.AddComponent<PyEngine::LightComponent>();
                    lc.Intensity = 1.0f;
                    e.GetComponent<PyEngine::TransformComponent>().Rotation = {-50.0f, -30.0f, 0.0f};
                    SelectEntity(e);
                }
                if (ImGui::MenuItem("Point Light")) {
                    auto e = m_ActiveScene->CreateEntity("Point Light");
                    auto& lc = e.AddComponent<PyEngine::LightComponent>();
                    lc.LightType = PyEngine::LightComponent::Type::Point;
                    lc.Intensity = 1.5f;
                    lc.Range = 15.0f;
                    e.GetComponent<PyEngine::TransformComponent>().Position = {0.0f, 3.0f, 0.0f};
                    SelectEntity(e);
                }
                if (ImGui::MenuItem("Spot Light")) {
                    auto e = m_ActiveScene->CreateEntity("Spot Light");
                    auto& lc = e.AddComponent<PyEngine::LightComponent>();
                    lc.LightType = PyEngine::LightComponent::Type::Spot;
                    lc.Intensity = 2.0f;
                    e.GetComponent<PyEngine::TransformComponent>().Position = {0.0f, 3.0f, 0.0f};
                    e.GetComponent<PyEngine::TransformComponent>().Rotation = {-45.0f, 0.0f, 0.0f};
                    SelectEntity(e);
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::MenuItem("Camera")) {
                auto e = m_ActiveScene->CreateEntity("Camera");
                e.AddComponent<PyEngine::CameraComponent>();
                e.GetComponent<PyEngine::TransformComponent>().Position = {0.0f, 2.0f, 5.0f};
                e.GetComponent<PyEngine::TransformComponent>().Rotation = {-10.0f, 0.0f, 0.0f};
                SelectEntity(e);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Component")) {
            ImGui::TextDisabled("Use Inspector instead.");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window")) {
            if (ImGui::BeginMenu("AI")) {
                if (ImGui::MenuItem("Bake NavMesh")) {
                    m_ActiveScene->GetNavMeshSystem().BakeNavMesh(m_ActiveScene.get());
                    UpdateNavMeshDebugMesh();
                }
                ImGui::MenuItem("Show NavMesh", nullptr, &m_ShowNavMesh);
                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Show Demo Window", nullptr, m_ShowDemoWindow))
                m_ShowDemoWindow = !m_ShowDemoWindow;
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            ImGui::Text("PyEngine v0.1.0");
            ImGui::Separator();
            ImGui::TextDisabled("C++20 / Vulkan 1.3 / Python Scripting");
            ImGui::TextDisabled("by Pyrena Studios");
            ImGui::Separator();
            ImGui::TextDisabled("Scripting: Python via pybind11");
            ImGui::TextDisabled("ECS: EnTT");
            ImGui::TextDisabled("Physics: Custom Engine");
            ImGui::TextDisabled("AI: Behavior Tree + NavMesh");
            ImGui::EndMenu();
        }

        // ── Right-aligned scene name ─────────────────────────
        float rightOffset = ImGui::GetWindowContentRegionMax().x - 200.0f;
        if (rightOffset > ImGui::GetCursorPosX()) {
            ImGui::SameLine(rightOffset);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::Text("Scene: %s", m_ActiveScene ? m_ActiveScene->GetName().c_str() : "None");
            ImGui::PopStyleColor();
        }

        ImGui::EndMenuBar();
    }
}

void EditorLayer::NewScene() {
    m_ActiveScene = std::make_shared<PyEngine::Scene>("Untitled");
    m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
    m_HierarchyPanel.SetScene(m_ActiveScene);
    m_CurrentScenePath = "";

    // ── Camera ─────────────────────────────────────────────
    auto camera = m_ActiveScene->CreateEntity("Main Camera");
    camera.AddComponent<PyEngine::CameraComponent>();
    auto& camTc = camera.GetComponent<PyEngine::TransformComponent>();
    camTc.Position = {0.0f, 3.0f, 8.0f};
    camTc.Rotation = {-15.0f, 0.0f, 0.0f};

    // ── Floor ──────────────────────────────────────────────
    auto floor = m_ActiveScene->CreateEntity("Floor");
    auto& floorMesh = floor.AddComponent<PyEngine::MeshRendererComponent>();
    floorMesh.MeshID = 2;  // Plane
    floorMesh.ColorTint = {0.18f, 0.18f, 0.22f, 1.0f};
    floorMesh.Roughness = 0.95f;
    floorMesh.Metallic = 0.0f;
    auto& floorTc = floor.GetComponent<PyEngine::TransformComponent>();
    floorTc.Position = {0.0f, 0.0f, 0.0f};
    floorTc.Scale = {15.0f, 1.0f, 15.0f};

    // ── Center Cube ────────────────────────────────────────
    auto cube = m_ActiveScene->CreateEntity("Cube");
    auto& cubeMesh = cube.AddComponent<PyEngine::MeshRendererComponent>();
    cubeMesh.MeshID = 0;
    cubeMesh.ColorTint = {0.15f, 0.55f, 0.85f, 1.0f};
    cubeMesh.Metallic = 0.7f;
    cubeMesh.Roughness = 0.25f;
    auto& cubeTc = cube.GetComponent<PyEngine::TransformComponent>();
    cubeTc.Position = {0.0f, 0.5f, 0.0f};

    // ── Sphere (left) ──────────────────────────────────────
    auto sphere = m_ActiveScene->CreateEntity("Sphere");
    auto& sphereMesh = sphere.AddComponent<PyEngine::MeshRendererComponent>();
    sphereMesh.MeshID = 1;
    sphereMesh.ColorTint = {0.85f, 0.25f, 0.3f, 1.0f};
    sphereMesh.Metallic = 0.1f;
    sphereMesh.Roughness = 0.5f;
    auto& sphereTc = sphere.GetComponent<PyEngine::TransformComponent>();
    sphereTc.Position = {-3.0f, 0.5f, 0.0f};

    // ── Cylinder (right) ───────────────────────────────────
    auto cyl = m_ActiveScene->CreateEntity("Cylinder");
    auto& cylMesh = cyl.AddComponent<PyEngine::MeshRendererComponent>();
    cylMesh.MeshID = 3;
    cylMesh.ColorTint = {0.95f, 0.75f, 0.1f, 1.0f};
    cylMesh.Metallic = 0.9f;
    cylMesh.Roughness = 0.15f;
    auto& cylTc = cyl.GetComponent<PyEngine::TransformComponent>();
    cylTc.Position = {3.0f, 0.75f, 0.0f};
    cylTc.Scale = {0.8f, 1.5f, 0.8f};

    // ── Directional Light (main sun) ───────────────────────
    auto dirLight = m_ActiveScene->CreateEntity("Directional Light");
    auto& dLightComp = dirLight.AddComponent<PyEngine::LightComponent>();
    dLightComp.Color = {1.0f, 0.95f, 0.9f};
    dLightComp.Intensity = 1.0f;
    dLightComp.CastShadows = true;
    auto& dLightTc = dirLight.GetComponent<PyEngine::TransformComponent>();
    dLightTc.Rotation = {-50.0f, -30.0f, 0.0f};

    // ── Point Light (warm fill) ────────────────────────────
    auto pointLight = m_ActiveScene->CreateEntity("Point Light");
    auto& pLightComp = pointLight.AddComponent<PyEngine::LightComponent>();
    pLightComp.LightType = PyEngine::LightComponent::Type::Point;
    pLightComp.Color = {1.0f, 0.6f, 0.3f};
    pLightComp.Intensity = 1.5f;
    pLightComp.Range = 20.0f;
    auto& pLightTc = pointLight.GetComponent<PyEngine::TransformComponent>();
    pLightTc.Position = {-3.0f, 3.0f, 3.0f};

    // Reset editor camera to look at origin
    m_EditorCamera = PyEngine::EditorCamera(60.0f, 1.778f, 0.1f, 1000.0f);
}

void EditorLayer::OpenScene() {
    /*
    std::string filepath = PyEngine::FileDialogs::OpenFile("PyEngine Scene (*.pyscene)\0*.pyscene\0");
    if (!filepath.empty()) {
        m_ActiveScene = std::make_shared<PyEngine::Scene>();
        m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        m_HierarchyPanel.SetScene(m_ActiveScene);

        PyEngine::SceneSerializer serializer(m_ActiveScene);
        serializer.Deserialize(filepath);
        m_CurrentScenePath = filepath;
    }
    */
}

void EditorLayer::SaveScene() {
    if (!m_CurrentScenePath.empty()) {
        PyEngine::SceneSerializer serializer(m_ActiveScene);
        serializer.Serialize(m_CurrentScenePath);
    } else {
        SaveSceneAs();
    }
}

void EditorLayer::SaveSceneAs() {
    /*
    std::string filepath = PyEngine::FileDialogs::SaveFile("PyEngine Scene (*.pyscene)\0*.pyscene\0");
    if (!filepath.empty()) {
        PyEngine::SceneSerializer serializer(m_ActiveScene);
        serializer.Serialize(filepath);
        m_CurrentScenePath = filepath;
    }
    */
}

void EditorLayer::InitGrid() {
    auto& app = PyEngine::Application::Get();
    auto& renderer = app.GetRenderer();

    // Create Grid Pipeline Layout
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(GridPushConstants);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;  // No descriptor sets for now
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(renderer.GetContext().GetDevice(), &pipelineLayoutInfo, nullptr,
                               &m_GridPipelineLayout) != VK_SUCCESS) {
        PYENGINE_CORE_ERROR("Failed to create Grid pipeline layout!");
        return;
    }

    // Create Grid Pipeline
    auto exeDir = PyEngine::FileSystem::GetExecutableDirectory();
    std::string vertPath = (exeDir / "shaders" / "grid.vert.spv").string();
    std::string fragPath = (exeDir / "shaders" / "grid.frag.spv").string();

    PyEngine::PipelineConfig config;
    config.RenderPass = m_OffscreenRenderer->GetRenderPass();
    config.DepthTestEnable = true;
    config.DepthWriteEnable = false;      // Grid is transparent/overlay
    config.CullMode = VK_CULL_MODE_NONE;  // Double sided
    config.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    m_GridPipeline =
        std::make_unique<PyEngine::Pipeline>(renderer.GetContext(), vertPath, fragPath, m_GridPipelineLayout, config);

    // Create Grid Mesh (Just a quad)
    m_GridMesh.reset(PyEngine::Mesh::CreatePlane(renderer.GetContext(), renderer.GetAllocator(), 100.0f));  // Big plane
}

void EditorLayer::DrawGrid(VkCommandBuffer cmd, const glm::mat4& view, const glm::mat4& proj) {
    if (!m_GridPipeline || !m_GridMesh)
        return;

    m_GridPipeline->Bind(cmd);

    GridPushConstants push{};
    push.View = view;
    push.Proj = proj;

    vkCmdPushConstants(cmd, m_GridPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GridPushConstants), &push);

    m_GridMesh->Bind(cmd);
    m_GridMesh->Draw(cmd);
}

void EditorLayer::InitNavMeshDebug() {
    auto& app = PyEngine::Application::Get();
    auto& renderer = app.GetRenderer();

    // Create NavMesh Debug Pipeline Layout
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(GridPushConstants);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(renderer.GetContext().GetDevice(), &pipelineLayoutInfo, nullptr,
                               &m_NavMeshPipelineLayout) != VK_SUCCESS) {
        PYENGINE_CORE_ERROR("Failed to create NavMesh pipeline layout!");
        return;
    }

    // Create NavMesh Debug Pipeline
    auto exeDir = PyEngine::FileSystem::GetExecutableDirectory();
    std::string vertPath = (exeDir / "shaders" / "navmesh_debug.vert.spv").string();
    std::string fragPath = (exeDir / "shaders" / "navmesh_debug.frag.spv").string();

    PyEngine::PipelineConfig config;
    config.RenderPass = m_OffscreenRenderer->GetRenderPass();
    config.DepthTestEnable = true;
    config.DepthWriteEnable = false;
    config.CullMode = VK_CULL_MODE_NONE;
    // Use LINE_LIST for wireframe
    config.Topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    m_NavMeshPipeline = std::make_unique<PyEngine::Pipeline>(renderer.GetContext(), vertPath, fragPath,
                                                             m_NavMeshPipelineLayout, config);
}

void EditorLayer::UpdateNavMeshDebugMesh() {
    auto& system = m_ActiveScene->GetNavMeshSystem();
    auto navMesh = system.GetNavMesh();
    if (!navMesh)
        return;

    const auto& triangles = navMesh->GetTriangles();
    if (triangles.empty())
        return;

    std::vector<PyEngine::Vertex> vertices;
    std::vector<uint32_t> indices;

    for (const auto& tri : triangles) {
        uint32_t baseIndex = (uint32_t)vertices.size();

        // Push vertices
        vertices.push_back({tri.A, {0, 1, 0}, {0, 0}});
        vertices.push_back({tri.B, {0, 1, 0}, {0, 0}});
        vertices.push_back({tri.C, {0, 1, 0}, {0, 0}});

        // Add indices for lines: A-B, B-C, C-A
        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);

        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);

        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 0);
    }

    auto& app = PyEngine::Application::Get();
    // Create new mesh (replacing old one)
    m_NavMeshDebugMesh.reset(
        new PyEngine::Mesh(app.GetRenderer().GetContext(), app.GetRenderer().GetAllocator(), vertices, indices));
}

void EditorLayer::DrawNavMeshDebug(VkCommandBuffer commandBuffer, const glm::mat4& view, const glm::mat4& proj) {
    if (!m_ShowNavMesh || !m_NavMeshDebugMesh || !m_NavMeshPipeline)
        return;

    m_NavMeshPipeline->Bind(commandBuffer);

    GridPushConstants pushConsts;
    pushConsts.View = view;
    pushConsts.Proj = proj;

    vkCmdPushConstants(commandBuffer, m_NavMeshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GridPushConstants),
                       &pushConsts);

    m_NavMeshDebugMesh->Bind(commandBuffer);
    m_NavMeshDebugMesh->Draw(commandBuffer);
}

// ══════════════════════════════════════════════════════════════════
// Entity Selection & Picking
// ══════════════════════════════════════════════════════════════════

void EditorLayer::SelectEntity(PyEngine::Entity entity) {
    m_HierarchyPanel.SetSelectedEntity(entity);
    m_InspectorPanel.SetSelectedEntity(entity);
}

glm::vec2 EditorLayer::ProjectToScreen(const glm::vec3& worldPos,
                                        const glm::mat4& viewProj,
                                        const glm::vec2& viewportSize,
                                        const glm::vec2& viewportMin) const {
    glm::vec4 clip = viewProj * glm::vec4(worldPos, 1.0f);
    if (clip.w <= 0.0001f) return {-1, -1};
    glm::vec3 ndc = glm::vec3(clip) / clip.w;
    float sx = viewportMin.x + (ndc.x * 0.5f + 0.5f) * viewportSize.x;
    float sy = viewportMin.y + (1.0f - (ndc.y * 0.5f + 0.5f)) * viewportSize.y;
    return {sx, sy};
}

void EditorLayer::DoMousePicking() {
    glm::vec2 mouseVP = m_SceneViewPanel.GetMouseViewportPos();
    ImVec2 vpSize = m_SceneViewPanel.GetViewportSize();
    if (vpSize.x < 1 || vpSize.y < 1) return;

    // Normalized device coordinates
    float ndcX = (2.0f * mouseVP.x / vpSize.x) - 1.0f;
    float ndcY = 1.0f - (2.0f * mouseVP.y / vpSize.y);

    // Build ray from camera
    glm::mat4 invProj = glm::inverse(m_EditorCamera.GetProjectionMatrix());
    glm::mat4 invView = glm::inverse(m_EditorCamera.GetViewMatrix());

    glm::vec4 rayClip(ndcX, ndcY, -1.0f, 1.0f);
    glm::vec4 rayEye = invProj * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
    glm::vec3 rayDir = glm::normalize(glm::vec3(invView * rayEye));
    glm::vec3 rayOrigin = m_EditorCamera.GetPosition();

    // Test all meshes with AABB
    float closestT = std::numeric_limits<float>::max();
    PyEngine::Entity closestEntity;

    auto entities = m_ActiveScene->GetAllEntities();
    for (auto entity : entities) {
        if (!entity.HasComponent<PyEngine::TransformComponent>()) continue;
        if (!entity.HasComponent<PyEngine::MeshRendererComponent>()) continue;

        auto& tc = entity.GetComponent<PyEngine::TransformComponent>();
        glm::vec3 pos = tc.Position;
        glm::vec3 halfScale = tc.Scale * 0.5f;

        // AABB bounds
        glm::vec3 aabbMin = pos - halfScale;
        glm::vec3 aabbMax = pos + halfScale;

        // Ray-AABB intersection (slab method)
        float tmin = -std::numeric_limits<float>::max();
        float tmax = std::numeric_limits<float>::max();

        for (int i = 0; i < 3; i++) {
            if (std::abs(rayDir[i]) < 0.0001f) {
                if (rayOrigin[i] < aabbMin[i] || rayOrigin[i] > aabbMax[i]) {
                    tmin = std::numeric_limits<float>::max();
                    break;
                }
            } else {
                float t1 = (aabbMin[i] - rayOrigin[i]) / rayDir[i];
                float t2 = (aabbMax[i] - rayOrigin[i]) / rayDir[i];
                if (t1 > t2) std::swap(t1, t2);
                tmin = std::max(tmin, t1);
                tmax = std::min(tmax, t2);
                if (tmin > tmax) {
                    tmin = std::numeric_limits<float>::max();
                    break;
                }
            }
        }

        if (tmin > 0 && tmin < closestT) {
            closestT = tmin;
            closestEntity = entity;
        }
    }

    SelectEntity(closestEntity);  // Selects closest or deselects if nothing hit
}

void EditorLayer::DrawSelectionOutline() {
    auto selectedEntity = m_HierarchyPanel.GetSelectedEntity();
    if (!selectedEntity || !selectedEntity.HasComponent<PyEngine::TransformComponent>()) return;

    auto& tc = selectedEntity.GetComponent<PyEngine::TransformComponent>();
    glm::vec3 pos = tc.Position;
    glm::vec3 halfScale = tc.Scale * 0.5f;

    glm::mat4 viewProj = m_EditorCamera.GetViewProjectionMatrix();
    ImVec2 vpSize = m_SceneViewPanel.GetViewportSize();
    ImVec2 vpMin  = m_SceneViewPanel.GetViewportMin();
    glm::vec2 vpSizeVec = {vpSize.x, vpSize.y};
    glm::vec2 vpMinVec  = {vpMin.x, vpMin.y};

    // 8 corners of the AABB
    glm::vec3 corners[8] = {
        pos + glm::vec3(-halfScale.x, -halfScale.y, -halfScale.z),
        pos + glm::vec3( halfScale.x, -halfScale.y, -halfScale.z),
        pos + glm::vec3( halfScale.x,  halfScale.y, -halfScale.z),
        pos + glm::vec3(-halfScale.x,  halfScale.y, -halfScale.z),
        pos + glm::vec3(-halfScale.x, -halfScale.y,  halfScale.z),
        pos + glm::vec3( halfScale.x, -halfScale.y,  halfScale.z),
        pos + glm::vec3( halfScale.x,  halfScale.y,  halfScale.z),
        pos + glm::vec3(-halfScale.x,  halfScale.y,  halfScale.z),
    };

    glm::vec2 screenCorners[8];
    bool allValid = true;
    for (int i = 0; i < 8; i++) {
        screenCorners[i] = ProjectToScreen(corners[i], viewProj, vpSizeVec, vpMinVec);
        if (screenCorners[i].x < -5000) allValid = false;
    }
    if (!allValid) return;

    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    ImU32 color = IM_COL32(255, 165, 0, 220);  // Orange selection
    float thickness = 2.0f;

    // 12 edges of the box
    int edges[12][2] = {
        {0,1}, {1,2}, {2,3}, {3,0},  // front face
        {4,5}, {5,6}, {6,7}, {7,4},  // back face
        {0,4}, {1,5}, {2,6}, {3,7}   // connecting edges
    };

    for (auto& e : edges) {
        drawList->AddLine(
            ImVec2(screenCorners[e[0]].x, screenCorners[e[0]].y),
            ImVec2(screenCorners[e[1]].x, screenCorners[e[1]].y),
            color, thickness);
    }
}

void EditorLayer::DrawColliderDebug() {
    auto selectedEntity = m_HierarchyPanel.GetSelectedEntity();
    if (!selectedEntity) return;

    glm::mat4 viewProj = m_EditorCamera.GetViewProjectionMatrix();
    ImVec2 vpSize = m_SceneViewPanel.GetViewportSize();
    ImVec2 vpMin  = m_SceneViewPanel.GetViewportMin();
    glm::vec2 vpSizeVec = {vpSize.x, vpSize.y};
    glm::vec2 vpMinVec  = {vpMin.x, vpMin.y};

    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    // ── Box Collider ─────────────────────────────────────────────
    if (selectedEntity.HasComponent<PyEngine::BoxColliderComponent>()) {
        auto& bc = selectedEntity.GetComponent<PyEngine::BoxColliderComponent>();
        auto& tc = selectedEntity.GetComponent<PyEngine::TransformComponent>();

        glm::vec3 center = tc.Position + bc.Center;
        glm::vec3 halfSize = bc.Size * 0.5f * tc.Scale;

        glm::vec3 corners[8] = {
            center + glm::vec3(-halfSize.x, -halfSize.y, -halfSize.z),
            center + glm::vec3( halfSize.x, -halfSize.y, -halfSize.z),
            center + glm::vec3( halfSize.x,  halfSize.y, -halfSize.z),
            center + glm::vec3(-halfSize.x,  halfSize.y, -halfSize.z),
            center + glm::vec3(-halfSize.x, -halfSize.y,  halfSize.z),
            center + glm::vec3( halfSize.x, -halfSize.y,  halfSize.z),
            center + glm::vec3( halfSize.x,  halfSize.y,  halfSize.z),
            center + glm::vec3(-halfSize.x,  halfSize.y,  halfSize.z),
        };

        glm::vec2 sc[8];
        for (int i = 0; i < 8; i++)
            sc[i] = ProjectToScreen(corners[i], viewProj, vpSizeVec, vpMinVec);

        ImU32 color = IM_COL32(0, 255, 0, 180);
        int edges[12][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
        for (auto& e : edges)
            drawList->AddLine(ImVec2(sc[e[0]].x, sc[e[0]].y), ImVec2(sc[e[1]].x, sc[e[1]].y), color, 1.5f);
    }

    // ── Sphere Collider ──────────────────────────────────────────
    if (selectedEntity.HasComponent<PyEngine::SphereColliderComponent>()) {
        auto& sc = selectedEntity.GetComponent<PyEngine::SphereColliderComponent>();
        auto& tc = selectedEntity.GetComponent<PyEngine::TransformComponent>();

        glm::vec3 center = tc.Position + sc.Center;
        float maxScale = std::max({tc.Scale.x, tc.Scale.y, tc.Scale.z});
        float worldRadius = sc.Radius * maxScale;

        // Draw circle approximation — 3 rings (XY, XZ, YZ)
        ImU32 color = IM_COL32(0, 255, 0, 160);
        int segments = 32;

        auto drawCircle = [&](int axis1, int axis2) {
            glm::vec2 prevScreen;
            for (int i = 0; i <= segments; i++) {
                float angle = 2.0f * 3.14159265f * (float)i / (float)segments;
                glm::vec3 point = center;
                point[axis1] += cosf(angle) * worldRadius;
                point[axis2] += sinf(angle) * worldRadius;
                glm::vec2 screen = ProjectToScreen(point, viewProj, vpSizeVec, vpMinVec);
                if (i > 0 && prevScreen.x > -5000 && screen.x > -5000) {
                    drawList->AddLine(ImVec2(prevScreen.x, prevScreen.y),
                                     ImVec2(screen.x, screen.y), color, 1.5f);
                }
                prevScreen = screen;
            }
        };

        drawCircle(0, 1);  // XY
        drawCircle(0, 2);  // XZ
        drawCircle(1, 2);  // YZ
    }
}
