#pragma once

#include <string>

#include "PyEngine/Scripting/ScriptEngine.hpp"

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// PythonScript — A Script subclass that runs a .py file
// ═══════════════════════════════════════════════════════════════
// Each PythonScript loads a .py file and calls:
//   on_create(), on_start(), on_update(dt), on_destroy()
// functions defined in that Python file.

class PythonScript : public Script {
public:
    PythonScript() = default;
    ~PythonScript() override = default;

    // Set the path to the .py script file
    void SetScriptPath(const std::string& path) { m_ScriptPath = path; }
    const std::string& GetScriptPath() const { return m_ScriptPath; }

    // Load (or reload) the Python module from m_ScriptPath
    bool LoadModule();

    // Script lifecycle overrides
    void OnCreate() override;
    void OnStart() override;
    void OnUpdate(float deltaTime) override;
    void OnFixedUpdate(float fixedDeltaTime) override;
    void OnDestroy() override;

private:
    // Call a Python function by name if it exists, with optional float arg
    void CallPythonFunc(const std::string& funcName);
    void CallPythonFunc(const std::string& funcName, float arg);

    std::string m_ScriptPath;
    bool m_ModuleLoaded = false;

    // Opaque handle — we store a py::module via void* to avoid
    // pybind11 headers in this header
    void* m_PyModule = nullptr;
};

}  // namespace PyEngine
