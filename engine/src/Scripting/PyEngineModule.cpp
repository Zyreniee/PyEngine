#include "PyEngine/Scripting/PyEngineModule.hpp"

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Platform/Input.hpp"
#include "PyEngine/Scripting/ScriptEngine.hpp"

namespace py = pybind11;

// ═══════════════════════════════════════════════════════════════
// Embedded Python Module: "pyengine"
//
// Unity-like Python API for PyEngine.
// Usage in .py scripts:
//   import pyengine
//   class PlayerController(pyengine.Component):
//       def on_update(self, dt):
//           pos = self.get_position()
//           if pyengine.Input.is_key_pressed(pyengine.KEY_W):
//               pos.z -= 5.0 * dt
//           self.set_position(pos)
// ═══════════════════════════════════════════════════════════════

PYBIND11_EMBEDDED_MODULE(pyengine, m) {
    m.doc() = "PyEngine — Python scripting API for the game engine";

    // ── Vec3 ─────────────────────────────────────────────────
    py::class_<glm::vec3>(m, "Vec3")
        .def(py::init<>())
        .def(py::init<float, float, float>())
        .def(py::init([](float v) { return glm::vec3(v); }))
        .def_readwrite("x", &glm::vec3::x)
        .def_readwrite("y", &glm::vec3::y)
        .def_readwrite("z", &glm::vec3::z)
        .def("__repr__", [](const glm::vec3& v) {
            return "Vec3(" + std::to_string(v.x) + ", " +
                   std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
        })
        .def("__add__", [](const glm::vec3& a, const glm::vec3& b) { return a + b; })
        .def("__sub__", [](const glm::vec3& a, const glm::vec3& b) { return a - b; })
        .def("__mul__", [](const glm::vec3& a, float s) { return a * s; })
        .def("__rmul__", [](const glm::vec3& a, float s) { return s * a; })
        .def("__neg__", [](const glm::vec3& a) { return -a; })
        .def("__eq__", [](const glm::vec3& a, const glm::vec3& b) { return a == b; })
        .def("length", [](const glm::vec3& v) { return glm::length(v); })
        .def("normalized", [](const glm::vec3& v) {
            float len = glm::length(v);
            return len > 0.0001f ? v / len : glm::vec3(0.0f);
        })
        .def("dot", [](const glm::vec3& a, const glm::vec3& b) { return glm::dot(a, b); })
        .def("cross", [](const glm::vec3& a, const glm::vec3& b) { return glm::cross(a, b); })
        .def("distance", [](const glm::vec3& a, const glm::vec3& b) { return glm::distance(a, b); })
        .def("lerp", [](const glm::vec3& a, const glm::vec3& b, float t) { return glm::mix(a, b, t); })
        .def_static("zero", []() { return glm::vec3(0.0f); })
        .def_static("one", []() { return glm::vec3(1.0f); })
        .def_static("up", []() { return glm::vec3(0.0f, 1.0f, 0.0f); })
        .def_static("down", []() { return glm::vec3(0.0f, -1.0f, 0.0f); })
        .def_static("forward", []() { return glm::vec3(0.0f, 0.0f, -1.0f); })
        .def_static("back", []() { return glm::vec3(0.0f, 0.0f, 1.0f); })
        .def_static("right", []() { return glm::vec3(1.0f, 0.0f, 0.0f); })
        .def_static("left", []() { return glm::vec3(-1.0f, 0.0f, 0.0f); });

    // ── Vec4 / Color ─────────────────────────────────────────
    py::class_<glm::vec4>(m, "Vec4")
        .def(py::init<>())
        .def(py::init<float, float, float, float>())
        .def_readwrite("x", &glm::vec4::x)
        .def_readwrite("y", &glm::vec4::y)
        .def_readwrite("z", &glm::vec4::z)
        .def_readwrite("w", &glm::vec4::w)
        .def("__repr__", [](const glm::vec4& v) {
            return "Vec4(" + std::to_string(v.x) + ", " + std::to_string(v.y) +
                   ", " + std::to_string(v.z) + ", " + std::to_string(v.w) + ")";
        })
        .def("__add__", [](const glm::vec4& a, const glm::vec4& b) { return a + b; })
        .def("__sub__", [](const glm::vec4& a, const glm::vec4& b) { return a - b; })
        .def("__mul__", [](const glm::vec4& a, float s) { return a * s; });

    // Color alias (same as Vec4 but with r,g,b,a names)
    m.def("Color", [](float r, float g, float b, float a) {
        return glm::vec4(r, g, b, a);
    }, py::arg("r") = 1.0f, py::arg("g") = 1.0f, py::arg("b") = 1.0f, py::arg("a") = 1.0f,
    "Create a color (RGBA)");

    // ── Logging ──────────────────────────────────────────────
    auto debug = m.def_submodule("Debug", "Debug utilities");
    debug.def("log", [](const std::string& msg) {
        PYENGINE_INFO("[Python] {}", msg);
    }, "Log an info message");
    debug.def("warn", [](const std::string& msg) {
        PYENGINE_WARN("[Python] {}", msg);
    }, "Log a warning");
    debug.def("error", [](const std::string& msg) {
        PYENGINE_ERROR("[Python] {}", msg);
    }, "Log an error");

    // Also keep top-level convenience functions
    m.def("log_info", [](const std::string& msg) {
        PYENGINE_INFO("[Python] {}", msg);
    });
    m.def("log_warn", [](const std::string& msg) {
        PYENGINE_WARN("[Python] {}", msg);
    });
    m.def("log_error", [](const std::string& msg) {
        PYENGINE_ERROR("[Python] {}", msg);
    });
    m.def("print", [](const std::string& msg) {
        PYENGINE_INFO("[Python] {}", msg);
    }, "Print to engine console");

    // ── Input (static class-like submodule) ──────────────────
    auto input = m.def_submodule("Input", "Input system");
    input.def("is_key_pressed", [](int keycode) -> bool {
        return PyEngine::Input::IsKeyPressed(keycode);
    }, py::arg("keycode"));
    input.def("is_key_down", [](int keycode) -> bool {
        return PyEngine::Input::IsKeyPressed(keycode);
    }, py::arg("keycode"));
    input.def("is_mouse_pressed", [](int button) -> bool {
        return PyEngine::Input::IsMouseButtonPressed(button);
    }, py::arg("button"));
    input.def("get_mouse_position", []() -> glm::vec3 {
        auto pos = PyEngine::Input::GetMousePosition();
        return glm::vec3(pos.x, pos.y, 0.0f);
    });
    input.def("get_mouse_x", []() -> float {
        return PyEngine::Input::GetMousePosition().x;
    });
    input.def("get_mouse_y", []() -> float {
        return PyEngine::Input::GetMousePosition().y;
    });

    // Also keep backward-compatible top-level input functions
    m.def("is_key_pressed", [](int keycode) -> bool {
        return PyEngine::Input::IsKeyPressed(keycode);
    }, py::arg("keycode"));
    m.def("is_mouse_pressed", [](int button) -> bool {
        return PyEngine::Input::IsMouseButtonPressed(button);
    }, py::arg("button"));
    m.def("get_mouse_x", []() -> float {
        return PyEngine::Input::GetMousePosition().x;
    });
    m.def("get_mouse_y", []() -> float {
        return PyEngine::Input::GetMousePosition().y;
    });

    // ── Key Constants (GLFW key codes) ───────────────────────
    // Letters
    for (int i = 0; i < 26; i++) {
        std::string name = "KEY_" + std::string(1, (char)('A' + i));
        m.attr(name.c_str()) = 65 + i;
    }
    // Numbers
    for (int i = 0; i < 10; i++) {
        std::string name = "KEY_" + std::to_string(i);
        m.attr(name.c_str()) = 48 + i;
    }
    // Special keys
    m.attr("KEY_SPACE") = 32;
    m.attr("KEY_ESCAPE") = 256;
    m.attr("KEY_ENTER") = 257;
    m.attr("KEY_TAB") = 258;
    m.attr("KEY_BACKSPACE") = 259;
    m.attr("KEY_INSERT") = 260;
    m.attr("KEY_DELETE") = 261;
    m.attr("KEY_RIGHT") = 262;
    m.attr("KEY_LEFT") = 263;
    m.attr("KEY_DOWN") = 264;
    m.attr("KEY_UP") = 265;
    m.attr("KEY_PAGE_UP") = 266;
    m.attr("KEY_PAGE_DOWN") = 267;
    m.attr("KEY_HOME") = 268;
    m.attr("KEY_END") = 269;
    m.attr("KEY_LEFT_SHIFT") = 340;
    m.attr("KEY_LEFT_CONTROL") = 341;
    m.attr("KEY_LEFT_ALT") = 342;
    m.attr("KEY_RIGHT_SHIFT") = 344;
    m.attr("KEY_RIGHT_CONTROL") = 345;
    m.attr("KEY_F1") = 290;
    m.attr("KEY_F2") = 291;
    m.attr("KEY_F3") = 292;
    m.attr("KEY_F4") = 293;
    m.attr("KEY_F5") = 294;
    m.attr("KEY_F6") = 295;
    m.attr("KEY_F7") = 296;
    m.attr("KEY_F8") = 297;
    m.attr("KEY_F9") = 298;
    m.attr("KEY_F10") = 299;
    m.attr("KEY_F11") = 300;
    m.attr("KEY_F12") = 301;

    // Mouse buttons
    m.attr("MOUSE_LEFT") = 0;
    m.attr("MOUSE_RIGHT") = 1;
    m.attr("MOUSE_MIDDLE") = 2;

    // ── Component base class (Python-side) ───────────────────
    // This is the base class that user scripts inherit from.
    // The C++ side (PythonScript) creates instances and calls
    // lifecycle methods on them.
    py::class_<PyEngine::Script, std::shared_ptr<PyEngine::Script>>(m, "Component")
        .def(py::init<>())
        .def("get_entity_id", &PyEngine::Script::GetEntityID)
        .def("is_enabled", &PyEngine::Script::IsEnabled)
        .def("set_enabled", &PyEngine::Script::SetEnabled)
        .def("get_name", &PyEngine::Script::GetName);

    // ── Math utilities ───────────────────────────────────────
    auto math = m.def_submodule("Math", "Math utilities");
    math.def("lerp", [](float a, float b, float t) { return a + (b - a) * t; });
    math.def("clamp", [](float v, float mn, float mx) {
        return std::max(mn, std::min(mx, v));
    });
    math.def("clamp01", [](float v) { return std::max(0.0f, std::min(1.0f, v)); });
    math.def("abs", [](float v) { return std::abs(v); });
    math.def("sign", [](float v) { return v > 0.0f ? 1.0f : (v < 0.0f ? -1.0f : 0.0f); });
    math.def("min", [](float a, float b) { return std::min(a, b); });
    math.def("max", [](float a, float b) { return std::max(a, b); });
    math.def("radians", [](float deg) { return glm::radians(deg); });
    math.def("degrees", [](float rad) { return glm::degrees(rad); });
    math.def("sin", [](float v) { return std::sin(v); });
    math.def("cos", [](float v) { return std::cos(v); });
    math.def("sqrt", [](float v) { return std::sqrt(v); });
    math.attr("PI") = 3.14159265358979323846f;
    math.attr("DEG2RAD") = 0.0174532925f;
    math.attr("RAD2DEG") = 57.2957795131f;
}

namespace PyEngine {

void RegisterPyEngineModule() {
    // The PYBIND11_EMBEDDED_MODULE macro automatically registers the module.
    // This function exists as a linkage anchor to ensure the module's
    // translation unit is included by the linker.
    PYENGINE_CORE_INFO("[PythonEngine] pyengine module registered");
}

}  // namespace PyEngine
