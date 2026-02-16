#include "PyEngine/Animation/AnimationSystem.hpp"

namespace PyEngine {

void AnimationStateMachine::Update(float deltaTime) {
    if (m_CurrentState.empty())
        return;

    auto* current = GetCurrentState();
    if (!current || !current->IsPlaying || current->IsPaused)
        return;

    // Update current state time
    current->CurrentTime += deltaTime * current->Speed;
    if (current->Clip) {
        if (current->Clip->IsLooping && current->CurrentTime >= current->Clip->Duration) {
            current->CurrentTime = std::fmod(current->CurrentTime, current->Clip->Duration);
        }
    }

    // Handle blend transition
    if (m_IsBlending && m_BlendDuration > 0.0f) {
        m_BlendFactor += deltaTime / m_BlendDuration;
        if (m_BlendFactor >= 1.0f) {
            m_BlendFactor = 1.0f;
            m_IsBlending = false;
            m_PreviousState.clear();
        }

        // Also update previous state time
        if (auto* prev = GetState(m_PreviousState)) {
            prev->CurrentTime += deltaTime * prev->Speed;
        }
    }

    // Check transitions
    for (auto& transition : m_Transitions) {
        if (transition.FromState != m_CurrentState)
            continue;
        if (transition.IsActive)
            continue;

        // Check exit time
        if (transition.HasExitTime && current->GetNormalizedTime() < transition.ExitTime)
            continue;

        // Check conditions
        if (CheckTransitionConditions(transition)) {
            ForceTransition(transition.ToState, transition.Duration);
            break;
        }
    }

    // Reset triggers
    for (auto& [name, param] : m_Parameters) {
        if (param.ParamType == AnimationParameter::Type::Trigger) {
            param.TriggerValue = false;
        }
    }
}

void AnimationStateMachine::ForceTransition(const std::string& stateName, float blendTime) {
    if (m_CurrentState == stateName)
        return;

    auto* targetState = GetState(stateName);
    if (!targetState)
        return;

    m_PreviousState = m_CurrentState;
    m_CurrentState = stateName;
    m_BlendFactor = 0.0f;
    m_BlendDuration = blendTime;
    m_IsBlending = blendTime > 0.0f;

    targetState->Reset();
    targetState->Play();
}

bool AnimationStateMachine::CheckTransitionConditions(const AnimationTransition& transition) const {
    if (transition.Conditions.empty())
        return false;

    for (const auto& condition : transition.Conditions) {
        auto it = m_Parameters.find(condition.ParameterName);
        if (it == m_Parameters.end())
            return false;

        const auto& param = it->second;
        float value = 0.0f;

        switch (param.ParamType) {
            case AnimationParameter::Type::Float:
                value = param.FloatValue;
                break;
            case AnimationParameter::Type::Int:
                value = static_cast<float>(param.IntValue);
                break;
            case AnimationParameter::Type::Bool:
                value = param.BoolValue ? 1.0f : 0.0f;
                break;
            case AnimationParameter::Type::Trigger:
                value = param.TriggerValue ? 1.0f : 0.0f;
                break;
        }

        switch (condition.Op) {
            case AnimationTransition::Condition::CompareOp::Equals:
                if (std::abs(value - condition.Value) > 1e-6f)
                    return false;
                break;
            case AnimationTransition::Condition::CompareOp::Greater:
                if (value <= condition.Value)
                    return false;
                break;
            case AnimationTransition::Condition::CompareOp::Less:
                if (value >= condition.Value)
                    return false;
                break;
            case AnimationTransition::Condition::CompareOp::NotEqual:
                if (std::abs(value - condition.Value) <= 1e-6f)
                    return false;
                break;
        }
    }

    return true;
}

AnimationStateMachine::BlendResult AnimationStateMachine::Sample(const std::string& channelPath) const {
    BlendResult result;

    auto sampleState = [&](const std::string& stateName) -> BlendResult {
        BlendResult r;
        auto it = m_States.find(stateName);
        if (it == m_States.end() || !it->second.Clip)
            return r;

        const auto& state = it->second;
        const auto& clip = *state.Clip;

        for (const auto& channel : clip.Channels) {
            if (channel.TargetPath != channelPath)
                continue;

            switch (channel.Property) {
                case AnimationProperty::Position:
                    r.Position = channel.SampleVec3(state.CurrentTime);
                    break;
                case AnimationProperty::Rotation:
                    r.Rotation = channel.SampleQuat(state.CurrentTime);
                    break;
                case AnimationProperty::Scale:
                    r.Scale = channel.SampleVec3(state.CurrentTime);
                    break;
                default:
                    break;
            }
        }
        r.Weight = state.Weight;
        return r;
    };

    if (m_IsBlending && !m_PreviousState.empty()) {
        auto prev = sampleState(m_PreviousState);
        auto curr = sampleState(m_CurrentState);

        result.Position = glm::mix(prev.Position, curr.Position, m_BlendFactor);
        result.Rotation = glm::slerp(prev.Rotation, curr.Rotation, m_BlendFactor);
        result.Scale = glm::mix(prev.Scale, curr.Scale, m_BlendFactor);
    } else {
        result = sampleState(m_CurrentState);
    }

    return result;
}

}  // namespace PyEngine
