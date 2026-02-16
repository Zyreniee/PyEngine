#include "PyEngine/Debug/Profiler.hpp"

#include <algorithm>
#include <iomanip>
#include <numeric>

namespace PyEngine {

void Profiler::BeginScope(const std::string& name, const std::string& category) {
    if (!m_Enabled)
        return;

    ScopeData scope;
    scope.Name = name;
    scope.Category = category;
    scope.StartTime = std::chrono::high_resolution_clock::now();
    scope.Depth = m_CurrentDepth;

    m_ScopeStack.push_back(scope);
    m_CurrentDepth++;
}

void Profiler::EndScope() {
    if (!m_Enabled || m_ScopeStack.empty())
        return;

    auto endTime = std::chrono::high_resolution_clock::now();
    auto& scope = m_ScopeStack.back();

    float durationMs = std::chrono::duration<float, std::milli>(endTime - scope.StartTime).count();

    // Record entry
    ProfileEntry entry;
    entry.Name = scope.Name;
    entry.Category = scope.Category;
    entry.DurationMs = durationMs;
    entry.Depth = scope.Depth;
    entry.CallCount = 1;
    m_CurrentEntries.push_back(entry);

    // Accumulate stats
    auto& accumulated = m_AccumulatedStats[scope.Name];
    accumulated.Name = scope.Name;
    accumulated.Category = scope.Category;
    accumulated.CallCount++;
    accumulated.TotalMs += durationMs;
    accumulated.MinMs = std::min(accumulated.MinMs, durationMs);
    accumulated.MaxMs = std::max(accumulated.MaxMs, durationMs);
    accumulated.AvgMs = accumulated.TotalMs / static_cast<float>(accumulated.CallCount);
    accumulated.DurationMs = durationMs;

    m_ScopeStack.pop_back();
    m_CurrentDepth--;
}

void Profiler::BeginFrame() {
    if (!m_Enabled)
        return;

    m_FrameStart = std::chrono::high_resolution_clock::now();
    m_CurrentEntries.clear();
    m_CurrentFrame = FrameStats();
    m_CurrentDepth = 0;
    m_ScopeStack.clear();
}

void Profiler::EndFrame() {
    if (!m_Enabled)
        return;

    m_FrameEnd = std::chrono::high_resolution_clock::now();
    float frameTimeMs = std::chrono::duration<float, std::milli>(m_FrameEnd - m_FrameStart).count();

    m_CurrentFrame.TotalFrameTime = frameTimeMs;

    // Update history
    m_FrameTimeHistory.push_back(frameTimeMs);
    if (m_FrameTimeHistory.size() > m_HistorySize) {
        m_FrameTimeHistory.erase(m_FrameTimeHistory.begin());
    }

    // Update FPS
    m_FPS = (frameTimeMs > 0.0f) ? (1000.0f / frameTimeMs) : 0.0f;
    m_MinFPS = std::min(m_MinFPS, m_FPS);
    m_MaxFPS = std::max(m_MaxFPS, m_FPS);

    m_FPSAccumulator += m_FPS;
    m_FPSFrameCount++;
    m_AverageFPS = m_FPSAccumulator / static_cast<float>(m_FPSFrameCount);

    // Rolling average reset every 5 seconds
    if (m_FPSFrameCount > 300) {
        m_FPSAccumulator = m_AverageFPS;
        m_FPSFrameCount = 1;
    }

    // Calculate average frame stats
    if (!m_FrameTimeHistory.empty()) {
        float totalTime = std::accumulate(m_FrameTimeHistory.begin(), m_FrameTimeHistory.end(), 0.0f);
        m_AverageFrame.TotalFrameTime = totalTime / static_cast<float>(m_FrameTimeHistory.size());
    }

    m_TotalFrames++;
}

void Profiler::Reset() {
    m_CurrentEntries.clear();
    m_AccumulatedStats.clear();
    m_FrameTimeHistory.clear();
    m_ScopeStack.clear();
    m_CurrentFrame = {};
    m_AverageFrame = {};
    m_TotalFrames = 0;
    m_FPS = 0.0f;
    m_AverageFPS = 0.0f;
    m_MinFPS = 999.0f;
    m_MaxFPS = 0.0f;
    m_FPSAccumulator = 0.0f;
    m_FPSFrameCount = 0;
    m_CurrentDepth = 0;
}

std::string Profiler::GenerateReport() const {
    std::ostringstream report;
    report << std::fixed << std::setprecision(2);

    report << "═══════════════════════════════════════════\n";
    report << "  PyEngine Performance Report\n";
    report << "═══════════════════════════════════════════\n\n";

    report << "Frame Stats:\n";
    report << "  FPS: " << m_FPS << " (Avg: " << m_AverageFPS << " Min: " << m_MinFPS << " Max: " << m_MaxFPS << ")\n";
    report << "  Frame Time: " << m_CurrentFrame.TotalFrameTime << " ms\n";
    report << "  Total Frames: " << m_TotalFrames << "\n\n";

    report << "  Draw Calls: " << m_CurrentFrame.DrawCalls << "\n";
    report << "  Triangles: " << m_CurrentFrame.Triangles << "\n";
    report << "  Vertices: " << m_CurrentFrame.Vertices << "\n";
    report << "  Batches: " << m_CurrentFrame.Batches << "\n\n";

    report << "  Entities: " << m_CurrentFrame.EntityCount << "\n";
    report << "  Particles: " << m_CurrentFrame.ActiveParticles << "\n";
    report << "  Physics Bodies: " << m_CurrentFrame.PhysicsBodies << "\n";
    report << "  Audio Sources: " << m_CurrentFrame.AudioSources << "\n\n";

    report << "  Memory: " << m_CurrentFrame.MemoryUsageMB << " MB\n";
    report << "  GPU Memory: " << m_CurrentFrame.GpuMemoryUsageMB << " MB\n\n";

    if (!m_AccumulatedStats.empty()) {
        report << "Profiled Scopes:\n";
        report << "  " << std::setw(30) << std::left << "Name" << std::setw(10) << "Calls" << std::setw(10) << "Avg(ms)"
               << std::setw(10) << "Min(ms)" << std::setw(10) << "Max(ms)" << std::setw(12) << "Total(ms)" << "\n";
        report << "  " << std::string(82, '-') << "\n";

        for (const auto& [name, entry] : m_AccumulatedStats) {
            report << "  " << std::setw(30) << std::left << name << std::setw(10) << entry.CallCount << std::setw(10)
                   << entry.AvgMs << std::setw(10) << entry.MinMs << std::setw(10) << entry.MaxMs << std::setw(12)
                   << entry.TotalMs << "\n";
        }
    }

    return report.str();
}

}  // namespace PyEngine
