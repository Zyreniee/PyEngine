#pragma once

#include <imgui.h>

class SceneViewPanel {
public:
    SceneViewPanel() = default;

    void OnImGuiRender();

    bool IsFocused() const { return m_Focused; }
    bool IsHovered() const { return m_Hovered; }
    ImVec2 GetViewportSize() const { return m_ViewportSize; }

private:
    bool m_Focused = false;
    bool m_Hovered = false;
    ImVec2 m_ViewportSize = {0, 0};
};
