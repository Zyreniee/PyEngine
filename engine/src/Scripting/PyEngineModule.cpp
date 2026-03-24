#include "PyEngine/Scripting/PyEngineModule.hpp"

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Platform/Input.hpp"

namespace py = pybind11;

// ═══════════════════════════════════════════════════════════════
// Embedded Python Module: "pyengine"
//
// This is the Python API exposed to game scripts:
//   import pyengine
//   pyengine.log_info("Hello from Python!")
//   if pyengine.is_key_pressed(pyengine.KEY_W):
//       ...
// ═══════════════════════════════════════════════════════════════

PYBIND11_EMBEDDED_MODULE(pyengine, m) {
    m.doc() = "PyEngine — Python scripting API for the game engine";

    // ── Logging ──────────────────────────────────────────────
    m.def("log_info", [](const std::string& msg) {
        PYENGINE_INFO("[Python] {}", msg);
    }, "Log an info message to the engine console");

    m.def("log_warn", [](const std::string& msg) {
        PYENGINE_WARN("[Python] {}", msg);
    }, "Log a warning message to the engine console");

    m.def("log_error", [](const std::string& msg) {
        PYENGINE_ERROR("[Python] {}", msg);
    }, "Log an error message to the engine console");

    // ── Input ────────────────────────────────────────────────
    m.def("is_key_pressed", [](int keycode) -> bool {
        return PyEngine::Input::IsKeyPressed(keycode);
    }, py::arg("keycode"), "Check if a keyboard key is currently pressed");

    m.def("is_mouse_pressed", [](int button) -> bool {
        return PyEngine::Input::IsMouseButtonPressed(button);
    }, py::arg("button"), "Check if a mouse button is currently pressed");

    m.def("get_mouse_x", []() -> float {
        return PyEngine::Input::GetMousePosition().x;
    }, "Get current mouse X position");

    m.def("get_mouse_y", []() -> float {
        return PyEngine::Input::GetMousePosition().y;
    }, "Get current mouse Y position");

    // ── Key Constants (matching GLFW key codes) ──────────────
    // Letters
    m.attr("KEY_A") = 65;
    m.attr("KEY_B") = 66;
    m.attr("KEY_C") = 67;
    m.attr("KEY_D") = 68;
    m.attr("KEY_E") = 69;
    m.attr("KEY_F") = 70;
    m.attr("KEY_G") = 71;
    m.attr("KEY_H") = 72;
    m.attr("KEY_I") = 73;
    m.attr("KEY_J") = 74;
    m.attr("KEY_K") = 75;
    m.attr("KEY_L") = 76;
    m.attr("KEY_M") = 77;
    m.attr("KEY_N") = 78;
    m.attr("KEY_O") = 79;
    m.attr("KEY_P") = 80;
    m.attr("KEY_Q") = 81;
    m.attr("KEY_R") = 82;
    m.attr("KEY_S") = 83;
    m.attr("KEY_T") = 84;
    m.attr("KEY_U") = 85;
    m.attr("KEY_V") = 86;
    m.attr("KEY_W") = 87;
    m.attr("KEY_X") = 88;
    m.attr("KEY_Y") = 89;
    m.attr("KEY_Z") = 90;

    // Special keys
    m.attr("KEY_SPACE") = 32;
    m.attr("KEY_ESCAPE") = 256;
    m.attr("KEY_ENTER") = 257;
    m.attr("KEY_TAB") = 258;
    m.attr("KEY_RIGHT") = 262;
    m.attr("KEY_LEFT") = 263;
    m.attr("KEY_DOWN") = 264;
    m.attr("KEY_UP") = 265;
    m.attr("KEY_LEFT_SHIFT") = 340;
    m.attr("KEY_LEFT_CONTROL") = 341;
    m.attr("KEY_LEFT_ALT") = 342;

    // Mouse buttons
    m.attr("MOUSE_LEFT") = 0;
    m.attr("MOUSE_RIGHT") = 1;
    m.attr("MOUSE_MIDDLE") = 2;

    // ── Vec3 helper class ────────────────────────────────────
    py::class_<glm::vec3>(m, "Vec3")
        .def(py::init<>())
        .def(py::init<float, float, float>())
        .def_readwrite("x", &glm::vec3::x)
        .def_readwrite("y", &glm::vec3::y)
        .def_readwrite("z", &glm::vec3::z)
        .def("__repr__", [](const glm::vec3& v) {
            return "Vec3(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
        })
        .def("__add__", [](const glm::vec3& a, const glm::vec3& b) { return a + b; })
        .def("__sub__", [](const glm::vec3& a, const glm::vec3& b) { return a - b; })
        .def("__mul__", [](const glm::vec3& a, float s) { return a * s; })
        .def("length", [](const glm::vec3& v) { return glm::length(v); });
}

namespace PyEngine {

void RegisterPyEngineModule() {
    // The PYBIND11_EMBEDDED_MODULE macro automatically registers the module.
    // This function exists as a linkage anchor to ensure the module's
    // translation unit is included by the linker.
    PYENGINE_CORE_INFO("[PythonEngine] pyengine module registered");
}

}  // namespace PyEngine
