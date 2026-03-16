#pragma once

#include <imgui.h>
#include <glm/glm.hpp>

namespace PyEngine { class OffscreenRenderer; }

class SceneViewPanel {
public:
    SceneViewPanel() = default;

    void OnImGuiRender();

    bool IsFocused() const { return m_Focused; }
    bool IsHovered() const { return m_Hovered; }
    ImVec2 GetViewportSize() const { return m_ViewportSize; }

    // Set by EditorLayer each frame so this panel can show the 3D scene
    void SetOffscreenRenderer(PyEngine::OffscreenRenderer* r) { m_OffscreenRenderer = r; }

private:
    bool   m_Focused  = false;
    bool   m_Hovered  = false;
    ImVec2 m_ViewportSize = {1280, 720};

    PyEngine::OffscreenRenderer* m_OffscreenRenderer = nullptr;
};
