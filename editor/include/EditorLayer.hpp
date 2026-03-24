#pragma once

#include <vulkan/vulkan.h>

#include <memory>

#include "Panels/ConsolePanel.hpp"
#include "Panels/HierarchyPanel.hpp"
#include "Panels/InspectorPanel.hpp"
#include "Panels/ProjectPanel.hpp"
#include "Panels/SceneViewPanel.hpp"
#include "Panels/ToolbarPanel.hpp"
#include "PyEngine/Assets/Mesh.hpp"
#include "PyEngine/Core/Event.hpp"
#include "PyEngine/Core/Layer.hpp"
#include "PyEngine/Renderer/OffscreenRenderer.hpp"
#include "PyEngine/Renderer/Pipeline.hpp"
#include "PyEngine/Scene/EditorCamera.hpp"
#include "PyEngine/Scene/Scene.hpp"
#include "ImGuizmo.h"

class EditorLayer : public PyEngine::Layer {
public:
    EditorLayer();
    ~EditorLayer() override = default;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(float deltaTime) override;
    void OnImGuiRender() override;
    void OnEvent(PyEngine::Event& event) override;

private:
    void SetupDockspace();
    void BuildDefaultLayout(ImGuiID dockspace_id);
    void DrawMenuBar();

    void NewScene();
    void OpenScene();
    void SaveScene();
    void SaveSceneAs();

    bool OnKeyPressed(PyEngine::KeyPressedEvent& event);

    // ── Entity Selection & Picking ──────────────────────────────
    void SelectEntity(PyEngine::Entity entity);
    void DoMousePicking();
    void DrawSelectionOutline();
    void DrawColliderDebug();
    glm::vec2 ProjectToScreen(const glm::vec3& worldPos,
                              const glm::mat4& viewProj,
                              const glm::vec2& viewportSize,
                              const glm::vec2& viewportMin) const;

private:
    // Scene
    std::shared_ptr<PyEngine::Scene> m_ActiveScene;

    // Editor Camera
    PyEngine::EditorCamera m_EditorCamera;

    // Panels
    HierarchyPanel m_HierarchyPanel;
    InspectorPanel m_InspectorPanel;
    ConsolePanel m_ConsolePanel;
    SceneViewPanel m_SceneViewPanel;
    ProjectPanel m_ProjectPanel;
    ToolbarPanel m_ToolbarPanel;

    // Grid
    struct GridPushConstants {
        glm::mat4 View;
        glm::mat4 Proj;
    };

    std::unique_ptr<PyEngine::Pipeline> m_GridPipeline;
    VkPipelineLayout m_GridPipelineLayout = VK_NULL_HANDLE;
    std::unique_ptr<PyEngine::Mesh> m_GridMesh;

    void InitGrid();
    void DrawGrid(VkCommandBuffer cmd, const glm::mat4& view, const glm::mat4& proj);

    // State
    std::string m_CurrentScenePath;
    bool m_ShowDemoWindow = false;
    bool m_FirstFrame = true;
    float m_LastMouseX = 0.0f;
    float m_LastMouseY = 0.0f;
    glm::vec2 m_ViewportSize = {0.0f, 0.0f};
    bool m_ViewportFocused = false;
    bool m_ViewportHovered = false;

    // NavMesh Debug
    void InitNavMeshDebug();
    void UpdateNavMeshDebugMesh();
    void DrawNavMeshDebug(VkCommandBuffer commandBuffer, const glm::mat4& view, const glm::mat4& proj);

    VkPipelineLayout m_NavMeshPipelineLayout = VK_NULL_HANDLE;
    std::unique_ptr<PyEngine::Pipeline> m_NavMeshPipeline;
    std::shared_ptr<PyEngine::Mesh> m_NavMeshDebugMesh;
    bool m_ShowNavMesh = true;

    // ── Imports Modal ──────────────────────────────────────────
    bool m_ShowModelImportModal = false;
    char m_ImportPathBuffer[256] = "";

    // ── Offscreen Scene Renderer ───────────────────────────────
    std::unique_ptr<PyEngine::OffscreenRenderer> m_OffscreenRenderer;

    // ── Gizmo State ───────────────────────────────────────────
    bool m_UsingGizmo = false;
};
