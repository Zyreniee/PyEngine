#include "PyEngine/Debug/RuntimeConsole.hpp"

#include <imgui.h>

#include <chrono>
#include <cstring>
#include <iomanip>
#include <sstream>

namespace PyEngine {

void RuntimeConsole::AddLog(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);

    std::stringstream ss;
    ss << std::put_time(&tm, "[%H:%M:%S]");

    m_Buffer.push_back({level, message, ss.str()});

    // Limit buffer size
    if (m_Buffer.size() > 2000) {
        m_Buffer.erase(m_Buffer.begin());
    }

    m_ScrollToBottom = true;
}

void RuntimeConsole::Clear() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Buffer.clear();
}

void RuntimeConsole::OnImGuiRender(bool* open) {
    if (!ImGui::Begin("Console", open)) {
        ImGui::End();
        return;
    }

    // Toolbar
    if (ImGui::Button("Clear"))
        Clear();
    ImGui::SameLine();
    ImGui::Checkbox("Info", &m_ShowInfo);
    ImGui::SameLine();
    ImGui::Checkbox("Warn", &m_ShowWarn);
    ImGui::SameLine();
    ImGui::Checkbox("Error", &m_ShowError);
    ImGui::Separator();

    // Scroll region
    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false,
                      ImGuiWindowFlags_HorizontalScrollbar);

    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        for (const auto& log : m_Buffer) {
            if (log.Level == LogLevel::Info && !m_ShowInfo)
                continue;
            if (log.Level == LogLevel::Warn && !m_ShowWarn)
                continue;
            if (log.Level == LogLevel::Error && !m_ShowError)
                continue;
            if (log.Level == LogLevel::Critical && !m_ShowError)
                continue;

            ImVec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
            if (log.Level == LogLevel::Warn)
                color = {1.0f, 1.0f, 0.0f, 1.0f};
            else if (log.Level == LogLevel::Error)
                color = {1.0f, 0.0f, 0.0f, 1.0f};
            else if (log.Level == LogLevel::Critical)
                color = {1.0f, 0.0f, 0.0f, 1.0f};
            else if (log.Level == LogLevel::Trace)
                color = {0.5f, 0.5f, 0.5f, 1.0f};

            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s ", log.Timestamp.c_str());
            ImGui::SameLine();
            ImGui::TextColored(color, "%s", log.Message.c_str());
        }
    }

    if (m_ScrollToBottom && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
    m_ScrollToBottom = false;

    ImGui::EndChild();

    // Command Input
    static char input[256] = "";
    bool reclaim_focus = false;
    if (ImGui::InputText("Input", input, IM_ARRAYSIZE(input), ImGuiInputTextFlags_EnterReturnsTrue)) {
        std::string cmd = input;
        AddLog(LogLevel::Info, "> " + cmd);

        // Execute command (placeholder)
        if (cmd == "clear")
            Clear();
        else if (cmd == "help")
            AddLog(LogLevel::Info, "Commands: clear, help");
        else
            AddLog(LogLevel::Warn, "Unknown command");

        strcpy(input, "");
        reclaim_focus = true;
    }

    if (reclaim_focus)
        ImGui::SetKeyboardFocusHere(-1);

    ImGui::End();
}

}  // namespace PyEngine
