#include "PyEngine/Audio/AudioEngine.hpp"

#include <chrono>

namespace PyEngine {

AudioEngine::AudioEngine() {
    m_MasterChannel.Name = "Master";
    m_Sources.reserve(256);
}

AudioEngine::~AudioEngine() {
    Shutdown();
}

void AudioEngine::Initialize(uint32_t sampleRate, uint32_t bufferSize) {
    m_SampleRate = sampleRate;
    m_BufferSize = bufferSize;
    m_Initialized = true;

    // Add default mixer channels
    AddMixerChannel("Music");
    AddMixerChannel("SFX");
    AddMixerChannel("Ambient");
    AddMixerChannel("UI");
    AddMixerChannel("Voice");
}

void AudioEngine::Shutdown() {
    StopAll();
    m_Sources.clear();
    m_MixerChannels.clear();
    m_Initialized = false;
}

uint32_t AudioEngine::CreateSource() {
    AudioSource source;
    source.ID = m_NextSourceID++;
    m_Sources.push_back(source);
    return source.ID;
}

void AudioEngine::DestroySource(uint32_t sourceID) {
    m_Sources.erase(std::remove_if(m_Sources.begin(), m_Sources.end(),
                                   [sourceID](const AudioSource& s) { return s.ID == sourceID; }),
                    m_Sources.end());
}

AudioSource* AudioEngine::GetSource(uint32_t sourceID) {
    for (auto& source : m_Sources) {
        if (source.ID == sourceID)
            return &source;
    }
    return nullptr;
}

uint32_t AudioEngine::PlayOneShot(std::shared_ptr<AudioClip> clip, float volume) {
    uint32_t id = CreateSource();
    if (auto* source = GetSource(id)) {
        source->Clip = clip;
        source->Volume = volume;
        source->IsLooping = false;
        source->Play();
    }
    return id;
}

uint32_t AudioEngine::PlayOneShotAt(std::shared_ptr<AudioClip> clip, const glm::vec3& position, float volume) {
    uint32_t id = CreateSource();
    if (auto* source = GetSource(id)) {
        source->Clip = clip;
        source->Volume = volume;
        source->IsLooping = false;
        source->Is3D = true;
        source->Position = position;
        source->Play();
    }
    return id;
}

void AudioEngine::StopAll() {
    for (auto& source : m_Sources) {
        source.Stop();
    }
}

void AudioEngine::PauseAll() {
    for (auto& source : m_Sources) {
        if (source.IsPlaying)
            source.Pause();
    }
}

void AudioEngine::ResumeAll() {
    for (auto& source : m_Sources) {
        if (source.IsPaused)
            source.Resume();
    }
}

AudioMixerChannel& AudioEngine::AddMixerChannel(const std::string& name) {
    AudioMixerChannel channel;
    channel.Name = name;
    m_MixerChannels.push_back(channel);
    return m_MixerChannels.back();
}

AudioMixerChannel* AudioEngine::GetMixerChannel(const std::string& name) {
    for (auto& channel : m_MixerChannels) {
        if (channel.Name == name)
            return &channel;
    }
    return nullptr;
}

void AudioEngine::Update(float deltaTime) {
    if (!m_Initialized)
        return;

    auto start = std::chrono::high_resolution_clock::now();

    m_Stats.ActiveSources = 0;
    m_Stats.TotalSources = static_cast<uint32_t>(m_Sources.size());

    // Update each source
    for (auto& source : m_Sources) {
        if (!source.IsPlaying || source.IsPaused)
            continue;
        UpdateSource(source, deltaTime);
        m_Stats.ActiveSources++;
    }

    // Remove finished one-shot sources
    m_Sources.erase(std::remove_if(m_Sources.begin(), m_Sources.end(),
                                   [](const AudioSource& s) { return !s.IsPlaying && !s.IsPaused && !s.IsLooping; }),
                    m_Sources.end());

    // Update mixer levels
    MixOutput();

    auto end = std::chrono::high_resolution_clock::now();
    m_Stats.CPUUsage = std::chrono::duration<float, std::milli>(end - start).count();
}

void AudioEngine::UpdateSource(AudioSource& source, float dt) {
    if (!source.Clip || !source.Clip->IsLoaded)
        return;

    // Update fade
    if (source.IsFadingIn) {
        source.FadeTimer += dt;
        if (source.FadeTimer >= source.FadeInDuration) {
            source.IsFadingIn = false;
        }
    }
    if (source.IsFadingOut) {
        source.FadeTimer += dt;
        if (source.FadeTimer >= source.FadeOutDuration) {
            source.IsFadingOut = false;
            source.IsPlaying = false;
            source.CurrentTime = 0.0f;
            source.CurrentSample = 0;
            return;
        }
    }

    // Advance playback position
    float effectivePitch = std::clamp(source.Pitch, 0.1f, 4.0f);
    source.CurrentTime += dt * effectivePitch;
    source.CurrentSample = static_cast<uint32_t>(source.CurrentTime * source.Clip->SampleRate);

    // Check end of clip
    if (source.CurrentTime >= source.Clip->Duration) {
        if (source.IsLooping) {
            source.CurrentTime = std::fmod(source.CurrentTime, source.Clip->Duration);
            source.CurrentSample = static_cast<uint32_t>(source.CurrentTime * source.Clip->SampleRate);
        } else {
            source.IsPlaying = false;
            source.CurrentTime = 0.0f;
            source.CurrentSample = 0;
        }
    }
}

float AudioEngine::CalculateDistanceAttenuation(const AudioSource& source) const {
    if (!source.Is3D)
        return 1.0f;

    float distance = glm::distance(source.Position, m_Listener.Position);

    if (distance <= source.MinDistance)
        return 1.0f;
    if (distance >= source.MaxDistance)
        return 0.0f;

    // Inverse distance clamped rolloff
    float attenuation =
        source.MinDistance / (source.MinDistance + source.RolloffFactor * (distance - source.MinDistance));

    return std::clamp(attenuation, 0.0f, 1.0f);
}

glm::vec2 AudioEngine::CalculatePanning(const AudioSource& source) const {
    if (!source.Is3D)
        return glm::vec2(1.0f);

    // Calculate direction from listener to source
    glm::vec3 toSource = glm::normalize(source.Position - m_Listener.Position);
    glm::vec3 right = glm::normalize(glm::cross(m_Listener.Forward, m_Listener.Up));

    float pan = glm::dot(toSource, right);

    // Constant power panning
    float angle = (pan + 1.0f) * 0.5f * 3.14159265f * 0.5f;
    return glm::vec2(std::cos(angle), std::sin(angle));
}

void AudioEngine::MixOutput() {
    // Calculate master peak level
    float maxLevel = 0.0f;

    for (const auto& source : m_Sources) {
        if (!source.IsPlaying || source.IsPaused || !source.Clip)
            continue;

        float sourceVolume = source.GetEffectiveVolume();
        float attenuation = CalculateDistanceAttenuation(source);
        float level = sourceVolume * attenuation * m_Listener.MasterVolume;

        // Apply fade
        if (source.IsFadingIn && source.FadeInDuration > 0.0f) {
            level *= std::clamp(source.FadeTimer / source.FadeInDuration, 0.0f, 1.0f);
        }
        if (source.IsFadingOut && source.FadeOutDuration > 0.0f) {
            level *= 1.0f - std::clamp(source.FadeTimer / source.FadeOutDuration, 0.0f, 1.0f);
        }

        // Read current sample value
        if (source.CurrentSample < source.Clip->Samples.size() / source.Clip->Channels) {
            float sample = std::abs(source.Clip->GetSample(0, source.CurrentSample));
            maxLevel = std::max(maxLevel, sample * level);
        }
    }

    // Smooth peak metering
    m_Stats.MasterPeakLevel = m_Stats.MasterPeakLevel * 0.95f + maxLevel * 0.05f;

    // Update mixer channel meters
    for (auto& channel : m_MixerChannels) {
        float channelPeak = 0.0f;
        for (uint32_t srcID : channel.AssignedSources) {
            if (auto* src = GetSource(srcID)) {
                float srcLevel = src->GetEffectiveVolume() * CalculateDistanceAttenuation(*src);
                channelPeak = std::max(channelPeak, srcLevel);
            }
        }
        channel.PeakLevel = channel.PeakLevel * 0.95f + channelPeak * 0.05f;
        channel.RMSLevel = channel.RMSLevel * 0.98f + channelPeak * 0.02f;
    }

    m_MasterChannel.PeakLevel = m_Stats.MasterPeakLevel;
}

}  // namespace PyEngine
