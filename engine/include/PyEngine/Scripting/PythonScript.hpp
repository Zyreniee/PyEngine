#pragma once

#include <string>

#include "PyEngine/Scripting/ScriptEngine.hpp"

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// PythonScript — A Script subclass that runs a .py file
// ═══════════════════════════════════════════════════════════════
// Supports two modes:
//
// 1) Class-based (recommended, like Unity MonoBehaviour):
//    class PlayerController:
//        def on_create(self):  ...
//        def on_update(self, dt):  ...
//
// 2) Module-level functions (simple scripts):
//    def on_create():  ...
//    def on_update(dt):  ...

class PythonScript : public Script {
public:
    PythonScript() = default;
    ~PythonScript() override = default;

    // Set the path to the .py script file
    void SetScriptPath(const std::string& path) { m_ScriptPath = path; }
    const std::string& GetScriptPath() const { return m_ScriptPath; }

    // Load (or reload) the Python module from m_ScriptPath
    bool LoadModule();

    // Hot-reload: destroy current state, reimport module, re-call lifecycle
    void Reload();

    // Get the discovered class name (if class-based)
    const std::string& GetClassName() const { return m_ClassName; }
    bool IsClassBased() const { return m_HasClassInstance; }

    // Script lifecycle overrides
    void OnCreate() override;
    void OnStart() override;
    void OnUpdate(float deltaTime) override;
    void OnFixedUpdate(float fixedDeltaTime) override;
    void OnLateUpdate(float deltaTime) override;
    void OnDestroy() override;

    // Physics callbacks
    void OnCollisionEnter(uint32_t otherEntity) override;
    void OnCollisionExit(uint32_t otherEntity) override;
    void OnTriggerEnter(uint32_t otherEntity) override;
    void OnTriggerExit(uint32_t otherEntity) override;

private:
    // Call a Python function by name with optional argument
    void CallPythonFunc(const std::string& funcName);
    void CallPythonFunc(const std::string& funcName, float arg);
    void CallPythonFunc(const std::string& funcName, uint32_t arg);

    std::string m_ScriptPath;
    std::string m_ClassName;
    bool m_ModuleLoaded = false;

    // Opaque handles — stored as void* to avoid pybind11 headers
    void* m_PyModule = nullptr;         // py::module_*
    void* m_PyClassInstance = nullptr;   // py::object*
    bool m_HasClassInstance = false;
};

}  // namespace PyEngine
