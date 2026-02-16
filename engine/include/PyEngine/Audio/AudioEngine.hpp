#pragma once

#include <algorithm>
#include <cmath>
#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// AudioClip — Single audio data buffer
// ═══════════════════════════════════════════════════════════════
struct AudioClip {
    std::string Name;
    std::string FilePath;
    std::vector<float> Samples;
    uint32_t SampleRate = 44100;
    uint32_t Channels = 2;
    float Duration = 0.0f;
    bool IsLoaded = false;

    float GetSample(uint32_t channel, uint32_t sampleIndex) const {
        if (!IsLoaded || Samples.empty())
            return 0.0f;
        uint32_t idx = sampleIndex * Channels + channel;
        if (idx >= Samples.size())
            return 0.0f;
        return Samples[idx];
    }

    void GenerateSineWave(float frequency, float duration, float amplitude = 0.5f) {
        Duration = duration;
        uint32_t totalSamples = static_cast<uint32_t>(SampleRate * duration);
        Samples.resize(totalSamples * Channels);

        for (uint32_t i = 0; i < totalSamples; i++) {
            float t = static_cast<float>(i) / static_cast<float>(SampleRate);
            float value = amplitude * std::sin(2.0f * 3.14159265f * frequency * t);
            for (uint32_t ch = 0; ch < Channels; ch++) {
                Samples[i * Channels + ch] = value;
            }
        }
        IsLoaded = true;
    }

    void GenerateWhiteNoise(float duration, float amplitude = 0.3f) {
        Duration = duration;
        uint32_t totalSamples = static_cast<uint32_t>(SampleRate * duration);
        Samples.resize(totalSamples * Channels);

        for (uint32_t i = 0; i < totalSamples * Channels; i++) {
            Samples[i] = amplitude * (2.0f * (static_cast<float>(rand()) / RAND_MAX) - 1.0f);
        }
        IsLoaded = true;
    }
};

// ═══════════════════════════════════════════════════════════════
// AudioSource — Represents a playing sound
// ═══════════════════════════════════════════════════════════════
struct AudioSource {
    uint32_t ID = 0;
    std::shared_ptr<AudioClip> Clip;

    // Playback
    float Volume = 1.0f;
    float Pitch = 1.0f;
    bool IsPlaying = false;
    bool IsLooping = false;
    bool IsPaused = false;
    bool IsMuted = false;
    float CurrentTime = 0.0f;
    uint32_t CurrentSample = 0;

    // 3D spatial audio
    bool Is3D = false;
    glm::vec3 Position{0.0f};
    float MinDistance = 1.0f;
    float MaxDistance = 50.0f;
    float DopplerLevel = 1.0f;
    float RolloffFactor = 1.0f;

    // Envelope
    float FadeInDuration = 0.0f;
    float FadeOutDuration = 0.0f;
    float FadeTimer = 0.0f;
    bool IsFadingIn = false;
    bool IsFadingOut = false;

    // Filters
    float LowPassCutoff = 22000.0f;
    float HighPassCutoff = 20.0f;
    float ReverbMix = 0.0f;

    // Priority (lower = higher priority)
    int Priority = 128;

    void Play() {
        IsPlaying = true;
        IsPaused = false;
        if (FadeInDuration > 0.0f) {
            IsFadingIn = true;
            FadeTimer = 0.0f;
        }
    }

    void Pause() { IsPaused = true; }
    void Stop() {
        if (FadeOutDuration > 0.0f) {
            IsFadingOut = true;
            FadeTimer = 0.0f;
        } else {
            IsPlaying = false;
            CurrentTime = 0.0f;
            CurrentSample = 0;
        }
    }

    void Resume() { IsPaused = false; }

    float GetEffectiveVolume() const {
        if (IsMuted)
            return 0.0f;
        return Volume;
    }

    float GetNormalizedTime() const {
        if (!Clip || Clip->Duration <= 0.0f)
            return 0.0f;
        return CurrentTime / Clip->Duration;
    }
};

// ═══════════════════════════════════════════════════════════════
// AudioMixerChannel — Submix channel for grouping
// ═══════════════════════════════════════════════════════════════
struct AudioMixerChannel {
    std::string Name = "Master";
    float Volume = 1.0f;
    bool IsMuted = false;
    bool IsSolo = false;

    // Effects chain
    float EQBass = 0.0f;
    float EQMid = 0.0f;
    float EQTreble = 0.0f;
    float Compressor = 0.0f;
    float CompressorThreshold = -20.0f;
    float CompressorRatio = 4.0f;
    float ReverbMix = 0.0f;
    float DelayMix = 0.0f;
    float DelayTime = 0.3f;

    // Metering
    float PeakLevel = 0.0f;
    float RMSLevel = 0.0f;

    // Routing
    int OutputChannel = -1;  // -1 = master output

    std::vector<uint32_t> AssignedSources;
};

// ═══════════════════════════════════════════════════════════════
// AudioListener — Where 3D audio is heard from
// ═══════════════════════════════════════════════════════════════
struct AudioListener {
    glm::vec3 Position{0.0f};
    glm::vec3 Forward{0.0f, 0.0f, -1.0f};
    glm::vec3 Up{0.0f, 1.0f, 0.0f};
    glm::vec3 Velocity{0.0f};
    float MasterVolume = 1.0f;
};

// ═══════════════════════════════════════════════════════════════
// AudioEngine — Main audio system
// ═══════════════════════════════════════════════════════════════
class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    // Initialization
    void Initialize(uint32_t sampleRate = 44100, uint32_t bufferSize = 512);
    void Shutdown();

    // Source management
    uint32_t CreateSource();
    void DestroySource(uint32_t sourceID);
    AudioSource* GetSource(uint32_t sourceID);

    // Playback shortcuts
    uint32_t PlayOneShot(std::shared_ptr<AudioClip> clip, float volume = 1.0f);
    uint32_t PlayOneShotAt(std::shared_ptr<AudioClip> clip, const glm::vec3& position, float volume = 1.0f);
    void StopAll();
    void PauseAll();
    void ResumeAll();

    // Mixer
    AudioMixerChannel& GetMasterChannel() { return m_MasterChannel; }
    AudioMixerChannel& AddMixerChannel(const std::string& name);
    AudioMixerChannel* GetMixerChannel(const std::string& name);

    // Listener
    AudioListener& GetListener() { return m_Listener; }
    void SetListenerPosition(const glm::vec3& pos) { m_Listener.Position = pos; }
    void SetListenerOrientation(const glm::vec3& forward, const glm::vec3& up) {
        m_Listener.Forward = forward;
        m_Listener.Up = up;
    }

    // Update
    void Update(float deltaTime);

    // Stats
    struct Stats {
        uint32_t ActiveSources = 0;
        uint32_t TotalSources = 0;
        float CPUUsage = 0.0f;
        float MasterPeakLevel = 0.0f;
    };
    const Stats& GetStats() const { return m_Stats; }

    // Global settings
    void SetMasterVolume(float volume) { m_Listener.MasterVolume = std::clamp(volume, 0.0f, 1.0f); }
    float GetMasterVolume() const { return m_Listener.MasterVolume; }
    void SetSpeedOfSound(float speed) { m_SpeedOfSound = speed; }
    float GetSpeedOfSound() const { return m_SpeedOfSound; }

private:
    void UpdateSource(AudioSource& source, float dt);
    float CalculateDistanceAttenuation(const AudioSource& source) const;
    glm::vec2 CalculatePanning(const AudioSource& source) const;
    void MixOutput();

private:
    std::vector<AudioSource> m_Sources;
    AudioMixerChannel m_MasterChannel;
    std::vector<AudioMixerChannel> m_MixerChannels;
    AudioListener m_Listener;

    uint32_t m_SampleRate = 44100;
    uint32_t m_BufferSize = 512;
    uint32_t m_NextSourceID = 1;
    float m_SpeedOfSound = 343.0f;
    bool m_Initialized = false;

    Stats m_Stats;
};

}  // namespace PyEngine
