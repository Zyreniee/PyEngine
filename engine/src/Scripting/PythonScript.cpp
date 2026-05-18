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

        // Try to find a class that inherits from pyengine.Component
        // Convention: class name matches module name (PascalCase) or any class with lifecycle methods
        m_HasClassInstance = false;
        if (m_PyClassInstance) {
            delete static_cast<py::object*>(m_PyClassInstance);
            m_PyClassInstance = nullptr;
        }

        // Search for a suitable script class in the module
        py::dict moduleDict = mod.attr("__dict__");
        for (auto& item : moduleDict) {
            py::object obj = py::reinterpret_borrow<py::object>(item.second);
            if (py::isinstance<py::type>(obj)) {
                // Check if class has any lifecycle method
                bool hasLifecycle = py::hasattr(obj, "on_create") ||
                                    py::hasattr(obj, "on_start") ||
                                    py::hasattr(obj, "on_update") ||
                                    py::hasattr(obj, "on_destroy");
                if (hasLifecycle) {
                    // Instantiate the class
                    py::object instance = obj();
                    m_PyClassInstance = new py::object(instance);
                    m_HasClassInstance = true;
                    m_ClassName = py::str(item.first).cast<std::string>();
                    PYENGINE_CORE_INFO("[PythonScript] Found script class '{}' in '{}'",
                                       m_ClassName, moduleName);
                    break;
                }
            }
        }

        if (!m_HasClassInstance) {
            PYENGINE_CORE_INFO("[PythonScript] No class found, using module-level functions for '{}'",
                               moduleName);
        }

        PYENGINE_CORE_INFO("[PythonScript] Loaded module '{}'", moduleName);
        return true;
    } catch (const py::error_already_set& e) {
        PYENGINE_CORE_ERROR("[PythonScript] Failed to load '{}': {}", m_ScriptPath, e.what());
        m_ModuleLoaded = false;
        return false;
    }
}

void PythonScript::CallPythonFunc(const std::string& funcName) {
    if (!m_ModuleLoaded) return;

    try {
        if (m_HasClassInstance && m_PyClassInstance) {
            // Call method on class instance
            auto* inst = static_cast<py::object*>(m_PyClassInstance);
            if (py::hasattr(*inst, funcName.c_str())) {
                inst->attr(funcName.c_str())();
            }
        } else if (m_PyModule) {
            // Call module-level function
            auto* mod = static_cast<py::module_*>(m_PyModule);
            if (py::hasattr(*mod, funcName.c_str())) {
                mod->attr(funcName.c_str())();
            }
        }
    } catch (const py::error_already_set& e) {
        PYENGINE_CORE_ERROR("[PythonScript] Error in {}(): {}", funcName, e.what());
    }
}

void PythonScript::CallPythonFunc(const std::string& funcName, float arg) {
    if (!m_ModuleLoaded) return;

    try {
        if (m_HasClassInstance && m_PyClassInstance) {
            auto* inst = static_cast<py::object*>(m_PyClassInstance);
            if (py::hasattr(*inst, funcName.c_str())) {
                inst->attr(funcName.c_str())(arg);
            }
        } else if (m_PyModule) {
            auto* mod = static_cast<py::module_*>(m_PyModule);
            if (py::hasattr(*mod, funcName.c_str())) {
                mod->attr(funcName.c_str())(arg);
            }
        }
    } catch (const py::error_already_set& e) {
        PYENGINE_CORE_ERROR("[PythonScript] Error in {}(): {}", funcName, e.what());
    }
}

void PythonScript::CallPythonFunc(const std::string& funcName, uint32_t arg) {
    if (!m_ModuleLoaded) return;

    try {
        if (m_HasClassInstance && m_PyClassInstance) {
            auto* inst = static_cast<py::object*>(m_PyClassInstance);
            if (py::hasattr(*inst, funcName.c_str())) {
                inst->attr(funcName.c_str())(arg);
            }
        } else if (m_PyModule) {
            auto* mod = static_cast<py::module_*>(m_PyModule);
            if (py::hasattr(*mod, funcName.c_str())) {
                mod->attr(funcName.c_str())(arg);
            }
        }
    } catch (const py::error_already_set& e) {
        PYENGINE_CORE_ERROR("[PythonScript] Error in {}(): {}", funcName, e.what());
    }
}

void PythonScript::OnCreate() {
    if (!m_ModuleLoaded) {
        LoadModule();
    }

    // Inject entity_id into class instance
    if (m_HasClassInstance && m_PyClassInstance) {
        try {
            auto* inst = static_cast<py::object*>(m_PyClassInstance);
            inst->attr("entity_id") = py::cast(m_EntityID);
        } catch (...) {}
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

void PythonScript::OnLateUpdate(float deltaTime) {
    CallPythonFunc("on_late_update", deltaTime);
}

void PythonScript::OnDestroy() {
    CallPythonFunc("on_destroy");

    if (m_PyClassInstance) {
        delete static_cast<py::object*>(m_PyClassInstance);
        m_PyClassInstance = nullptr;
    }
    m_HasClassInstance = false;

    if (m_PyModule) {
        delete static_cast<py::module_*>(m_PyModule);
        m_PyModule = nullptr;
    }
    m_ModuleLoaded = false;
}

void PythonScript::OnCollisionEnter(uint32_t otherEntity) {
    CallPythonFunc("on_collision_enter", otherEntity);
}

void PythonScript::OnCollisionExit(uint32_t otherEntity) {
    CallPythonFunc("on_collision_exit", otherEntity);
}

void PythonScript::OnTriggerEnter(uint32_t otherEntity) {
    CallPythonFunc("on_trigger_enter", otherEntity);
}

void PythonScript::OnTriggerExit(uint32_t otherEntity) {
    CallPythonFunc("on_trigger_exit", otherEntity);
}

void PythonScript::Reload() {
    PYENGINE_CORE_INFO("[PythonScript] Hot-reloading '{}'", m_ScriptPath);
    OnDestroy();
    LoadModule();
    OnCreate();
    OnStart();
}

}  // namespace PyEngine
