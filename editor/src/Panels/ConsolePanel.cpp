#include "Panels/ConsolePanel.hpp"

std::vector<ConsolePanel::LogMessage> ConsolePanel::s_Messages;

ConsolePanel& ConsolePanel::Get() {
    static ConsolePanel instance;
    return instance;
}

void ConsolePanel::Log(const std::string& message, LogLevel level) {
    s_Messages.push_back({message, level});

    // Cap at 1000 messages
    if (s_Messages.size() > 1000) {
        s_Messages.erase(s_Messages.begin());
    }
}

void ConsolePanel::Clear() {
    s_Messages.clear();
}

void ConsolePanel::OnImGuiRender() {
    ImGui::Begin("\xef\x84\xa0  Console");  // Icon: terminal

    // Toolbar
    if (ImGui::Button("Clear")) {
        Clear();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_AutoScroll);
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.8f, 1.0f, 1.0f));
    ImGui::Checkbox("Info", &m_ShowInfo);
    ImGui::PopStyleColor();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
    ImGui::Checkbox("Warnings", &m_ShowWarnings);
    ImGui::PopStyleColor();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
    ImGui::Checkbox("Errors", &m_ShowErrors);
    ImGui::PopStyleColor();

    ImGui::Separator();

    // Log area
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto& msg : s_Messages) {
        bool show = false;
        ImVec4 color;

        switch (msg.Level) {
            case LogLevel::Info:
                show = m_ShowInfo;
                color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
                break;
            case LogLevel::Warning:
                show = m_ShowWarnings;
                color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
                break;
            case LogLevel::Error:
                show = m_ShowErrors;
                color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
                break;
        }

        if (show) {
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextWrapped("%s", msg.Message.c_str());
            ImGui::PopStyleColor();
        }
    }

    if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::End();
}
