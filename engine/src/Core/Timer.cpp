#include "PyEngine/Core/Timer.hpp"

namespace PyEngine {

Timer::Timer()
    : m_StartTime(std::chrono::high_resolution_clock::now()),
      m_LastFrameTime(m_StartTime) {}

void Timer::Reset() {
  auto now = std::chrono::high_resolution_clock::now();
  m_LastFrameTime = now;
}

float Timer::GetDeltaTime() const {
  auto now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> delta = now - m_LastFrameTime;
  return delta.count();
}

float Timer::GetElapsedTime() const {
  auto now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> elapsed = now - m_StartTime;
  return elapsed.count();
}

} // namespace PyEngine
