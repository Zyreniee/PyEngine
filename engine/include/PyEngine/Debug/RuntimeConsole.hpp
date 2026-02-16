#pragma once

#include <mutex>
#include <string>
#include <vector>

namespace PyEngine {

enum class LogLevel { Trace, Info, Warn, Error, Critical };

struct LogEntry {
    LogLevel Level;
    std::string Message;
    std::string Timestamp;
};

class RuntimeConsole {
public:
    static RuntimeConsole& Get() {
        static RuntimeConsole instance;
        return instance;
    }

    void AddLog(LogLevel level, const std::string& message);
    void Clear();

    // Renders the ImGui window
    void OnImGuiRender(bool* open = nullptr);

private:
    RuntimeConsole() = default;

    std::vector<LogEntry> m_Buffer;
    bool m_ScrollToBottom = true;
    std::mutex m_Mutex;

    // Filters
    bool m_ShowInfo = true;
    bool m_ShowWarn = true;
    bool m_ShowError = true;
};

}  // namespace PyEngine
