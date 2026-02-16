#pragma once

#include <algorithm>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// Keyframe — Single animation sample
// ═══════════════════════════════════════════════════════════════
template <typename T>
struct Keyframe {
    float Time = 0.0f;
    T Value{};

    Keyframe() = default;
    Keyframe(float time, const T& value) : Time(time), Value(value) {}

    bool operator<(const Keyframe& other) const { return Time < other.Time; }
};

using Vec3Keyframe = Keyframe<glm::vec3>;
using QuatKeyframe = Keyframe<glm::quat>;
using FloatKeyframe = Keyframe<float>;

// ═══════════════════════════════════════════════════════════════
// InterpolationType
// ═══════════════════════════════════════════════════════════════
enum class InterpolationType { Linear, Step, CubicSpline, Hermite, Bezier };

// ═══════════════════════════════════════════════════════════════
// AnimationChannel — Animates a single property
// ═══════════════════════════════════════════════════════════════
enum class AnimationProperty {
    PositionX,
    PositionY,
    PositionZ,
    RotationX,
    RotationY,
    RotationZ,
    RotationW,
    ScaleX,
    ScaleY,
    ScaleZ,
    Position,  // vec3
    Rotation,  // quat
    Scale,     // vec3
    Custom
};

struct AnimationChannel {
    std::string TargetPath;
    AnimationProperty Property = AnimationProperty::Position;
    InterpolationType Interpolation = InterpolationType::Linear;

    std::vector<Vec3Keyframe> Vec3Keys;
    std::vector<QuatKeyframe> QuatKeys;
    std::vector<FloatKeyframe> FloatKeys;

    float GetDuration() const {
        float maxTime = 0.0f;
        if (!Vec3Keys.empty())
            maxTime = std::max(maxTime, Vec3Keys.back().Time);
        if (!QuatKeys.empty())
            maxTime = std::max(maxTime, QuatKeys.back().Time);
        if (!FloatKeys.empty())
            maxTime = std::max(maxTime, FloatKeys.back().Time);
        return maxTime;
    }

    void SortKeys() {
        std::sort(Vec3Keys.begin(), Vec3Keys.end());
        std::sort(QuatKeys.begin(), QuatKeys.end());
        std::sort(FloatKeys.begin(), FloatKeys.end());
    }

    glm::vec3 SampleVec3(float time) const {
        if (Vec3Keys.empty())
            return glm::vec3(0.0f);
        if (Vec3Keys.size() == 1)
            return Vec3Keys[0].Value;

        // Clamp to range
        if (time <= Vec3Keys.front().Time)
            return Vec3Keys.front().Value;
        if (time >= Vec3Keys.back().Time)
            return Vec3Keys.back().Value;

        // Find surrounding keyframes
        size_t i = 0;
        for (; i < Vec3Keys.size() - 1; i++) {
            if (time < Vec3Keys[i + 1].Time)
                break;
        }

        float t = (time - Vec3Keys[i].Time) / (Vec3Keys[i + 1].Time - Vec3Keys[i].Time);

        switch (Interpolation) {
            case InterpolationType::Step:
                return Vec3Keys[i].Value;
            case InterpolationType::CubicSpline: {
                float t2 = t * t;
                float t3 = t2 * t;
                float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
                float h10 = t3 - 2.0f * t2 + t;
                float h01 = -2.0f * t3 + 3.0f * t2;
                float h11 = t3 - t2;
                glm::vec3 tangentA =
                    (i + 1 < Vec3Keys.size()) ? (Vec3Keys[i + 1].Value - Vec3Keys[i].Value) : glm::vec3(0.0f);
                glm::vec3 tangentB = tangentA;
                return h00 * Vec3Keys[i].Value + h10 * tangentA + h01 * Vec3Keys[i + 1].Value + h11 * tangentB;
            }
            default:  // Linear
                return glm::mix(Vec3Keys[i].Value, Vec3Keys[i + 1].Value, t);
        }
    }

    glm::quat SampleQuat(float time) const {
        if (QuatKeys.empty())
            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        if (QuatKeys.size() == 1)
            return QuatKeys[0].Value;

        if (time <= QuatKeys.front().Time)
            return QuatKeys.front().Value;
        if (time >= QuatKeys.back().Time)
            return QuatKeys.back().Value;

        size_t i = 0;
        for (; i < QuatKeys.size() - 1; i++) {
            if (time < QuatKeys[i + 1].Time)
                break;
        }

        float t = (time - QuatKeys[i].Time) / (QuatKeys[i + 1].Time - QuatKeys[i].Time);

        if (Interpolation == InterpolationType::Step) {
            return QuatKeys[i].Value;
        }

        return glm::slerp(QuatKeys[i].Value, QuatKeys[i + 1].Value, t);
    }

    float SampleFloat(float time) const {
        if (FloatKeys.empty())
            return 0.0f;
        if (FloatKeys.size() == 1)
            return FloatKeys[0].Value;

        if (time <= FloatKeys.front().Time)
            return FloatKeys.front().Value;
        if (time >= FloatKeys.back().Time)
            return FloatKeys.back().Value;

        size_t i = 0;
        for (; i < FloatKeys.size() - 1; i++) {
            if (time < FloatKeys[i + 1].Time)
                break;
        }

        float t = (time - FloatKeys[i].Time) / (FloatKeys[i + 1].Time - FloatKeys[i].Time);

        if (Interpolation == InterpolationType::Step)
            return FloatKeys[i].Value;
        return FloatKeys[i].Value + (FloatKeys[i + 1].Value - FloatKeys[i].Value) * t;
    }
};

// ═══════════════════════════════════════════════════════════════
// AnimationClip — Collection of channels forming one animation
// ═══════════════════════════════════════════════════════════════
struct AnimationClip {
    std::string Name = "Animation";
    float Duration = 0.0f;
    float TicksPerSecond = 30.0f;
    bool IsLooping = false;

    std::vector<AnimationChannel> Channels;

    void RecalculateDuration() {
        Duration = 0.0f;
        for (auto& channel : Channels) {
            Duration = std::max(Duration, channel.GetDuration());
        }
    }

    AnimationChannel* FindChannel(const std::string& path, AnimationProperty prop) {
        for (auto& ch : Channels) {
            if (ch.TargetPath == path && ch.Property == prop)
                return &ch;
        }
        return nullptr;
    }
};

// ═══════════════════════════════════════════════════════════════
// AnimationState — Current playback state of one clip
// ═══════════════════════════════════════════════════════════════
struct AnimationState {
    std::string Name;
    std::shared_ptr<AnimationClip> Clip;
    float CurrentTime = 0.0f;
    float Speed = 1.0f;
    float Weight = 1.0f;
    bool IsPlaying = false;
    bool IsPaused = false;

    void Play() {
        IsPlaying = true;
        IsPaused = false;
    }
    void Pause() { IsPaused = true; }
    void Stop() {
        IsPlaying = false;
        IsPaused = false;
        CurrentTime = 0.0f;
    }
    void Reset() { CurrentTime = 0.0f; }

    float GetNormalizedTime() const {
        if (!Clip || Clip->Duration <= 0.0f)
            return 0.0f;
        return CurrentTime / Clip->Duration;
    }

    bool IsFinished() const {
        if (!Clip)
            return true;
        return !Clip->IsLooping && CurrentTime >= Clip->Duration;
    }
};

// ═══════════════════════════════════════════════════════════════
// AnimationTransition — Blend between states
// ═══════════════════════════════════════════════════════════════
struct AnimationTransition {
    std::string FromState;
    std::string ToState;
    float Duration = 0.3f;
    float CurrentBlend = 0.0f;
    bool IsActive = false;
    bool HasExitTime = false;
    float ExitTime = 1.0f;

    struct Condition {
        std::string ParameterName;
        enum class CompareOp { Equals, Greater, Less, NotEqual } Op = CompareOp::Equals;
        float Value = 0.0f;
    };
    std::vector<Condition> Conditions;
};

// ═══════════════════════════════════════════════════════════════
// AnimationParameter — State machine parameters
// ═══════════════════════════════════════════════════════════════
struct AnimationParameter {
    enum class Type { Float, Int, Bool, Trigger } ParamType = Type::Float;
    float FloatValue = 0.0f;
    int IntValue = 0;
    bool BoolValue = false;
    bool TriggerValue = false;
};

// ═══════════════════════════════════════════════════════════════
// AnimationStateMachine — Controls animation flow
// ═══════════════════════════════════════════════════════════════
class AnimationStateMachine {
public:
    AnimationStateMachine() = default;

    // State management
    void AddState(const std::string& name, std::shared_ptr<AnimationClip> clip) {
        AnimationState state;
        state.Name = name;
        state.Clip = clip;
        m_States[name] = state;
        if (m_DefaultState.empty())
            m_DefaultState = name;
    }

    void RemoveState(const std::string& name) { m_States.erase(name); }
    AnimationState* GetState(const std::string& name) {
        auto it = m_States.find(name);
        return (it != m_States.end()) ? &it->second : nullptr;
    }

    void SetDefaultState(const std::string& name) { m_DefaultState = name; }
    const std::string& GetCurrentStateName() const { return m_CurrentState; }
    AnimationState* GetCurrentState() { return GetState(m_CurrentState); }

    // Transitions
    void AddTransition(const AnimationTransition& transition) { m_Transitions.push_back(transition); }

    // Parameters
    void SetFloat(const std::string& name, float value) {
        m_Parameters[name].FloatValue = value;
        m_Parameters[name].ParamType = AnimationParameter::Type::Float;
    }
    void SetInt(const std::string& name, int value) {
        m_Parameters[name].IntValue = value;
        m_Parameters[name].ParamType = AnimationParameter::Type::Int;
    }
    void SetBool(const std::string& name, bool value) {
        m_Parameters[name].BoolValue = value;
        m_Parameters[name].ParamType = AnimationParameter::Type::Bool;
    }
    void SetTrigger(const std::string& name) {
        m_Parameters[name].TriggerValue = true;
        m_Parameters[name].ParamType = AnimationParameter::Type::Trigger;
    }

    float GetFloat(const std::string& name) const {
        auto it = m_Parameters.find(name);
        return (it != m_Parameters.end()) ? it->second.FloatValue : 0.0f;
    }
    int GetInt(const std::string& name) const {
        auto it = m_Parameters.find(name);
        return (it != m_Parameters.end()) ? it->second.IntValue : 0;
    }
    bool GetBool(const std::string& name) const {
        auto it = m_Parameters.find(name);
        return (it != m_Parameters.end()) ? it->second.BoolValue : false;
    }

    // Update
    void Start() {
        m_CurrentState = m_DefaultState;
        if (auto* state = GetCurrentState())
            state->Play();
    }

    void Update(float deltaTime);
    void ForceTransition(const std::string& stateName, float blendTime = 0.3f);

    // Blending output
    struct BlendResult {
        glm::vec3 Position{0.0f};
        glm::quat Rotation{1.0f, 0.0f, 0.0f, 0.0f};
        glm::vec3 Scale{1.0f};
        float Weight = 1.0f;
    };

    BlendResult Sample(const std::string& channelPath) const;

private:
    bool CheckTransitionConditions(const AnimationTransition& transition) const;

private:
    std::unordered_map<std::string, AnimationState> m_States;
    std::vector<AnimationTransition> m_Transitions;
    std::unordered_map<std::string, AnimationParameter> m_Parameters;

    std::string m_DefaultState;
    std::string m_CurrentState;
    std::string m_PreviousState;
    float m_BlendFactor = 0.0f;
    float m_BlendDuration = 0.0f;
    bool m_IsBlending = false;
};

// ═══════════════════════════════════════════════════════════════
// Bone / Skeleton for skeletal animation
// ═══════════════════════════════════════════════════════════════
struct Bone {
    std::string Name;
    int ParentIndex = -1;
    glm::mat4 InverseBindPose{1.0f};
    glm::mat4 LocalTransform{1.0f};
    glm::mat4 GlobalTransform{1.0f};

    glm::vec3 LocalPosition{0.0f};
    glm::quat LocalRotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 LocalScale{1.0f};
};

class Skeleton {
public:
    Skeleton() = default;

    int AddBone(const Bone& bone) {
        m_Bones.push_back(bone);
        m_BoneNameToIndex[bone.Name] = static_cast<int>(m_Bones.size() - 1);
        return static_cast<int>(m_Bones.size() - 1);
    }

    Bone* FindBone(const std::string& name) {
        auto it = m_BoneNameToIndex.find(name);
        if (it != m_BoneNameToIndex.end())
            return &m_Bones[it->second];
        return nullptr;
    }

    int FindBoneIndex(const std::string& name) const {
        auto it = m_BoneNameToIndex.find(name);
        return (it != m_BoneNameToIndex.end()) ? it->second : -1;
    }

    size_t GetBoneCount() const { return m_Bones.size(); }
    Bone& GetBone(int index) { return m_Bones[index]; }
    const Bone& GetBone(int index) const { return m_Bones[index]; }

    const std::vector<Bone>& GetBones() const { return m_Bones; }

    void UpdateGlobalTransforms() {
        for (size_t i = 0; i < m_Bones.size(); i++) {
            auto& bone = m_Bones[i];
            glm::mat4 local = glm::translate(glm::mat4(1.0f), bone.LocalPosition) * glm::mat4_cast(bone.LocalRotation) *
                              glm::scale(glm::mat4(1.0f), bone.LocalScale);
            bone.LocalTransform = local;

            if (bone.ParentIndex >= 0) {
                bone.GlobalTransform = m_Bones[bone.ParentIndex].GlobalTransform * local;
            } else {
                bone.GlobalTransform = local;
            }
        }
    }

    std::vector<glm::mat4> GetFinalBoneMatrices() const {
        std::vector<glm::mat4> matrices(m_Bones.size());
        for (size_t i = 0; i < m_Bones.size(); i++) {
            matrices[i] = m_Bones[i].GlobalTransform * m_Bones[i].InverseBindPose;
        }
        return matrices;
    }

private:
    std::vector<Bone> m_Bones;
    std::unordered_map<std::string, int> m_BoneNameToIndex;
};

}  // namespace PyEngine
