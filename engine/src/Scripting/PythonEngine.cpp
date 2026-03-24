#include "PyEngine/Scripting/PythonEngine.hpp"

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

#include <filesystem>

#include "PyEngine/Core/Log.hpp"

namespace py = pybind11;

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// PythonEngine::Impl — holds the actual pybind11 interpreter
// ═══════════════════════════════════════════════════════════════
struct PythonEngine::Impl {
    std::unique_ptr<py::scoped_interpreter> Interpreter;
};

PythonEngine::PythonEngine() = default;
PythonEngine::~PythonEngine() = default;

void PythonEngine::Initialize() {
    if (m_Initialized) return;

    try {
        m_Impl = std::make_unique<Impl>();
        m_Impl->Interpreter = std::make_unique<py::scoped_interpreter>();

        // Add the project's scripts directory to sys.path
        py::module_ sys = py::module_::import("sys");
        py::list path = sys.attr("path").cast<py::list>();

        // Add common script paths
        auto cwd = std::filesystem::current_path();
        path.append(cwd.string());
        path.append((cwd / "assets" / "scripts").string());

        m_Initialized = true;
        m_LastError.clear();

        PYENGINE_CORE_INFO("[PythonEngine] Python {} interpreter initialized",
                           py::str(sys.attr("version")).cast<std::string>());
    } catch (const py::error_already_set& e) {
        m_LastError = e.what();
        PYENGINE_CORE_ERROR("[PythonEngine] Failed to initialize Python: {}", m_LastError);
    } catch (const std::exception& e) {
        m_LastError = e.what();
        PYENGINE_CORE_ERROR("[PythonEngine] Failed to initialize Python: {}", m_LastError);
    }
}

void PythonEngine::Shutdown() {
    if (!m_Initialized) return;

    try {
        m_Impl.reset();
        m_Initialized = false;
        PYENGINE_CORE_INFO("[PythonEngine] Python interpreter shut down");
    } catch (const std::exception& e) {
        PYENGINE_CORE_ERROR("[PythonEngine] Error during shutdown: {}", e.what());
    }
}

bool PythonEngine::ExecuteFile(const std::string& filepath) {
    if (!m_Initialized) {
        m_LastError = "Python interpreter not initialized";
        return false;
    }

    try {
        py::eval_file(filepath);
        m_LastError.clear();
        return true;
    } catch (const py::error_already_set& e) {
        m_LastError = e.what();
        PYENGINE_CORE_ERROR("[PythonEngine] Python error in '{}': {}", filepath, m_LastError);
        return false;
    }
}

bool PythonEngine::ExecuteString(const std::string& code) {
    if (!m_Initialized) {
        m_LastError = "Python interpreter not initialized";
        return false;
    }

    try {
        py::exec(code);
        m_LastError.clear();
        return true;
    } catch (const py::error_already_set& e) {
        m_LastError = e.what();
        PYENGINE_CORE_ERROR("[PythonEngine] Python error: {}", m_LastError);
        return false;
    }
}

void PythonEngine::AddToPath(const std::string& directory) {
    if (!m_Initialized) return;

    try {
        py::module_ sys = py::module_::import("sys");
        py::list path = sys.attr("path").cast<py::list>();
        path.append(directory);
    } catch (const py::error_already_set& e) {
        PYENGINE_CORE_ERROR("[PythonEngine] Failed to add path '{}': {}", directory, e.what());
    }
}

}  // namespace PyEngine
