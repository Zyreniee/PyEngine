#pragma once

#include <imgui.h>
#include <glm/glm.hpp>

namespace PyEngine {
class OffscreenRenderer;
class EditorCamera;
class Scene;
class Entity;
}  // namespace PyEngine

enum class GizmoOperation { None = -1, Translate = 0, Rotate = 1, Scale = 2 };

class SceneViewPanel {
public:
    SceneViewPanel() = default;

    void OnImGuiRender();
    void OnGamePanelRender();

    bool IsFocused() const { return m_Focused; }
    bool IsHovered() const { return m_Hovered; }
    ImVec2 GetViewportSize() const { return m_ViewportSize; }
    ImVec2 GetViewportMin() const { return m_ViewportMin; }

    // Set by EditorLayer each frame so this panel can show the 3D scene
    void SetOffscreenRenderer(PyEngine::OffscreenRenderer* r) { m_OffscreenRenderer = r; }

    // Gizmo tool state
    GizmoOperation GetGizmoOperation() const { return m_GizmoOp; }
    void SetGizmoOperation(GizmoOperation op) { m_GizmoOp = op; }

    // Mouse position relative to viewport (for picking)
    glm::vec2 GetMouseViewportPos() const { return m_MouseViewportPos; }
    bool WasClickedThisFrame() const { return m_ClickedThisFrame; }

private:
    bool   m_Focused  = false;
    bool   m_Hovered  = false;
    ImVec2 m_ViewportSize = {1280, 720};
    ImVec2 m_ViewportMin = {0, 0};

    GizmoOperation m_GizmoOp = GizmoOperation::Translate;

    glm::vec2 m_MouseViewportPos = {0, 0};
    bool m_ClickedThisFrame = false;
    bool m_WasPlayingLastFrame = false;

    PyEngine::OffscreenRenderer* m_OffscreenRenderer = nullptr;
};
