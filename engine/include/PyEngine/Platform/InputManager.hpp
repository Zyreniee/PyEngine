#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// InputAction — Named action for input abstraction
// ═══════════════════════════════════════════════════════════════
enum class InputActionType {
    Button,  // On/off (pressed, held, released)
    Axis,    // -1.0 to 1.0 (analog stick, triggers)
    Axis2D,  // Two axes combined (mouse delta, stick)
};

enum class InputActionPhase {
    None,
    Started,    // Just pressed
    Performed,  // Held down or value changed
    Canceled    // Released
};

struct InputBinding {
    int KeyCode = -1;
    int MouseButton = -1;
    int GamepadButton = -1;
    int GamepadAxis = -1;
    float DeadZone = 0.1f;
    float Sensitivity = 1.0f;
    bool Invert = false;

    // Composite axis (positive/negative keys)
    int PositiveKey = -1;
    int NegativeKey = -1;
};

struct InputAction {
    std::string Name;
    InputActionType Type = InputActionType::Button;
    std::vector<InputBinding> Bindings;

    InputActionPhase Phase = InputActionPhase::None;
    float Value = 0.0f;
    float PreviousValue = 0.0f;
    bool WasPerformed = false;
    bool IsPerformed = false;

    // Callbacks
    using Callback = std::function<void(const InputAction&)>;
    Callback OnStarted;
    Callback OnPerformed;
    Callback OnCanceled;
};

// ═══════════════════════════════════════════════════════════════
// InputContext — Group of actions that can be enabled/disabled
// ═══════════════════════════════════════════════════════════════
struct InputContext {
    std::string Name;
    bool Enabled = true;
    int Priority = 0;
    std::vector<InputAction> Actions;

    InputAction* FindAction(const std::string& name) {
        for (auto& action : Actions) {
            if (action.Name == name)
                return &action;
        }
        return nullptr;
    }
};

// ═══════════════════════════════════════════════════════════════
// InputManager — Advanced input mapping system
// ═══════════════════════════════════════════════════════════════
class InputManager {
public:
    static InputManager& Get() {
        static InputManager instance;
        return instance;
    }

    // Context management
    InputContext& CreateContext(const std::string& name, int priority = 0);
    void RemoveContext(const std::string& name);
    InputContext* GetContext(const std::string& name);
    void EnableContext(const std::string& name);
    void DisableContext(const std::string& name);

    // Action creation helpers
    InputAction& AddButtonAction(const std::string& contextName, const std::string& actionName, int keyCode);
    InputAction& AddAxisAction(const std::string& contextName, const std::string& actionName, int positiveKey,
                               int negativeKey);
    InputAction& AddMouseButtonAction(const std::string& contextName, const std::string& actionName, int mouseButton);

    // Queries (check actions across all active contexts)
    bool IsActionStarted(const std::string& actionName) const;
    bool IsActionPerformed(const std::string& actionName) const;
    bool IsActionCanceled(const std::string& actionName) const;
    float GetActionValue(const std::string& actionName) const;

    // Update
    void Update();

    // Rebinding
    void StartRebinding(const std::string& contextName, const std::string& actionName);
    bool IsRebinding() const { return m_IsRebinding; }
    void CancelRebinding();

    // Gamepad
    struct GamepadState {
        bool Connected = false;
        std::string Name;
        float Axes[6] = {};
        bool Buttons[16] = {};
        float LeftTrigger = 0.0f;
        float RightTrigger = 0.0f;
        float RumbleLeft = 0.0f;
        float RumbleRight = 0.0f;
    };

    const GamepadState& GetGamepadState(int index = 0) const;
    bool IsGamepadConnected(int index = 0) const;
    void SetRumble(int index, float left, float right);

    // Defaults
    void SetupDefaultEditorBindings();
    void SetupDefaultGameBindings();

    // Serialization
    void SaveBindings(const std::string& filepath) const;
    void LoadBindings(const std::string& filepath);

    // Stats
    struct Stats {
        uint32_t TotalContexts = 0;
        uint32_t ActiveContexts = 0;
        uint32_t TotalActions = 0;
        uint32_t GamepadsConnected = 0;
    };
    const Stats& GetStats() const { return m_Stats; }

private:
    InputManager() = default;
    void UpdateAction(InputAction& action);
    float EvaluateBinding(const InputBinding& binding, InputActionType type);

private:
    std::vector<InputContext> m_Contexts;
    GamepadState m_Gamepads[4];

    bool m_IsRebinding = false;
    std::string m_RebindContext;
    std::string m_RebindAction;

    Stats m_Stats;
};

}  // namespace PyEngine
