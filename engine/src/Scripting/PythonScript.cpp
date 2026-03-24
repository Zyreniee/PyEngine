#include "PyEngine/Scripting/PythonScript.hpp"

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

#include <filesystem>

#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Scripting/PythonEngine.hpp"

namespace py = pybind11;

namespace PyEngine {

bool PythonScript::LoadModule() {
    if (!PythonEngine::Get().IsInitialized()) {
        PYENGINE_CORE_ERROR("[PythonScript] Cannot load '{}' — Python not initialized", m_ScriptPath);
        return false;
    }

    if (m_ScriptPath.empty()) {
        PYENGINE_CORE_WARN("[PythonScript] No script path set");
        return false;
    }

    try {
        // Extract module name from file path (e.g., "player_controller" from "player_controller.py")
        std::filesystem::path p(m_ScriptPath);
        std::string moduleName = p.stem().string();

        // Add the script's directory to sys.path
        std::string scriptDir = p.parent_path().string();
        if (!scriptDir.empty()) {
            PythonEngine::Get().AddToPath(scriptDir);
        }

        // Import (or reimport) the module
        py::module_ mod = py::module_::import(moduleName.c_str());
        mod.reload();  // Support hot-reload

        // Store as void* to avoid pybind11 in the header
        if (m_PyModule) {
            delete static_cast<py::module_*>(m_PyModule);
        }
        m_PyModule = new py::module_(mod);
        m_ModuleLoaded = true;

        PYENGINE_CORE_INFO("[PythonScript] Loaded module '{}'", moduleName);
        return true;
    } catch (const py::error_already_set& e) {
        PYENGINE_CORE_ERROR("[PythonScript] Failed to load '{}': {}", m_ScriptPath, e.what());
        m_ModuleLoaded = false;
        return false;
    }
}

void PythonScript::CallPythonFunc(const std::string& funcName) {
    if (!m_ModuleLoaded || !m_PyModule) return;

    try {
        auto* mod = static_cast<py::module_*>(m_PyModule);
        if (py::hasattr(*mod, funcName.c_str())) {
            mod->attr(funcName.c_str())();
        }
    } catch (const py::error_already_set& e) {
        PYENGINE_CORE_ERROR("[PythonScript] Error in {}(): {}", funcName, e.what());
    }
}

void PythonScript::CallPythonFunc(const std::string& funcName, float arg) {
    if (!m_ModuleLoaded || !m_PyModule) return;

    try {
        auto* mod = static_cast<py::module_*>(m_PyModule);
        if (py::hasattr(*mod, funcName.c_str())) {
            mod->attr(funcName.c_str())(arg);
        }
    } catch (const py::error_already_set& e) {
        PYENGINE_CORE_ERROR("[PythonScript] Error in {}(): {}", funcName, e.what());
    }
}

void PythonScript::OnCreate() {
    if (!m_ModuleLoaded) {
        LoadModule();
    }
    CallPythonFunc("on_create");
}

void PythonScript::OnStart() {
    if (!m_ModuleLoaded) {
        LoadModule();
    }
    CallPythonFunc("on_start");
}

void PythonScript::OnUpdate(float deltaTime) {
    CallPythonFunc("on_update", deltaTime);
}

void PythonScript::OnFixedUpdate(float fixedDeltaTime) {
    CallPythonFunc("on_fixed_update", fixedDeltaTime);
}

void PythonScript::OnDestroy() {
    CallPythonFunc("on_destroy");

    if (m_PyModule) {
        delete static_cast<py::module_*>(m_PyModule);
        m_PyModule = nullptr;
    }
    m_ModuleLoaded = false;
}

}  // namespace PyEngine
