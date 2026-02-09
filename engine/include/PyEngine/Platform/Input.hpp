#pragma once

#include <glm/glm.hpp>

struct GLFWwindow;

namespace PyEngine {

class Input {
public:
    static void Init(GLFWwindow* window);
    static void Update();

    // Keyboard
    static bool IsKeyPressed(int keycode);
    static bool IsKeyDown(int keycode);
    static bool IsKeyReleased(int keycode);

    // Mouse
    static bool IsMouseButtonPressed(int button);
    static glm::vec2 GetMousePosition();
    static glm::vec2 GetMouseDelta();
    static void SetCursorMode(bool disabled);

private:
    static GLFWwindow* s_Window;
    static glm::vec2 s_LastMousePosition;
    static glm::vec2 s_MouseDelta;
    static bool s_FirstMouse;
};

}  // namespace PyEngine
