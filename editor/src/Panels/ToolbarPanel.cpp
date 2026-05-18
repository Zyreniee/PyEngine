#include "Panels/ToolbarPanel.hpp"

#include "PyEngine/Core/Application.hpp"

void ToolbarPanel::OnImGuiRender() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 0));

    ImGui::Begin("##Toolbar", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav);

    auto& app = PyEngine::Application::Get();
    auto state = app.GetRuntimeState();

    float btnH = ImGui::GetWindowHeight() - 8.0f;
    if (btnH < 16.0f) btnH = 16.0f;
    float btnW = 60.0f;

    // Center the buttons
    float totalWidth = btnW * 3 + 12.0f;
    float startX = (ImGui::GetWindowContentRegionMax().x * 0.5f) - (totalWidth * 0.5f);
    if (startX < 0) startX = 0;
    ImGui::SetCursorPosX(startX);

    // ── Play / Stop Button ───────────────────────────────
    bool isPlaying = (state == PyEngine::Application::RuntimeState::Play ||
                      state == PyEngine::Application::RuntimeState::Pause);

    if (isPlaying) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.15f, 0.15f, 1.0f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.2f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.4f, 0.15f, 1.0f));
    }

    const char* playLabel = isPlaying ? "Stop" : "Play";
    if (ImGui::Button(playLabel, ImVec2(btnW, btnH))) {
        if (state == PyEngine::Application::RuntimeState::Edit) {
            app.SetRuntimeState(PyEngine::Application::RuntimeState::Play);
        } else {
            app.SetRuntimeState(PyEngine::Application::RuntimeState::Edit);
        }
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();

    // ── Pause Button ─────────────────────────────────────
    bool isPaused = (state == PyEngine::Application::RuntimeState::Pause);

    if (isPaused) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.6f, 0.2f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.7f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.15f, 1.0f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.28f, 0.28f, 0.28f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.35f, 0.35f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
    }

    if (ImGui::Button("Pause", ImVec2(btnW, btnH))) {
        if (state == PyEngine::Application::RuntimeState::Play) {
            app.SetRuntimeState(PyEngine::Application::RuntimeState::Pause);
        } else if (state == PyEngine::Application::RuntimeState::Pause) {
            app.SetRuntimeState(PyEngine::Application::RuntimeState::Play);
        }
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();

    // ── Step Button ──────────────────────────────────────
    bool canStep = (state == PyEngine::Application::RuntimeState::Pause);
    if (!canStep) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.35f);
    }
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.28f, 0.28f, 0.28f, 0.9f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.35f, 0.35f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));

    if (ImGui::Button("Step", ImVec2(btnW, btnH))) {
        // TODO: Step one frame
    }
    ImGui::PopStyleColor(3);
    if (!canStep) {
        ImGui::PopStyleVar();
    }

    // ── Runtime state indicator + FPS ────────────────────
    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 200);
    
    if (isPlaying && !isPaused) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.9f, 0.3f, 1.0f));
        ImGui::Text("PLAYING");
    } else if (isPaused) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.3f, 1.0f));
        ImGui::Text("PAUSED");
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::Text("EDIT");
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    ImGui::Text("| FPS: %.0f", app.GetFPS());
    ImGui::PopStyleColor();

    ImGui::End();
    ImGui::PopStyleVar(2);
}
