#include "PyEngine/Platform/Input.hpp"
#include <GLFW/glfw3.h>

namespace PyEngine {

GLFWwindow *Input::s_Window = nullptr;
glm::vec2 Input::s_LastMousePos = glm::vec2(0.0f);
glm::vec2 Input::s_MouseDelta = glm::vec2(0.0f);
bool Input::s_FirstMouse = true;

void Input::Init(GLFWwindow *window) {
  s_Window = window;
  double mouseX, mouseY;
  glfwGetCursorPos(s_Window, &mouseX, &mouseY);
  s_LastMousePos =
      glm::vec2(static_cast<float>(mouseX), static_cast<float>(mouseY));
}

void Input::Update() {
  // Calculate mouse delta
  glm::vec2 currentMousePos = GetMousePosition();

  if (s_FirstMouse) {
    s_LastMousePos = currentMousePos;
    s_FirstMouse = false;
  }

  s_MouseDelta = currentMousePos - s_LastMousePos;
  s_LastMousePos = currentMousePos;
}

bool Input::IsKeyPressed(int keycode) {
  int state = glfwGetKey(s_Window, keycode);
  return state == GLFW_PRESS;
}

bool Input::IsKeyDown(int keycode) {
  int state = glfwGetKey(s_Window, keycode);
  return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::IsKeyReleased(int keycode) {
  int state = glfwGetKey(s_Window, keycode);
  return state == GLFW_RELEASE;
}

bool Input::IsMouseButtonPressed(int button) {
  int state = glfwGetMouseButton(s_Window, button);
  return state == GLFW_PRESS;
}

glm::vec2 Input::GetMousePosition() {
  double mouseX, mouseY;
  glfwGetCursorPos(s_Window, &mouseX, &mouseY);
  return glm::vec2(static_cast<float>(mouseX), static_cast<float>(mouseY));
}

glm::vec2 Input::GetMouseDelta() { return s_MouseDelta; }

void Input::SetCursorMode(bool disabled) {
  glfwSetInputMode(s_Window, GLFW_CURSOR,
                   disabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

} // namespace PyEngine
