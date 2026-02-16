#include "Panels/SceneViewPanel.hpp"

#include <imgui.h>

void SceneViewPanel::OnImGuiRender() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("\xef\x80\xb0  Scene");  // Icon: th-large

    m_Focused = ImGui::IsWindowFocused();
    m_Hovered = ImGui::IsWindowHovered();

    // Auto-focus if hovered and clicked
    if (m_Hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        ImGui::SetWindowFocus();
    }
    if (m_Hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImGui::SetWindowFocus();
    }

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    m_ViewportSize = viewportPanelSize;

    // ── Viewport background ─────────────────────────────────────
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Gradient background (dark gray to slightly lighter, like Unity)
    ImU32 topColor = IM_COL32(38, 38, 38, 255);     // #262626
    ImU32 bottomColor = IM_COL32(50, 50, 55, 255);  // #32323
    drawList->AddRectFilledMultiColor(cursorPos,
                                      ImVec2(cursorPos.x + viewportPanelSize.x, cursorPos.y + viewportPanelSize.y),
                                      topColor, topColor, bottomColor, bottomColor);

    // ── Grid lines in viewport ──────────────────────────────────
    float gridSize = 40.0f;
    ImU32 gridColor = IM_COL32(60, 60, 60, 80);
    ImU32 gridColorMajor = IM_COL32(70, 70, 70, 120);

    float startX = cursorPos.x;
    float startY = cursorPos.y;
    float endX = startX + viewportPanelSize.x;
    float endY = startY + viewportPanelSize.y;

    int lineIndex = 0;
    for (float x = startX; x < endX; x += gridSize) {
        ImU32 col = (lineIndex % 5 == 0) ? gridColorMajor : gridColor;
        drawList->AddLine(ImVec2(x, startY), ImVec2(x, endY), col, 1.0f);
        lineIndex++;
    }
    lineIndex = 0;
    for (float y = startY; y < endY; y += gridSize) {
        ImU32 col = (lineIndex % 5 == 0) ? gridColorMajor : gridColor;
        drawList->AddLine(ImVec2(startX, y), ImVec2(endX, y), col, 1.0f);
        lineIndex++;
    }

    // ── Center cross ────────────────────────────────────────────
    float cx = cursorPos.x + viewportPanelSize.x * 0.5f;
    float cy = cursorPos.y + viewportPanelSize.y * 0.5f;
    ImU32 crossColor = IM_COL32(100, 100, 100, 100);
    drawList->AddLine(ImVec2(cx - 15, cy), ImVec2(cx + 15, cy), crossColor, 1.0f);
    drawList->AddLine(ImVec2(cx, cy - 15), ImVec2(cx, cy + 15), crossColor, 1.0f);

    // ── XYZ Axis Gizmo (top-right corner) ───────────────────────
    float gizmoX = cursorPos.x + viewportPanelSize.x - 70.0f;
    float gizmoY = cursorPos.y + 60.0f;
    float axisLen = 35.0f;

    // X axis (Red, pointing right)
    drawList->AddLine(ImVec2(gizmoX, gizmoY), ImVec2(gizmoX + axisLen, gizmoY), IM_COL32(220, 60, 60, 255), 2.5f);
    drawList->AddText(ImVec2(gizmoX + axisLen + 3, gizmoY - 7), IM_COL32(220, 60, 60, 255), "X");

    // Y axis (Green, pointing up)
    drawList->AddLine(ImVec2(gizmoX, gizmoY), ImVec2(gizmoX, gizmoY - axisLen), IM_COL32(60, 200, 60, 255), 2.5f);
    drawList->AddText(ImVec2(gizmoX - 5, gizmoY - axisLen - 17), IM_COL32(60, 200, 60, 255), "Y");

    // Z axis (Blue, pointing diagonal)
    drawList->AddLine(ImVec2(gizmoX, gizmoY), ImVec2(gizmoX - axisLen * 0.5f, gizmoY + axisLen * 0.5f),
                      IM_COL32(60, 100, 230, 255), 2.5f);
    drawList->AddText(ImVec2(gizmoX - axisLen * 0.5f - 12, gizmoY + axisLen * 0.5f - 3), IM_COL32(60, 100, 230, 255),
                      "Z");

    // Gizmo origin dot
    drawList->AddCircleFilled(ImVec2(gizmoX, gizmoY), 4.0f, IM_COL32(200, 200, 200, 200));

    // ── Tool selector overlay (top-left) ────────────────────────
    ImGui::SetCursorScreenPos(ImVec2(cursorPos.x + 10, cursorPos.y + 10));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.49f, 0.71f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.22f, 0.42f, 0.63f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 0));

    ImGui::Button("W", ImVec2(26, 26));  // Move
    ImGui::SameLine();
    ImGui::Button("E", ImVec2(26, 26));  // Rotate
    ImGui::SameLine();
    ImGui::Button("R", ImVec2(26, 26));  // Scale

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);

    // ── Camera controls hint (bottom) ───────────────────────────
    if (m_Hovered) {
        float hintY = cursorPos.y + viewportPanelSize.y - 25.0f;
        float hintX = cursorPos.x + 10.0f;
        drawList->AddRectFilled(ImVec2(hintX - 4, hintY - 3), ImVec2(hintX + 370, hintY + 16),
                                IM_COL32(15, 15, 15, 200), 3.0f);
        drawList->AddText(ImVec2(hintX, hintY), IM_COL32(180, 180, 180, 200),
                          "RMB+WASD: Fly  |  MMB: Pan  |  Scroll: Zoom  |  Q/E: Up/Down");
    }

    // ── DEBUG OVERLAY (Top-Right) ──────────────────────────────
    {
        ImGui::SetCursorScreenPos(ImVec2(cursorPos.x + viewportPanelSize.x - 200, cursorPos.y + 10));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
        ImGui::Text("Focused: %d | Hovered: %d", m_Focused, m_Hovered);
        ImGui::SetCursorScreenPos(ImVec2(cursorPos.x + viewportPanelSize.x - 200, cursorPos.y + 25));
        ImGui::Text("RMB: %d | W: %d", ImGui::IsMouseDown(ImGuiMouseButton_Right), ImGui::IsKeyDown(ImGuiKey_W));
        ImGui::PopStyleColor();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}
