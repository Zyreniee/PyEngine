#include "PyEngine/Core/Log.hpp"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace PyEngine {

std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

void Log::Init() {
  // Create sinks
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_pattern("%^[%T] %n: %v%$");

  auto file_sink =
      std::make_shared<spdlog::sinks::basic_file_sink_mt>("PyEngine.log", true);
  file_sink->set_pattern("[%T] [%l] %n: %v");

  // Create loggers
  std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};

  s_CoreLogger =
      std::make_shared<spdlog::logger>("PYENGINE", sinks.begin(), sinks.end());
  s_CoreLogger->set_level(spdlog::level::trace);
  spdlog::register_logger(s_CoreLogger);

  s_ClientLogger =
      std::make_shared<spdlog::logger>("APP", sinks.begin(), sinks.end());
  s_ClientLogger->set_level(spdlog::level::trace);
  spdlog::register_logger(s_ClientLogger);

  PYENGINE_CORE_INFO("PyEngine logging initialized");
}

void Log::Shutdown() {
  PYENGINE_CORE_INFO("PyEngine logging shutdown");
  spdlog::shutdown();
}

} // namespace PyEngine
