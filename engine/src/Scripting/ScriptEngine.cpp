#include "PyEngine/Scripting/ScriptEngine.hpp"

#include <algorithm>
#include <chrono>

namespace PyEngine {

void ScriptEngine::Initialize() {
    m_Instances.clear();
    m_Running = false;
    m_Stats = {};
}

void ScriptEngine::Shutdown() {
    OnStop();
    m_Instances.clear();
    m_Stats = {};
}

ScriptInstance& ScriptEngine::AddScript(uint32_t entityID, const std::string& className) {
    auto script = ScriptRegistry::Get().Create(className);

    ScriptInstance instance;
    instance.EntityID = entityID;
    instance.ClassName = className;
    instance.ScriptPtr = script;
    instance.Started = false;

    if (script) {
        script->SetEntityID(entityID);
        script->SetName(className);
        script->OnCreate();
    }

    m_Instances.push_back(instance);
    return m_Instances.back();
}

void ScriptEngine::RemoveScript(uint32_t entityID, const std::string& className) {
    for (auto it = m_Instances.begin(); it != m_Instances.end(); ++it) {
        if (it->EntityID == entityID && it->ClassName == className) {
            if (it->ScriptPtr)
                it->ScriptPtr->OnDestroy();
            m_Instances.erase(it);
            return;
        }
    }
}

void ScriptEngine::RemoveAllScripts(uint32_t entityID) {
    for (auto& inst : m_Instances) {
        if (inst.EntityID == entityID && inst.ScriptPtr) {
            inst.ScriptPtr->OnDestroy();
            inst.Destroyed = true;
        }
    }
    m_Instances.erase(std::remove_if(m_Instances.begin(), m_Instances.end(),
                                     [entityID](const ScriptInstance& i) { return i.EntityID == entityID; }),
                      m_Instances.end());
}

ScriptInstance* ScriptEngine::GetScript(uint32_t entityID, const std::string& className) {
    for (auto& inst : m_Instances) {
        if (inst.EntityID == entityID && inst.ClassName == className)
            return &inst;
    }
    return nullptr;
}

std::vector<ScriptInstance*> ScriptEngine::GetScripts(uint32_t entityID) {
    std::vector<ScriptInstance*> result;
    for (auto& inst : m_Instances) {
        if (inst.EntityID == entityID)
            result.push_back(&inst);
    }
    return result;
}

void ScriptEngine::OnStart() {
    m_Running = true;
    for (auto& inst : m_Instances) {
        if (inst.ScriptPtr && inst.ScriptPtr->IsEnabled() && !inst.Started) {
            inst.ScriptPtr->OnStart();
            inst.Started = true;
        }
    }
}

void ScriptEngine::OnUpdate(float deltaTime) {
    if (!m_Running)
        return;

    auto start = std::chrono::high_resolution_clock::now();

    m_Stats.TotalInstances = static_cast<uint32_t>(m_Instances.size());
    m_Stats.ActiveInstances = 0;
    m_Stats.RegisteredClasses = static_cast<uint32_t>(ScriptRegistry::Get().GetRegisteredClasses().size());

    for (auto& inst : m_Instances) {
        if (!inst.ScriptPtr || !inst.ScriptPtr->IsEnabled())
            continue;

        if (!inst.Started) {
            inst.ScriptPtr->OnStart();
            inst.Started = true;
        }

        inst.ScriptPtr->OnUpdate(deltaTime);
        m_Stats.ActiveInstances++;
    }

    auto end = std::chrono::high_resolution_clock::now();
    m_Stats.UpdateTimeMs = std::chrono::duration<float, std::milli>(end - start).count();
}

void ScriptEngine::OnFixedUpdate(float fixedDeltaTime) {
    if (!m_Running)
        return;

    for (auto& inst : m_Instances) {
        if (!inst.ScriptPtr || !inst.ScriptPtr->IsEnabled())
            continue;
        inst.ScriptPtr->OnFixedUpdate(fixedDeltaTime);
    }
}

void ScriptEngine::OnLateUpdate(float deltaTime) {
    if (!m_Running)
        return;

    for (auto& inst : m_Instances) {
        if (!inst.ScriptPtr || !inst.ScriptPtr->IsEnabled())
            continue;
        inst.ScriptPtr->OnLateUpdate(deltaTime);
    }
}

void ScriptEngine::OnStop() {
    m_Running = false;

    for (auto& inst : m_Instances) {
        if (inst.ScriptPtr && !inst.Destroyed) {
            inst.ScriptPtr->OnDestroy();
            inst.Destroyed = true;
        }
        inst.Started = false;
    }
}

void ScriptEngine::OnCollisionEnter(uint32_t entityA, uint32_t entityB) {
    for (auto& inst : m_Instances) {
        if (!inst.ScriptPtr || !inst.ScriptPtr->IsEnabled())
            continue;
        if (inst.EntityID == entityA)
            inst.ScriptPtr->OnCollisionEnter(entityB);
        if (inst.EntityID == entityB)
            inst.ScriptPtr->OnCollisionEnter(entityA);
    }
}

void ScriptEngine::OnCollisionExit(uint32_t entityA, uint32_t entityB) {
    for (auto& inst : m_Instances) {
        if (!inst.ScriptPtr || !inst.ScriptPtr->IsEnabled())
            continue;
        if (inst.EntityID == entityA)
            inst.ScriptPtr->OnCollisionExit(entityB);
        if (inst.EntityID == entityB)
            inst.ScriptPtr->OnCollisionExit(entityA);
    }
}

void ScriptEngine::OnTriggerEnter(uint32_t entityA, uint32_t entityB) {
    for (auto& inst : m_Instances) {
        if (!inst.ScriptPtr || !inst.ScriptPtr->IsEnabled())
            continue;
        if (inst.EntityID == entityA)
            inst.ScriptPtr->OnTriggerEnter(entityB);
        if (inst.EntityID == entityB)
            inst.ScriptPtr->OnTriggerEnter(entityA);
    }
}

void ScriptEngine::OnTriggerExit(uint32_t entityA, uint32_t entityB) {
    for (auto& inst : m_Instances) {
        if (!inst.ScriptPtr || !inst.ScriptPtr->IsEnabled())
            continue;
        if (inst.EntityID == entityA)
            inst.ScriptPtr->OnTriggerExit(entityB);
        if (inst.EntityID == entityB)
            inst.ScriptPtr->OnTriggerExit(entityA);
    }
}

}  // namespace PyEngine
