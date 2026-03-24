#pragma once

#include <memory>
#include <string>

// Forward declare pybind11 types to avoid header pollution
namespace pybind11 {
class scoped_interpreter;
class module_;
class object;
}  // namespace pybind11

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// PythonEngine — Manages the embedded Python interpreter
// ═══════════════════════════════════════════════════════════════
class PythonEngine {
public:
    static PythonEngine& Get() {
        static PythonEngine instance;
        return instance;
    }

    void Initialize();
    void Shutdown();

    // Execute a Python file (returns true on success)
    bool ExecuteFile(const std::string& filepath);

    // Execute a Python string (returns true on success)
    bool ExecuteString(const std::string& code);

    // Get the last Python error traceback
    const std::string& GetLastError() const { return m_LastError; }

    // Check if interpreter is active
    bool IsInitialized() const { return m_Initialized; }

    // Add a directory to Python's sys.path
    void AddToPath(const std::string& directory);

private:
    PythonEngine();
    ~PythonEngine();

    bool m_Initialized = false;
    std::string m_LastError;

    // The interpreter is managed via raw pointer to avoid pybind11
    // header inclusion in this header file
    struct Impl;
    std::unique_ptr<Impl> m_Impl;
};

}  // namespace PyEngine
