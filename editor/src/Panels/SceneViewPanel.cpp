#include "Panels/SceneViewPanel.hpp"
#include <cstdio>
#include <glm/gtc/type_ptr.hpp>
#include "PyEngine/Core/Application.hpp"
#include "PyEngine/Renderer/OffscreenRenderer.hpp"

#include <imgui.h>

void SceneViewPanel::OnImGuiRender() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});

    ImGuiWindowFlags sceneFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::Begin("\xef\x80\xb0  Scene", nullptr, sceneFlags);

    m_Focused = ImGui::IsWindowFocused();
    m_Hovered = ImGui::IsWindowHovered();

    if (m_Hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))  ImGui::SetWindowFocus();
    if (m_Hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) ImGui::SetWindowFocus();

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    if (viewportPanelSize.x < 1.0f) viewportPanelSize.x = 1.0f;
    if (viewportPanelSize.y < 1.0f) viewportPanelSize.y = 1.0f;
    m_ViewportSize = viewportPanelSize;

    ImVec2 wPos  = ImGui::GetWindowPos();
    ImVec2 wSize = ImGui::GetWindowSize();
    ImVec2 contentStart = ImGui::GetCursorScreenPos();
    m_ViewportMin = contentStart;

    // Track mouse position relative to viewport for picking
    ImVec2 mousePos = ImGui::GetMousePos();
    m_MouseViewportPos = {mousePos.x - contentStart.x, mousePos.y - contentStart.y};
    m_ClickedThisFrame = m_Hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)
                         && !ImGui::IsAnyItemActive() && !ImGui::GetIO().WantCaptureMouse;

    // ─── Display the 3D Scene ────────────────────────────────────
    if (m_OffscreenRenderer) {
        ImTextureID texID = m_OffscreenRenderer->GetImTextureID();
        ImGui::Image(texID, viewportPanelSize, ImVec2{0, 0}, ImVec2{1, 1});
    } else {
        // Fallback dark bg
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(contentStart,
            ImVec2(contentStart.x + viewportPanelSize.x, contentStart.y + viewportPanelSize.y),
            IM_COL32(15, 15, 15, 255));
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // ─── XYZ Axis Gizmo (top-right corner, always visible) ───────
    {
        float gizmoX  = contentStart.x + viewportPanelSize.x - 80.0f;
        float gizmoY  = contentStart.y + 55.0f;
        float axisLen = 28.0f;

        // X (Red)
        drawList->AddLine(ImVec2(gizmoX, gizmoY), ImVec2(gizmoX + axisLen, gizmoY),
                          IM_COL32(230,  60,  60, 255), 2.2f);
        drawList->AddText(ImVec2(gizmoX + axisLen + 2, gizmoY - 7),
                          IM_COL32(230,  60,  60, 255), "X");

        // Y (Green)
        drawList->AddLine(ImVec2(gizmoX, gizmoY), ImVec2(gizmoX, gizmoY - axisLen),
                          IM_COL32( 60, 200,  60, 255), 2.2f);
        drawList->AddText(ImVec2(gizmoX - 6, gizmoY - axisLen - 15),
                          IM_COL32( 60, 200,  60, 255), "Y");

        // Z (Blue, diagonal for depth hint)
        drawList->AddLine(ImVec2(gizmoX, gizmoY),
                          ImVec2(gizmoX - axisLen * 0.6f, gizmoY + axisLen * 0.6f),
                          IM_COL32( 60, 100, 230, 255), 2.2f);
        drawList->AddText(ImVec2(gizmoX - axisLen * 0.6f - 12, gizmoY + axisLen * 0.6f - 4),
                          IM_COL32( 60, 100, 230, 255), "Z");

        drawList->AddCircleFilled(ImVec2(gizmoX, gizmoY), 4.0f, IM_COL32(220, 220, 220, 200));
    }

    // ─── Tool selector (top-left) ─────────────────────────────────
    {
        ImGui::SetCursorScreenPos(ImVec2(contentStart.x + 8, contentStart.y + 8));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(3, 0));

        auto toolButton = [&](const char* label, GizmoOperation op) {
            bool active = (m_GizmoOp == op);
            if (active) {
                ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.22f, 0.44f, 0.72f, 1.00f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.52f, 0.82f, 1.00f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.18f, 0.38f, 0.65f, 1.00f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.10f, 0.10f, 0.10f, 0.82f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.44f, 0.72f, 0.88f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.18f, 0.38f, 0.65f, 1.00f));
            }

            if (ImGui::Button(label, ImVec2(26, 24))) {
                m_GizmoOp = op;
            }
            ImGui::PopStyleColor(3);
        };

        toolButton("W##move",   GizmoOperation::Translate);
        ImGui::SameLine();
        toolButton("E##rotate", GizmoOperation::Rotate);
        ImGui::SameLine();
        toolButton("R##scale",  GizmoOperation::Scale);

        ImGui::PopStyleVar(2);
    }

    // ─── Viewport Stats (Top-Right) ──────────────────────────────
    {
        float statsX = contentStart.x + viewportPanelSize.x - 130.0f;
        float statsY = contentStart.y + 10.0f;
        drawList->AddRectFilled(ImVec2(statsX - 5, statsY - 2), ImVec2(statsX + 120, statsY + 32), IM_COL32(20, 20, 20, 160), 4.0f);
        
        char statsBuf[64];
        snprintf(statsBuf, sizeof(statsBuf), "RES: %dx%d", (int)m_ViewportSize.x, (int)m_ViewportSize.y);
        drawList->AddText(ImVec2(statsX, statsY), IM_COL32(200, 200, 200, 255), statsBuf);
        
        snprintf(statsBuf, sizeof(statsBuf), "FPS: %.1f", PyEngine::Application::Get().GetFPS());
        drawList->AddText(ImVec2(statsX, statsY + 14), IM_COL32(100, 255, 100, 255), statsBuf);
    }

    // ─── Focused Border ──────────────────────────────────────────
    if (m_Focused) {
        drawList->AddRect(contentStart, ImVec2(contentStart.x + viewportPanelSize.x, contentStart.y + viewportPanelSize.y),
                          IM_COL32(66, 150, 250, 180), 0.0f, 0, 2.0f);
    }

    // ─── Camera hint bar (bottom, only when hovered) ─────────────
    if (m_Hovered) {
        float hintY = contentStart.y + viewportPanelSize.y - 24.0f;
        float hintX = contentStart.x + 8.0f;
        drawList->AddRectFilled(
            ImVec2(hintX - 4, hintY - 2),
            ImVec2(hintX + 480, hintY + 14),
            IM_COL32(8, 8, 8, 185), 3.0f);
        drawList->AddText(ImVec2(hintX, hintY), IM_COL32(165, 165, 165, 200),
            "RMB+WASD: Fly   Alt+LMB: Orbit   MMB: Pan   Scroll: Zoom   F: Focus");
    }

    ImGui::End();

    ImGui::PopStyleVar();
}
