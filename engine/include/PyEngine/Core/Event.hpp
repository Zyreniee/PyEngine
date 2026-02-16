#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace PyEngine {

// ─── Event Type Enum ─────────────────────────────────────────
enum class EventType : uint32_t {
    None = 0,
    // Window
    WindowClose,
    WindowResize,
    WindowFocus,
    WindowLostFocus,
    WindowMoved,
    // Key
    KeyPressed,
    KeyReleased,
    KeyTyped,
    // Mouse
    MouseButtonPressed,
    MouseButtonReleased,
    MouseMoved,
    MouseScrolled
};

enum EventCategory : uint32_t {
    EventCategoryNone = 0,
    EventCategoryApplication = 1 << 0,
    EventCategoryInput = 1 << 1,
    EventCategoryKeyboard = 1 << 2,
    EventCategoryMouse = 1 << 3,
    EventCategoryMouseButton = 1 << 4
};

// ─── Event Base Class ────────────────────────────────────────
class Event {
public:
    virtual ~Event() = default;

    bool Handled = false;

    virtual EventType GetEventType() const = 0;
    virtual const char* GetName() const = 0;
    virtual uint32_t GetCategoryFlags() const = 0;

    bool IsInCategory(EventCategory category) const { return GetCategoryFlags() & category; }
};

// ─── Event Dispatcher ────────────────────────────────────────
class EventDispatcher {
public:
    explicit EventDispatcher(Event& event) : m_Event(event) {}

    template <typename T, typename F>
    bool Dispatch(const F& func) {
        if (m_Event.GetEventType() == T::GetStaticType()) {
            m_Event.Handled |= func(static_cast<T&>(m_Event));
            return true;
        }
        return false;
    }

private:
    Event& m_Event;
};

// ─── Macros ──────────────────────────────────────────────────
#define EVENT_CLASS_TYPE(type)                \
    static EventType GetStaticType() {        \
        return EventType::type;               \
    }                                         \
    EventType GetEventType() const override { \
        return GetStaticType();               \
    }                                         \
    const char* GetName() const override {    \
        return #type;                         \
    }

#define EVENT_CLASS_CATEGORY(category)           \
    uint32_t GetCategoryFlags() const override { \
        return category;                         \
    }

// ─── Window Events ──────────────────────────────────────────
class WindowResizeEvent : public Event {
public:
    WindowResizeEvent(uint32_t width, uint32_t height) : m_Width(width), m_Height(height) {}

    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }

    EVENT_CLASS_TYPE(WindowResize)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)

private:
    uint32_t m_Width, m_Height;
};

class WindowCloseEvent : public Event {
public:
    WindowCloseEvent() = default;

    EVENT_CLASS_TYPE(WindowClose)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

// ─── Key Events ─────────────────────────────────────────────
class KeyEvent : public Event {
public:
    int GetKeyCode() const { return m_KeyCode; }

    EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryKeyboard)

protected:
    explicit KeyEvent(int keycode) : m_KeyCode(keycode) {}
    int m_KeyCode;
};

class KeyPressedEvent : public KeyEvent {
public:
    KeyPressedEvent(int keycode, bool isRepeat = false) : KeyEvent(keycode), m_IsRepeat(isRepeat) {}

    bool IsRepeat() const { return m_IsRepeat; }

    EVENT_CLASS_TYPE(KeyPressed)

private:
    bool m_IsRepeat;
};

class KeyReleasedEvent : public KeyEvent {
public:
    explicit KeyReleasedEvent(int keycode) : KeyEvent(keycode) {}

    EVENT_CLASS_TYPE(KeyReleased)
};

class KeyTypedEvent : public KeyEvent {
public:
    explicit KeyTypedEvent(int keycode) : KeyEvent(keycode) {}

    EVENT_CLASS_TYPE(KeyTyped)
};

// ─── Mouse Events ───────────────────────────────────────────
class MouseMovedEvent : public Event {
public:
    MouseMovedEvent(float x, float y) : m_MouseX(x), m_MouseY(y) {}

    float GetX() const { return m_MouseX; }
    float GetY() const { return m_MouseY; }

    EVENT_CLASS_TYPE(MouseMoved)
    EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)

private:
    float m_MouseX, m_MouseY;
};

class MouseScrolledEvent : public Event {
public:
    MouseScrolledEvent(float xOffset, float yOffset) : m_XOffset(xOffset), m_YOffset(yOffset) {}

    float GetXOffset() const { return m_XOffset; }
    float GetYOffset() const { return m_YOffset; }

    EVENT_CLASS_TYPE(MouseScrolled)
    EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)

private:
    float m_XOffset, m_YOffset;
};

class MouseButtonPressedEvent : public Event {
public:
    explicit MouseButtonPressedEvent(int button) : m_Button(button) {}

    int GetButton() const { return m_Button; }

    EVENT_CLASS_TYPE(MouseButtonPressed)
    EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse | EventCategoryMouseButton)

private:
    int m_Button;
};

class MouseButtonReleasedEvent : public Event {
public:
    explicit MouseButtonReleasedEvent(int button) : m_Button(button) {}

    int GetButton() const { return m_Button; }

    EVENT_CLASS_TYPE(MouseButtonReleased)
    EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse | EventCategoryMouseButton)

private:
    int m_Button;
};

using EventCallbackFn = std::function<void(Event&)>;

}  // namespace PyEngine
