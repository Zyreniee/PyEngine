#pragma once

#include <chrono>

namespace PyEngine {

class Timer {
public:
  Timer();

  void Reset();
  float GetDeltaTime() const;   // In seconds
  float GetElapsedTime() const; // In seconds

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_LastFrameTime;
};

} // namespace PyEngine
