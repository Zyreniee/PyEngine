#pragma once

#include <imgui.h>

#include <string>
#include <vector>

class ConsolePanel {
public:
    enum class LogLevel { Info, Warning, Error };

    struct LogMessage {
        std::string Message;
        LogLevel Level;
    };

    ConsolePanel() = default;

    void OnImGuiRender();

    static void Log(const std::string& message, LogLevel level = LogLevel::Info);
    static void Clear();

    static ConsolePanel& Get();

private:
    static std::vector<LogMessage> s_Messages;
    bool m_AutoScroll = true;
    bool m_ShowInfo = true;
    bool m_ShowWarnings = true;
    bool m_ShowErrors = true;
};
