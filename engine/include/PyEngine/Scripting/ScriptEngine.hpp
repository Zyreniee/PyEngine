#pragma once

#include <any>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// Script — Base class for all scripts
// ═══════════════════════════════════════════════════════════════
class Script {
public:
    virtual ~Script() = default;

    // Lifecycle callbacks (like Unity MonoBehaviour)
    virtual void OnCreate() {}
    virtual void OnStart() {}
    virtual void OnUpdate(float deltaTime) {}
    virtual void OnFixedUpdate(float fixedDeltaTime) {}
    virtual void OnLateUpdate(float deltaTime) {}
    virtual void OnDestroy() {}

    // Enable/Disable
    virtual void OnEnable() {}
    virtual void OnDisable() {}

    // Physics callbacks
    virtual void OnCollisionEnter(uint32_t otherEntity) {}
    virtual void OnCollisionStay(uint32_t otherEntity) {}
    virtual void OnCollisionExit(uint32_t otherEntity) {}
    virtual void OnTriggerEnter(uint32_t otherEntity) {}
    virtual void OnTriggerStay(uint32_t otherEntity) {}
    virtual void OnTriggerExit(uint32_t otherEntity) {}

    // UI events
    virtual void OnMouseEnter() {}
    virtual void OnMouseExit() {}
    virtual void OnMouseDown() {}
    virtual void OnMouseUp() {}

    // Scene events
    virtual void OnSceneLoaded() {}
    virtual void OnSceneUnloaded() {}

    // Serialization
    virtual void OnSerialize() {}
    virtual void OnDeserialize() {}

    // Properties
    uint32_t GetEntityID() const { return m_EntityID; }
    void SetEntityID(uint32_t id) { m_EntityID = id; }

    bool IsEnabled() const { return m_Enabled; }
    void SetEnabled(bool enabled) {
        if (m_Enabled != enabled) {
            m_Enabled = enabled;
            if (enabled)
                OnEnable();
            else
                OnDisable();
        }
    }

    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }

protected:
    uint32_t m_EntityID = 0;
    bool m_Enabled = true;
    std::string m_Name = "Script";
};

// ═══════════════════════════════════════════════════════════════
// ScriptInstance — Wrapper for a script attached to an entity
// ═══════════════════════════════════════════════════════════════
struct ScriptInstance {
    std::shared_ptr<Script> ScriptPtr;
    uint32_t EntityID = 0;
    std::string ClassName;
    bool Started = false;
    bool Destroyed = false;
};

// ═══════════════════════════════════════════════════════════════
// ScriptRegistry — Factory for creating scripts by class name
// ═══════════════════════════════════════════════════════════════
class ScriptRegistry {
public:
    static ScriptRegistry& Get() {
        static ScriptRegistry instance;
        return instance;
    }

    using CreateFunc = std::function<std::shared_ptr<Script>()>;

    void Register(const std::string& className, CreateFunc createFunc) { m_Registry[className] = createFunc; }

    std::shared_ptr<Script> Create(const std::string& className) {
        auto it = m_Registry.find(className);
        if (it != m_Registry.end()) {
            return it->second();
        }
        return nullptr;
    }

    bool IsRegistered(const std::string& className) const { return m_Registry.find(className) != m_Registry.end(); }

    std::vector<std::string> GetRegisteredClasses() const {
        std::vector<std::string> classes;
        for (const auto& [name, _] : m_Registry) {
            classes.push_back(name);
        }
        return classes;
    }

private:
    ScriptRegistry() = default;
    std::unordered_map<std::string, CreateFunc> m_Registry;
};

// ═══════════════════════════════════════════════════════════════
// ScriptEngine — Manages all script instances
// ═══════════════════════════════════════════════════════════════
class ScriptEngine {
public:
    static ScriptEngine& Get() {
        static ScriptEngine instance;
        return instance;
    }

    void Initialize();
    void Shutdown();

    // Instance management
    ScriptInstance& AddScript(uint32_t entityID, const std::string& className);
    void RemoveScript(uint32_t entityID, const std::string& className);
    void RemoveAllScripts(uint32_t entityID);
    ScriptInstance* GetScript(uint32_t entityID, const std::string& className);
    std::vector<ScriptInstance*> GetScripts(uint32_t entityID);

    // Lifecycle
    void OnStart();
    void OnUpdate(float deltaTime);
    void OnFixedUpdate(float fixedDeltaTime);
    void OnLateUpdate(float deltaTime);
    void OnStop();

    // Events
    void OnCollisionEnter(uint32_t entityA, uint32_t entityB);
    void OnCollisionExit(uint32_t entityA, uint32_t entityB);
    void OnTriggerEnter(uint32_t entityA, uint32_t entityB);
    void OnTriggerExit(uint32_t entityA, uint32_t entityB);

    // Stats
    struct Stats {
        uint32_t TotalInstances = 0;
        uint32_t ActiveInstances = 0;
        uint32_t RegisteredClasses = 0;
        float UpdateTimeMs = 0.0f;
    };
    const Stats& GetStats() const { return m_Stats; }

    // State
    bool IsRunning() const { return m_Running; }

private:
    ScriptEngine() = default;

    std::vector<ScriptInstance> m_Instances;
    bool m_Running = false;
    Stats m_Stats;
};

// ═══════════════════════════════════════════════════════════════
// PYENGINE_REGISTER_SCRIPT macro
// ═══════════════════════════════════════════════════════════════
#define PYENGINE_REGISTER_SCRIPT(ClassName)                                                                       \
    namespace {                                                                                                   \
    struct ClassName##_Registrar {                                                                                \
        ClassName##_Registrar() {                                                                                 \
            PyEngine::ScriptRegistry::Get().Register(#ClassName, []() { return std::make_shared<ClassName>(); }); \
        }                                                                                                         \
    };                                                                                                            \
    static ClassName##_Registrar s_##ClassName##_registrar;                                                       \
    }

}  // namespace PyEngine
