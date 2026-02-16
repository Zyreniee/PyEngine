#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "PyEngine/Core/Log.hpp"

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// ISystem — Interface for all systems
// ═══════════════════════════════════════════════════════════════
class Scene;

class ISystem {
public:
    virtual ~ISystem() = default;

    virtual void OnInit() {}
    virtual void OnShutdown() {}
    virtual void OnUpdate(float deltaTime, Scene* scene) {}
    virtual void OnFixedUpdate(float fixedDeltaTime, Scene* scene) {}
    virtual void OnRender(Scene* scene) {}
    virtual void OnImGuiRender() {}

    virtual const char* GetName() const = 0;

    void SetEnabled(bool enabled) { m_Enabled = enabled; }
    bool IsEnabled() const { return m_Enabled; }

protected:
    bool m_Enabled = true;
};

// ═══════════════════════════════════════════════════════════════
// SystemManager — Manages system lifecycle
// ═══════════════════════════════════════════════════════════════
class SystemManager {
public:
    static SystemManager& Get() {
        static SystemManager instance;
        return instance;
    }

    template <typename T, typename... Args>
    std::shared_ptr<T> AddSystem(Args&&... args) {
        auto system = std::make_shared<T>(std::forward<Args>(args)...);
        m_Systems.push_back(system);
        system->OnInit();
        PYENGINE_CORE_INFO("System initialized: {}", system->GetName());
        return system;
    }

    void RemoveSystem(const std::string& name) {
        auto it = std::remove_if(m_Systems.begin(), m_Systems.end(), [&](const std::shared_ptr<ISystem>& sys) {
            if (sys->GetName() == name) {
                sys->OnShutdown();
                return true;
            }
            return false;
        });
        m_Systems.erase(it, m_Systems.end());
    }

    void Shutdown() {
        // Shutdown in reverse order
        for (auto it = m_Systems.rbegin(); it != m_Systems.rend(); ++it) {
            (*it)->OnShutdown();
        }
        m_Systems.clear();
    }

    void Update(float deltaTime, Scene* scene) {
        for (auto& sys : m_Systems) {
            if (sys->IsEnabled())
                sys->OnUpdate(deltaTime, scene);
        }
    }

    void FixedUpdate(float fixedDeltaTime, Scene* scene) {
        for (auto& sys : m_Systems) {
            if (sys->IsEnabled())
                sys->OnFixedUpdate(fixedDeltaTime, scene);
        }
    }

    void Render(Scene* scene) {
        for (auto& sys : m_Systems) {
            if (sys->IsEnabled())
                sys->OnRender(scene);
        }
    }

    void OnImGuiRender() {
        for (auto& sys : m_Systems) {
            if (sys->IsEnabled())
                sys->OnImGuiRender();
        }
    }

private:
    SystemManager() = default;
    std::vector<std::shared_ptr<ISystem>> m_Systems;
};

}  // namespace PyEngine
