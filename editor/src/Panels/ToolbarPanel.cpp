#include "Panels/ToolbarPanel.hpp"

#include "PyEngine/Core/Application.hpp"

void ToolbarPanel::OnImGuiRender() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

    auto& colors = ImGui::GetStyle().Colors;
    auto hoveredColor = colors[ImGuiCol_ButtonHovered];
    auto activeColor = colors[ImGuiCol_ButtonActive];
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(hoveredColor.x, hoveredColor.y, hoveredColor.z, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(activeColor.x, activeColor.y, activeColor.z, 0.5f));

    ImGui::Begin("##Toolbar", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    auto& app = PyEngine::Application::Get();
    auto state = app.GetRuntimeState();

    float size = ImGui::GetWindowHeight() - 4.0f;
    ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 1.5f));

    // Play button
    bool isPlaying = (state == PyEngine::Application::RuntimeState::Play);
    if (isPlaying) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 0.7f));
    }

    if (ImGui::Button(isPlaying ? "\xef\x81\x8c" : "\xef\x81\x8b", ImVec2(size, size))) {  // pause : play icons
        if (state == PyEngine::Application::RuntimeState::Edit) {
            app.SetRuntimeState(PyEngine::Application::RuntimeState::Play);
        } else {
            app.SetRuntimeState(PyEngine::Application::RuntimeState::Edit);
        }
    }

    if (isPlaying) {
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    // Pause button
    bool isPaused = (state == PyEngine::Application::RuntimeState::Pause);
    if (isPaused) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.6f, 0.2f, 0.7f));
    }

    if (ImGui::Button("\xef\x81\x8c", ImVec2(size, size))) {  // pause icon
        if (state == PyEngine::Application::RuntimeState::Play) {
            app.SetRuntimeState(PyEngine::Application::RuntimeState::Pause);
        } else if (state == PyEngine::Application::RuntimeState::Pause) {
            app.SetRuntimeState(PyEngine::Application::RuntimeState::Play);
        }
    }

    if (isPaused) {
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    // Step button (only available when paused)
    bool canStep = (state == PyEngine::Application::RuntimeState::Pause);
    if (!canStep) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f);
    }
    if (ImGui::Button("\xef\x81\x91", ImVec2(size, size))) {  // step-forward icon
        // TODO: Step one frame
    }
    if (!canStep) {
        ImGui::PopStyleVar();
    }

    // FPS display on the right
    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 120);
    ImGui::Text("FPS: %.0f (%.1fms)", app.GetFPS(), app.GetFrameTime());

    ImGui::End();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}
