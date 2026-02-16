#pragma once

#include <chrono>
#include <limits>  // Required for std::numeric_limits
#include <string>
#include <unordered_map>
#include <vector>

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// ProfilerScope — Scoped timer for profiling code blocks
// ═══════════════════════════════════════════════════════════════
struct ProfileEntry {
    std::string Name;
    std::string Category;
    float DurationMs = 0.0f;
    float StartTimeMs = 0.0f;
    int Depth = 0;
    uint32_t CallCount = 0;
    float TotalMs = 0.0f;
    float MinMs = std::numeric_limits<float>::max();
    float MaxMs = 0.0f;
    float AvgMs = 0.0f;
};

// ═══════════════════════════════════════════════════════════════
// FrameStats — Per-frame timing data
// ═══════════════════════════════════════════════════════════════
struct FrameStats {
    float TotalFrameTime = 0.0f;
    float UpdateTime = 0.0f;
    float PhysicsTime = 0.0f;
    float RenderTime = 0.0f;
    float ImGuiTime = 0.0f;
    float GpuTime = 0.0f;
    float IdleTime = 0.0f;

    uint32_t DrawCalls = 0;
    uint32_t Triangles = 0;
    uint32_t Vertices = 0;
    uint32_t Batches = 0;

    uint32_t EntityCount = 0;
    uint32_t ActiveParticles = 0;
    uint32_t PhysicsBodies = 0;
    uint32_t AudioSources = 0;

    float MemoryUsageMB = 0.0f;
    float GpuMemoryUsageMB = 0.0f;
};

// ═══════════════════════════════════════════════════════════════
// Profiler — Performance profiling system
// ═══════════════════════════════════════════════════════════════
class Profiler {
public:
    static Profiler& Get() {
        static Profiler instance;
        return instance;
    }

    // ── Scoped profiling ─────────────────────────────────────
    void BeginScope(const std::string& name, const std::string& category = "General");
    void EndScope();

    // ── Frame management ─────────────────────────────────────
    void BeginFrame();
    void EndFrame();

    // ── Stats ────────────────────────────────────────────────
    FrameStats& GetCurrentFrameStats() { return m_CurrentFrame; }
    const FrameStats& GetCurrentFrameStats() const { return m_CurrentFrame; }
    const FrameStats& GetAverageFrameStats() const { return m_AverageFrame; }

    // History
    const std::vector<float>& GetFrameTimeHistory() const { return m_FrameTimeHistory; }
    const std::vector<ProfileEntry>& GetCurrentEntries() const { return m_CurrentEntries; }
    const std::unordered_map<std::string, ProfileEntry>& GetAccumulatedStats() const { return m_AccumulatedStats; }

    // FPS
    float GetFPS() const { return m_FPS; }
    float GetAverageFPS() const { return m_AverageFPS; }
    float GetMinFPS() const { return m_MinFPS; }
    float GetMaxFPS() const { return m_MaxFPS; }

    // Memory
    void SetMemoryUsage(float mb) { m_CurrentFrame.MemoryUsageMB = mb; }
    void SetGpuMemoryUsage(float mb) { m_CurrentFrame.GpuMemoryUsageMB = mb; }

    // Control
    void SetEnabled(bool enabled) { m_Enabled = enabled; }
    bool IsEnabled() const { return m_Enabled; }
    void Reset();

    // History size
    void SetHistorySize(uint32_t size) { m_HistorySize = size; }
    uint32_t GetHistorySize() const { return m_HistorySize; }

    // Report
    std::string GenerateReport() const;

private:
    Profiler() {
        m_FrameTimeHistory.reserve(m_HistorySize);
        m_ScopeStack.reserve(16);
    }

private:
    bool m_Enabled = true;

    // Current frame
    FrameStats m_CurrentFrame;
    FrameStats m_AverageFrame;
    std::vector<ProfileEntry> m_CurrentEntries;
    std::unordered_map<std::string, ProfileEntry> m_AccumulatedStats;

    // Scope stack
    struct ScopeData {
        std::string Name;
        std::string Category;
        std::chrono::high_resolution_clock::time_point StartTime;
        int Depth;
    };
    std::vector<ScopeData> m_ScopeStack;
    int m_CurrentDepth = 0;

    // Frame timing
    std::chrono::high_resolution_clock::time_point m_FrameStart;
    std::chrono::high_resolution_clock::time_point m_FrameEnd;

    // History
    std::vector<float> m_FrameTimeHistory;
    uint32_t m_HistorySize = 300;
    uint32_t m_TotalFrames = 0;

    // FPS
    float m_FPS = 0.0f;
    float m_AverageFPS = 0.0f;
    float m_MinFPS = 999.0f;
    float m_MaxFPS = 0.0f;
    float m_FPSAccumulator = 0.0f;
    uint32_t m_FPSFrameCount = 0;
};

// ═══════════════════════════════════════════════════════════════
// ScopedProfile — RAII profiler scope
// ═══════════════════════════════════════════════════════════════
class ScopedProfile {
public:
    ScopedProfile(const std::string& name, const std::string& category = "General") {
        Profiler::Get().BeginScope(name, category);
    }
    ~ScopedProfile() { Profiler::Get().EndScope(); }
};

#define PYENGINE_PROFILE_SCOPE(name) ::PyEngine::ScopedProfile _profile_##__LINE__(name)
#define PYENGINE_PROFILE_FUNCTION() ::PyEngine::ScopedProfile _profile_##__LINE__(__FUNCTION__)
#define PYENGINE_PROFILE_SCOPE_CAT(name, category) ::PyEngine::ScopedProfile _profile_##__LINE__(name, category)

}  // namespace PyEngine
