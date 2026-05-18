#include "Panels/SceneViewPanel.hpp"
#include <cstdio>
#include <glm/gtc/type_ptr.hpp>
#include "PyEngine/Core/Application.hpp"
#include "PyEngine/Renderer/OffscreenRenderer.hpp"

#include <imgui.h>

void SceneViewPanel::OnImGuiRender() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});

    ImGuiWindowFlags sceneFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::Begin("Scene", nullptr, sceneFlags);

    // ═══════════════════════════════════════════════════════════════
    // TOP BAR: Play / Pause / Step controls (Unity-style)
    // ═══════════════════════════════════════════════════════════════
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));

        // Dark toolbar background
        ImVec2 barStart = ImGui::GetCursorScreenPos();
        float barWidth = ImGui::GetContentRegionAvail().x;
        float barHeight = 28.0f;
        ImDrawList* barDraw = ImGui::GetWindowDrawList();
        barDraw->AddRectFilled(barStart, ImVec2(barStart.x + barWidth, barStart.y + barHeight),
                               IM_COL32(22, 22, 22, 255));
        barDraw->AddLine(ImVec2(barStart.x, barStart.y + barHeight),
                         ImVec2(barStart.x + barWidth, barStart.y + barHeight),
                         IM_COL32(10, 10, 10, 255));

        auto& app = PyEngine::Application::Get();
        auto state = app.GetRuntimeState();
        bool isPlaying = (state == PyEngine::Application::RuntimeState::Play ||
                          state == PyEngine::Application::RuntimeState::Pause);
        bool isPaused = (state == PyEngine::Application::RuntimeState::Pause);

        float btnW = 50.0f;
        float btnH = 20.0f;
        float totalBtns = btnW * 3 + 8.0f;
        float centerX = (barWidth - totalBtns) * 0.5f;

        ImGui::SetCursorPosX(centerX);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);

        // Play/Stop
        if (isPlaying) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.65f, 0.18f, 0.18f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.25f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.55f, 0.12f, 0.12f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.50f, 0.18f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.60f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.40f, 0.12f, 1.0f));
        }
        if (ImGui::Button(isPlaying ? "Stop" : "Play", ImVec2(btnW, btnH))) {
            if (!isPlaying)
                app.SetRuntimeState(PyEngine::Application::RuntimeState::Play);
            else
                app.SetRuntimeState(PyEngine::Application::RuntimeState::Edit);
        }
        ImGui::PopStyleColor(3);

        ImGui::SameLine();

        // Pause
        if (isPaused) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.55f, 0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f, 0.65f, 0.20f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.50f, 0.45f, 0.10f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.32f, 0.32f, 0.32f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.20f, 0.20f, 1.0f));
        }
        if (ImGui::Button("Pause", ImVec2(btnW, btnH))) {
            if (state == PyEngine::Application::RuntimeState::Play)
                app.SetRuntimeState(PyEngine::Application::RuntimeState::Pause);
            else if (isPaused)
                app.SetRuntimeState(PyEngine::Application::RuntimeState::Play);
        }
        ImGui::PopStyleColor(3);

        ImGui::SameLine();

        // Step
        bool canStep = isPaused;
        if (!canStep) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.3f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.32f, 0.32f, 0.32f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.20f, 0.20f, 1.0f));
        ImGui::Button("Step", ImVec2(btnW, btnH));
        ImGui::PopStyleColor(3);
        if (!canStep) ImGui::PopStyleVar();

        // State label (right side)
        {
            ImGui::SameLine(barWidth - 180);
            ImVec4 stateColor;
            const char* stateText;
            if (isPlaying && !isPaused) {
                stateColor = ImVec4(0.3f, 0.9f, 0.3f, 1.0f);
                stateText = "PLAYING";
            } else if (isPaused) {
                stateColor = ImVec4(0.9f, 0.9f, 0.3f, 1.0f);
                stateText = "PAUSED";
            } else {
                stateColor = ImVec4(0.45f, 0.45f, 0.45f, 1.0f);
                stateText = "EDIT MODE";
            }
            ImGui::PushStyleColor(ImGuiCol_Text, stateColor);
            ImGui::Text("%s", stateText);
            ImGui::PopStyleColor();

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::Text("%.0f FPS", app.GetFPS());
            ImGui::PopStyleColor();
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
        ImGui::PopStyleVar(3);
    }

    // ═══════════════════════════════════════════════════════════════
    // VIEWPORT
    // ═══════════════════════════════════════════════════════════════
    m_Focused = ImGui::IsWindowFocused();
    m_Hovered = ImGui::IsWindowHovered();

    if (m_Hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))  ImGui::SetWindowFocus();
    if (m_Hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) ImGui::SetWindowFocus();

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    if (viewportPanelSize.x < 1.0f) viewportPanelSize.x = 1.0f;
    if (viewportPanelSize.y < 1.0f) viewportPanelSize.y = 1.0f;
    m_ViewportSize = viewportPanelSize;

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
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(contentStart,
            ImVec2(contentStart.x + viewportPanelSize.x, contentStart.y + viewportPanelSize.y),
            IM_COL32(15, 15, 15, 255));
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // ─── XYZ Axis Gizmo (below branding) ─────────────────────────
    {
        float gizmoX  = contentStart.x + viewportPanelSize.x - 50.0f;
        float gizmoY  = contentStart.y + 50.0f;
        float axisLen = 24.0f;

        drawList->AddLine(ImVec2(gizmoX, gizmoY), ImVec2(gizmoX + axisLen, gizmoY),
                          IM_COL32(230, 60, 60, 255), 2.0f);
        drawList->AddText(ImVec2(gizmoX + axisLen + 2, gizmoY - 7),
                          IM_COL32(230, 60, 60, 255), "X");

        drawList->AddLine(ImVec2(gizmoX, gizmoY), ImVec2(gizmoX, gizmoY - axisLen),
                          IM_COL32(60, 200, 60, 255), 2.0f);
        drawList->AddText(ImVec2(gizmoX - 6, gizmoY - axisLen - 15),
                          IM_COL32(60, 200, 60, 255), "Y");

        drawList->AddLine(ImVec2(gizmoX, gizmoY),
                          ImVec2(gizmoX - axisLen * 0.6f, gizmoY + axisLen * 0.6f),
                          IM_COL32(60, 100, 230, 255), 2.0f);
        drawList->AddText(ImVec2(gizmoX - axisLen * 0.6f - 12, gizmoY + axisLen * 0.6f - 4),
                          IM_COL32(60, 100, 230, 255), "Z");

        drawList->AddCircleFilled(ImVec2(gizmoX, gizmoY), 3.0f, IM_COL32(200, 200, 200, 200));
    }

    // ─── Tool selector (top-left of viewport) ────────────────────
    {
        ImGui::SetCursorScreenPos(ImVec2(contentStart.x + 8, contentStart.y + 8));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3, 0));

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
            if (ImGui::Button(label, ImVec2(26, 24))) m_GizmoOp = op;
            ImGui::PopStyleColor(3);
        };

        toolButton("W##move",   GizmoOperation::Translate);
        ImGui::SameLine();
        toolButton("E##rotate", GizmoOperation::Rotate);
        ImGui::SameLine();
        toolButton("R##scale",  GizmoOperation::Scale);

        ImGui::PopStyleVar(2);
    }

    // ─── Pyrena Studios Branding (Top-Right of viewport) ─────────
    {
        const char* brandText = "PyEngine";
        float textWidth = ImGui::CalcTextSize(brandText).x;
        float brandX = contentStart.x + viewportPanelSize.x - textWidth - 15.0f;
        float brandY = contentStart.y + 10.0f;
        
        drawList->AddRectFilled(
            ImVec2(brandX - 8, brandY - 3),
            ImVec2(brandX + textWidth + 8, brandY + 16),
            IM_COL32(12, 12, 12, 160), 4.0f);
        
        drawList->AddText(ImVec2(brandX, brandY),
            IM_COL32(80, 150, 255, 180), brandText);
    }

    // ─── Focused Border ──────────────────────────────────────────
    if (m_Focused) {
        drawList->AddRect(contentStart, ImVec2(contentStart.x + viewportPanelSize.x, contentStart.y + viewportPanelSize.y),
                          IM_COL32(66, 150, 250, 120), 0.0f, 0, 1.5f);
    }

    // ─── Camera hint bar (bottom, only when hovered) ─────────────
    if (m_Hovered) {
        float hintY = contentStart.y + viewportPanelSize.y - 22.0f;
        float hintX = contentStart.x + 8.0f;
        drawList->AddRectFilled(
            ImVec2(hintX - 4, hintY - 2),
            ImVec2(hintX + 440, hintY + 14),
            IM_COL32(8, 8, 8, 200), 3.0f);
        drawList->AddText(ImVec2(hintX, hintY), IM_COL32(140, 140, 140, 200),
            "RMB+WASD: Fly  Alt+LMB: Orbit  MMB: Pan  Scroll: Zoom  F: Focus");
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

// ═══════════════════════════════════════════════════════════════
// Game Panel — shown when playing, like Unity's Game tab
// ═══════════════════════════════════════════════════════════════
void SceneViewPanel::OnGamePanelRender() {
    auto& app = PyEngine::Application::Get();
    auto state = app.GetRuntimeState();
    bool isPlaying = (state == PyEngine::Application::RuntimeState::Play ||
                      state == PyEngine::Application::RuntimeState::Pause);

    // Auto-focus Game tab when playing starts
    if (isPlaying && !m_WasPlayingLastFrame) {
        ImGui::SetNextWindowFocus();
    }
    m_WasPlayingLastFrame = isPlaying;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    if (isPlaying && m_OffscreenRenderer) {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImTextureID texID = m_OffscreenRenderer->GetImTextureID();
        ImGui::Image(texID, avail, ImVec2{0, 0}, ImVec2{1, 1});

        // Game viewport overlay
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 contentStart = ImGui::GetCursorScreenPos();
        // Subtle green border to indicate game mode
        ImVec2 winPos = ImGui::GetWindowPos();
        ImVec2 winSize = ImGui::GetWindowSize();
        drawList->AddRect(winPos, ImVec2(winPos.x + winSize.x, winPos.y + winSize.y),
                          IM_COL32(40, 180, 40, 120), 0.0f, 0, 2.0f);
    } else {
        // Not playing — show info message
        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 contentStart = ImGui::GetCursorScreenPos();
        drawList->AddRectFilled(contentStart,
            ImVec2(contentStart.x + avail.x, contentStart.y + avail.y),
            IM_COL32(18, 18, 18, 255));

        const char* msg = "Press Play to start the game";
        ImVec2 textSize = ImGui::CalcTextSize(msg);
        float centerX = contentStart.x + (avail.x - textSize.x) * 0.5f;
        float centerY = contentStart.y + (avail.y - textSize.y) * 0.5f;
        drawList->AddText(ImVec2(centerX, centerY), IM_COL32(80, 80, 80, 255), msg);
    }

    ImGui::End();
    ImGui::PopStyleVar();
}
