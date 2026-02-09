#pragma once

#include <spdlog/spdlog.h>

#include <memory>

namespace PyEngine {

class Log {
public:
    static void Init();
    static void Shutdown();

    static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
    static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

private:
    static std::shared_ptr<spdlog::logger> s_CoreLogger;
    static std::shared_ptr<spdlog::logger> s_ClientLogger;
};

}  // namespace PyEngine

// Core logging macros
#define PYENGINE_CORE_TRACE(...) ::PyEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define PYENGINE_CORE_INFO(...) ::PyEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define PYENGINE_CORE_WARN(...) ::PyEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define PYENGINE_CORE_ERROR(...) ::PyEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define PYENGINE_CORE_CRITICAL(...) ::PyEngine::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client logging macros
#define PYENGINE_TRACE(...) ::PyEngine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define PYENGINE_INFO(...) ::PyEngine::Log::GetClientLogger()->info(__VA_ARGS__)
#define PYENGINE_WARN(...) ::PyEngine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define PYENGINE_ERROR(...) ::PyEngine::Log::GetClientLogger()->error(__VA_ARGS__)
#define PYENGINE_CRITICAL(...) ::PyEngine::Log::GetClientLogger()->critical(__VA_ARGS__)
