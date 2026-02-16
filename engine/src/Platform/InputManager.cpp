#include "PyEngine/Platform/InputManager.hpp"

#include <fstream>

#include "PyEngine/Core/KeyCodes.hpp"
#include "PyEngine/Platform/Input.hpp"

namespace PyEngine {

InputContext& InputManager::CreateContext(const std::string& name, int priority) {
    InputContext context;
    context.Name = name;
    context.Priority = priority;
    m_Contexts.push_back(context);

    // Sort by priority (higher = processed first)
    std::sort(m_Contexts.begin(), m_Contexts.end(),
              [](const InputContext& a, const InputContext& b) { return a.Priority > b.Priority; });

    return m_Contexts.back();
}

void InputManager::RemoveContext(const std::string& name) {
    m_Contexts.erase(
        std::remove_if(m_Contexts.begin(), m_Contexts.end(), [&](const InputContext& c) { return c.Name == name; }),
        m_Contexts.end());
}

InputContext* InputManager::GetContext(const std::string& name) {
    for (auto& ctx : m_Contexts) {
        if (ctx.Name == name)
            return &ctx;
    }
    return nullptr;
}

void InputManager::EnableContext(const std::string& name) {
    if (auto* ctx = GetContext(name))
        ctx->Enabled = true;
}

void InputManager::DisableContext(const std::string& name) {
    if (auto* ctx = GetContext(name))
        ctx->Enabled = false;
}

InputAction& InputManager::AddButtonAction(const std::string& contextName, const std::string& actionName, int keyCode) {
    auto* ctx = GetContext(contextName);
    if (!ctx) {
        CreateContext(contextName);
        ctx = GetContext(contextName);
    }

    InputAction action;
    action.Name = actionName;
    action.Type = InputActionType::Button;

    InputBinding binding;
    binding.KeyCode = keyCode;
    action.Bindings.push_back(binding);

    ctx->Actions.push_back(action);
    return ctx->Actions.back();
}

InputAction& InputManager::AddAxisAction(const std::string& contextName, const std::string& actionName, int positiveKey,
                                         int negativeKey) {
    auto* ctx = GetContext(contextName);
    if (!ctx) {
        CreateContext(contextName);
        ctx = GetContext(contextName);
    }

    InputAction action;
    action.Name = actionName;
    action.Type = InputActionType::Axis;

    InputBinding binding;
    binding.PositiveKey = positiveKey;
    binding.NegativeKey = negativeKey;
    action.Bindings.push_back(binding);

    ctx->Actions.push_back(action);
    return ctx->Actions.back();
}

InputAction& InputManager::AddMouseButtonAction(const std::string& contextName, const std::string& actionName,
                                                int mouseButton) {
    auto* ctx = GetContext(contextName);
    if (!ctx) {
        CreateContext(contextName);
        ctx = GetContext(contextName);
    }

    InputAction action;
    action.Name = actionName;
    action.Type = InputActionType::Button;

    InputBinding binding;
    binding.MouseButton = mouseButton;
    action.Bindings.push_back(binding);

    ctx->Actions.push_back(action);
    return ctx->Actions.back();
}

bool InputManager::IsActionStarted(const std::string& actionName) const {
    for (const auto& ctx : m_Contexts) {
        if (!ctx.Enabled)
            continue;
        for (const auto& action : ctx.Actions) {
            if (action.Name == actionName)
                return action.Phase == InputActionPhase::Started;
        }
    }
    return false;
}

bool InputManager::IsActionPerformed(const std::string& actionName) const {
    for (const auto& ctx : m_Contexts) {
        if (!ctx.Enabled)
            continue;
        for (const auto& action : ctx.Actions) {
            if (action.Name == actionName)
                return action.IsPerformed;
        }
    }
    return false;
}

bool InputManager::IsActionCanceled(const std::string& actionName) const {
    for (const auto& ctx : m_Contexts) {
        if (!ctx.Enabled)
            continue;
        for (const auto& action : ctx.Actions) {
            if (action.Name == actionName)
                return action.Phase == InputActionPhase::Canceled;
        }
    }
    return false;
}

float InputManager::GetActionValue(const std::string& actionName) const {
    for (const auto& ctx : m_Contexts) {
        if (!ctx.Enabled)
            continue;
        for (const auto& action : ctx.Actions) {
            if (action.Name == actionName)
                return action.Value;
        }
    }
    return 0.0f;
}

void InputManager::Update() {
    m_Stats.TotalContexts = static_cast<uint32_t>(m_Contexts.size());
    m_Stats.ActiveContexts = 0;
    m_Stats.TotalActions = 0;

    for (auto& ctx : m_Contexts) {
        if (!ctx.Enabled)
            continue;
        m_Stats.ActiveContexts++;

        for (auto& action : ctx.Actions) {
            UpdateAction(action);
            m_Stats.TotalActions++;
        }
    }

    // Update gamepad state
    m_Stats.GamepadsConnected = 0;
    for (int i = 0; i < 4; i++) {
        // Placeholder for actual gamepad polling
        if (m_Gamepads[i].Connected)
            m_Stats.GamepadsConnected++;
    }
}

void InputManager::UpdateAction(InputAction& action) {
    action.PreviousValue = action.Value;
    action.WasPerformed = action.IsPerformed;

    float maxValue = 0.0f;
    for (const auto& binding : action.Bindings) {
        float value = EvaluateBinding(binding, action.Type);
        if (std::abs(value) > std::abs(maxValue)) {
            maxValue = value;
        }
    }

    action.Value = maxValue;

    // Determine phase
    if (action.Type == InputActionType::Button) {
        bool isPressed = std::abs(action.Value) > 0.5f;
        bool wasPressed = std::abs(action.PreviousValue) > 0.5f;

        if (isPressed && !wasPressed) {
            action.Phase = InputActionPhase::Started;
            action.IsPerformed = true;
            if (action.OnStarted)
                action.OnStarted(action);
        } else if (isPressed && wasPressed) {
            action.Phase = InputActionPhase::Performed;
            if (action.OnPerformed)
                action.OnPerformed(action);
        } else if (!isPressed && wasPressed) {
            action.Phase = InputActionPhase::Canceled;
            action.IsPerformed = false;
            if (action.OnCanceled)
                action.OnCanceled(action);
        } else {
            action.Phase = InputActionPhase::None;
        }
    } else {
        // Axis
        bool hasValue = std::abs(action.Value) > 0.01f;
        bool hadValue = std::abs(action.PreviousValue) > 0.01f;

        if (hasValue && !hadValue) {
            action.Phase = InputActionPhase::Started;
            action.IsPerformed = true;
        } else if (hasValue) {
            action.Phase = InputActionPhase::Performed;
        } else if (!hasValue && hadValue) {
            action.Phase = InputActionPhase::Canceled;
            action.IsPerformed = false;
        } else {
            action.Phase = InputActionPhase::None;
        }
    }
}

float InputManager::EvaluateBinding(const InputBinding& binding, InputActionType type) {
    if (type == InputActionType::Button) {
        if (binding.KeyCode >= 0 && Input::IsKeyPressed(binding.KeyCode))
            return 1.0f;
        if (binding.MouseButton >= 0 && Input::IsMouseButtonPressed(binding.MouseButton))
            return 1.0f;
        return 0.0f;
    }

    if (type == InputActionType::Axis) {
        float value = 0.0f;

        // Composite axis
        if (binding.PositiveKey >= 0 && Input::IsKeyPressed(binding.PositiveKey))
            value += 1.0f;
        if (binding.NegativeKey >= 0 && Input::IsKeyPressed(binding.NegativeKey))
            value -= 1.0f;

        // Apply dead zone
        if (std::abs(value) < binding.DeadZone)
            value = 0.0f;

        // Apply sensitivity and invert
        value *= binding.Sensitivity;
        if (binding.Invert)
            value = -value;

        return std::clamp(value, -1.0f, 1.0f);
    }

    return 0.0f;
}

void InputManager::StartRebinding(const std::string& contextName, const std::string& actionName) {
    m_IsRebinding = true;
    m_RebindContext = contextName;
    m_RebindAction = actionName;
}

void InputManager::CancelRebinding() {
    m_IsRebinding = false;
    m_RebindContext.clear();
    m_RebindAction.clear();
}

const InputManager::GamepadState& InputManager::GetGamepadState(int index) const {
    return m_Gamepads[std::clamp(index, 0, 3)];
}

bool InputManager::IsGamepadConnected(int index) const {
    return m_Gamepads[std::clamp(index, 0, 3)].Connected;
}

void InputManager::SetRumble(int index, float left, float right) {
    int i = std::clamp(index, 0, 3);
    m_Gamepads[i].RumbleLeft = std::clamp(left, 0.0f, 1.0f);
    m_Gamepads[i].RumbleRight = std::clamp(right, 0.0f, 1.0f);
}

void InputManager::SetupDefaultEditorBindings() {
    auto& ctx = CreateContext("Editor", 100);

    // Navigation
    InputAction moveForward;
    moveForward.Name = "MoveForward";
    moveForward.Type = InputActionType::Button;
    InputBinding wBind;
    wBind.KeyCode = Key::W;
    moveForward.Bindings.push_back(wBind);
    ctx.Actions.push_back(moveForward);

    InputAction moveBack;
    moveBack.Name = "MoveBackward";
    moveBack.Type = InputActionType::Button;
    InputBinding sBind;
    sBind.KeyCode = Key::S;
    moveBack.Bindings.push_back(sBind);
    ctx.Actions.push_back(moveBack);

    InputAction moveLeft;
    moveLeft.Name = "MoveLeft";
    moveLeft.Type = InputActionType::Button;
    InputBinding aBind;
    aBind.KeyCode = Key::A;
    moveLeft.Bindings.push_back(aBind);
    ctx.Actions.push_back(moveLeft);

    InputAction moveRight;
    moveRight.Name = "MoveRight";
    moveRight.Type = InputActionType::Button;
    InputBinding dBind;
    dBind.KeyCode = Key::D;
    moveRight.Bindings.push_back(dBind);
    ctx.Actions.push_back(moveRight);
}

void InputManager::SetupDefaultGameBindings() {
    auto& ctx = CreateContext("Gameplay", 50);

    // Movement axes
    AddAxisAction("Gameplay", "Horizontal", Key::D, Key::A);
    AddAxisAction("Gameplay", "Vertical", Key::W, Key::S);

    // Actions
    AddButtonAction("Gameplay", "Jump", Key::Space);
    AddButtonAction("Gameplay", "Sprint", Key::LeftShift);
    AddButtonAction("Gameplay", "Interact", Key::E);
    AddButtonAction("Gameplay", "Fire", Key::Space);
    AddMouseButtonAction("Gameplay", "PrimaryFire", 0);
    AddMouseButtonAction("Gameplay", "SecondaryFire", 1);
}

void InputManager::SaveBindings(const std::string& filepath) const {
    std::ofstream file(filepath);
    if (!file)
        return;

    for (const auto& ctx : m_Contexts) {
        file << "[Context] " << ctx.Name << " Priority=" << ctx.Priority << "\n";
        for (const auto& action : ctx.Actions) {
            file << "  Action: " << action.Name << " Type=" << static_cast<int>(action.Type) << "\n";
            for (const auto& binding : action.Bindings) {
                file << "    Binding: Key=" << binding.KeyCode << " Mouse=" << binding.MouseButton
                     << " Gamepad=" << binding.GamepadButton << " PosKey=" << binding.PositiveKey
                     << " NegKey=" << binding.NegativeKey << "\n";
            }
        }
    }
}

void InputManager::LoadBindings(const std::string& /*filepath*/) {
    // TODO: Parse saved bindings
}

}  // namespace PyEngine
