#pragma once

#include <any>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// Event System — Type-safe publish/subscribe event bus
// ═══════════════════════════════════════════════════════════════

// Base event class
struct Event {
    bool Handled = false;
    virtual ~Event() = default;
    virtual const char* GetName() const = 0;
    virtual std::type_index GetType() const = 0;
};

// ── Application Events ──────────────────────────────────────
struct WindowResizeEvent : Event {
    uint32_t Width, Height;
    WindowResizeEvent(uint32_t w, uint32_t h) : Width(w), Height(h) {}
    const char* GetName() const override { return "WindowResize"; }
    std::type_index GetType() const override { return typeid(WindowResizeEvent); }
};

struct WindowCloseEvent : Event {
    const char* GetName() const override { return "WindowClose"; }
    std::type_index GetType() const override { return typeid(WindowCloseEvent); }
};

struct WindowFocusEvent : Event {
    bool Focused;
    WindowFocusEvent(bool focused) : Focused(focused) {}
    const char* GetName() const override { return "WindowFocus"; }
    std::type_index GetType() const override { return typeid(WindowFocusEvent); }
};

struct WindowMinimizeEvent : Event {
    bool Minimized;
    WindowMinimizeEvent(bool min) : Minimized(min) {}
    const char* GetName() const override { return "WindowMinimize"; }
    std::type_index GetType() const override { return typeid(WindowMinimizeEvent); }
};

// ── Input Events ────────────────────────────────────────────
struct KeyPressedEvent : Event {
    int KeyCode;
    bool Repeat;
    KeyPressedEvent(int key, bool repeat) : KeyCode(key), Repeat(repeat) {}
    const char* GetName() const override { return "KeyPressed"; }
    std::type_index GetType() const override { return typeid(KeyPressedEvent); }
};

struct KeyReleasedEvent : Event {
    int KeyCode;
    KeyReleasedEvent(int key) : KeyCode(key) {}
    const char* GetName() const override { return "KeyReleased"; }
    std::type_index GetType() const override { return typeid(KeyReleasedEvent); }
};

struct KeyTypedEvent : Event {
    unsigned int Codepoint;
    KeyTypedEvent(unsigned int cp) : Codepoint(cp) {}
    const char* GetName() const override { return "KeyTyped"; }
    std::type_index GetType() const override { return typeid(KeyTypedEvent); }
};

struct MouseMovedEvent : Event {
    float X, Y;
    MouseMovedEvent(float x, float y) : X(x), Y(y) {}
    const char* GetName() const override { return "MouseMoved"; }
    std::type_index GetType() const override { return typeid(MouseMovedEvent); }
};

struct MouseScrolledEvent : Event {
    float XOffset, YOffset;
    MouseScrolledEvent(float x, float y) : XOffset(x), YOffset(y) {}
    const char* GetName() const override { return "MouseScrolled"; }
    std::type_index GetType() const override { return typeid(MouseScrolledEvent); }
};

struct MouseButtonPressedEvent : Event {
    int Button;
    MouseButtonPressedEvent(int btn) : Button(btn) {}
    const char* GetName() const override { return "MouseButtonPressed"; }
    std::type_index GetType() const override { return typeid(MouseButtonPressedEvent); }
};

struct MouseButtonReleasedEvent : Event {
    int Button;
    MouseButtonReleasedEvent(int btn) : Button(btn) {}
    const char* GetName() const override { return "MouseButtonReleased"; }
    std::type_index GetType() const override { return typeid(MouseButtonReleasedEvent); }
};

// ── Scene Events ────────────────────────────────────────────
struct SceneLoadedEvent : Event {
    std::string SceneName;
    SceneLoadedEvent(const std::string& name) : SceneName(name) {}
    const char* GetName() const override { return "SceneLoaded"; }
    std::type_index GetType() const override { return typeid(SceneLoadedEvent); }
};

struct SceneUnloadedEvent : Event {
    std::string SceneName;
    SceneUnloadedEvent(const std::string& name) : SceneName(name) {}
    const char* GetName() const override { return "SceneUnloaded"; }
    std::type_index GetType() const override { return typeid(SceneUnloadedEvent); }
};

struct EntityCreatedEvent : Event {
    uint32_t EntityID;
    EntityCreatedEvent(uint32_t id) : EntityID(id) {}
    const char* GetName() const override { return "EntityCreated"; }
    std::type_index GetType() const override { return typeid(EntityCreatedEvent); }
};

struct EntityDestroyedEvent : Event {
    uint32_t EntityID;
    EntityDestroyedEvent(uint32_t id) : EntityID(id) {}
    const char* GetName() const override { return "EntityDestroyed"; }
    std::type_index GetType() const override { return typeid(EntityDestroyedEvent); }
};

struct ComponentAddedEvent : Event {
    uint32_t EntityID;
    std::string ComponentName;
    ComponentAddedEvent(uint32_t id, const std::string& name) : EntityID(id), ComponentName(name) {}
    const char* GetName() const override { return "ComponentAdded"; }
    std::type_index GetType() const override { return typeid(ComponentAddedEvent); }
};

struct ComponentRemovedEvent : Event {
    uint32_t EntityID;
    std::string ComponentName;
    ComponentRemovedEvent(uint32_t id, const std::string& name) : EntityID(id), ComponentName(name) {}
    const char* GetName() const override { return "ComponentRemoved"; }
    std::type_index GetType() const override { return typeid(ComponentRemovedEvent); }
};

// ── Physics Events ──────────────────────────────────────────
struct CollisionEvent : Event {
    uint32_t EntityA, EntityB;
    glm::vec3 ContactPoint;
    glm::vec3 Normal;
    float Penetration;
    enum class Phase { Enter, Stay, Exit } EventPhase;
    CollisionEvent(uint32_t a, uint32_t b, Phase phase)
        : EntityA(a), EntityB(b), ContactPoint(0.0f), Normal(0.0f, 1.0f, 0.0f), Penetration(0.0f), EventPhase(phase) {}
    const char* GetName() const override { return "Collision"; }
    std::type_index GetType() const override { return typeid(CollisionEvent); }
};

struct TriggerEvent : Event {
    uint32_t EntityA, EntityB;
    enum class Phase { Enter, Stay, Exit } EventPhase;
    TriggerEvent(uint32_t a, uint32_t b, Phase phase) : EntityA(a), EntityB(b), EventPhase(phase) {}
    const char* GetName() const override { return "Trigger"; }
    std::type_index GetType() const override { return typeid(TriggerEvent); }
};

// ── Audio Events ────────────────────────────────────────────
struct AudioPlayEvent : Event {
    uint32_t SourceID;
    std::string ClipName;
    AudioPlayEvent(uint32_t id, const std::string& clip) : SourceID(id), ClipName(clip) {}
    const char* GetName() const override { return "AudioPlay"; }
    std::type_index GetType() const override { return typeid(AudioPlayEvent); }
};

struct AudioStopEvent : Event {
    uint32_t SourceID;
    AudioStopEvent(uint32_t id) : SourceID(id) {}
    const char* GetName() const override { return "AudioStop"; }
    std::type_index GetType() const override { return typeid(AudioStopEvent); }
};

// ── Editor Events ───────────────────────────────────────────
struct SelectionChangedEvent : Event {
    uint32_t EntityID;
    SelectionChangedEvent(uint32_t id) : EntityID(id) {}
    const char* GetName() const override { return "SelectionChanged"; }
    std::type_index GetType() const override { return typeid(SelectionChangedEvent); }
};

struct PlayModeChangedEvent : Event {
    enum class Mode { Edit, Play, Pause } NewMode;
    PlayModeChangedEvent(Mode mode) : NewMode(mode) {}
    const char* GetName() const override { return "PlayModeChanged"; }
    std::type_index GetType() const override { return typeid(PlayModeChangedEvent); }
};

// ═══════════════════════════════════════════════════════════════
// EventDispatcher — Type-safe event subscription & dispatch
// ═══════════════════════════════════════════════════════════════
class EventDispatcher {
public:
    static EventDispatcher& Get() {
        static EventDispatcher instance;
        return instance;
    }

    using HandlerID = uint64_t;

    template <typename T>
    HandlerID Subscribe(std::function<void(const T&)> handler) {
        auto wrappedHandler = [handler](const Event& event) { handler(static_cast<const T&>(event)); };
        HandlerID id = m_NextID++;
        m_Handlers[typeid(T)].push_back({id, wrappedHandler});
        m_HandlerCount++;
        return id;
    }

    void Unsubscribe(HandlerID id) {
        for (auto& [type, handlers] : m_Handlers) {
            handlers.erase(
                std::remove_if(handlers.begin(), handlers.end(), [id](const HandlerEntry& e) { return e.ID == id; }),
                handlers.end());
        }
        m_HandlerCount--;
    }

    template <typename T>
    void Dispatch(const T& event) {
        auto it = m_Handlers.find(typeid(T));
        if (it == m_Handlers.end())
            return;

        for (auto& handler : it->second) {
            handler.Callback(event);
            if (event.Handled)
                break;
        }
        m_EventsDispatched++;
    }

    template <typename T, typename... Args>
    void DispatchImmediate(Args&&... args) {
        T event(std::forward<Args>(args)...);
        Dispatch(event);
    }

    // Deferred dispatch (queue for batch processing)
    void QueueEvent(std::shared_ptr<Event> event) { m_EventQueue.push(event); }

    void ProcessQueue() {
        while (!m_EventQueue.empty()) {
            auto event = m_EventQueue.front();
            m_EventQueue.pop();

            auto it = m_Handlers.find(event->GetType());
            if (it != m_Handlers.end()) {
                for (auto& handler : it->second) {
                    handler.Callback(*event);
                    if (event->Handled)
                        break;
                }
            }
        }
    }

    void ClearAll() {
        m_Handlers.clear();
        while (!m_EventQueue.empty())
            m_EventQueue.pop();
        m_HandlerCount = 0;
    }

    // Stats
    uint32_t GetHandlerCount() const { return m_HandlerCount; }
    uint32_t GetEventsDispatched() const { return m_EventsDispatched; }
    uint32_t GetQueueSize() const { return static_cast<uint32_t>(m_EventQueue.size()); }

private:
    EventDispatcher() = default;

    struct HandlerEntry {
        HandlerID ID;
        std::function<void(const Event&)> Callback;
    };

    std::unordered_map<std::type_index, std::vector<HandlerEntry>> m_Handlers;
    std::queue<std::shared_ptr<Event>> m_EventQueue;
    HandlerID m_NextID = 1;
    uint32_t m_HandlerCount = 0;
    uint32_t m_EventsDispatched = 0;
};

}  // namespace PyEngine
