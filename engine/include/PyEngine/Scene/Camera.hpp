#pragma once

#include <glm/glm.hpp>

namespace PyEngine {

class Input;

class Camera {
public:
  Camera(float fov = 45.0f, float aspect = 16.0f / 9.0f, float near = 0.1f,
         float far = 1000.0f);

  void OnUpdate(float deltaTime);

  const glm::mat4 &GetProjection() const { return m_Projection; }
  const glm::mat4 &GetView() const { return m_View; }
  const glm::vec3 &GetPosition() const { return m_Position; }
  const glm::vec3 &GetForward() const { return m_Forward; }

  void SetPosition(const glm::vec3 &position) {
    m_Position = position;
    UpdateView();
  }
  void SetProjection(float fov, float aspect, float near, float far);

private:
  void UpdateView();
  void ProcessInput(float deltaTime);

private:
  glm::mat4 m_Projection;
  glm::mat4 m_View;

  glm::vec3 m_Position{0.0f, 2.0f, 5.0f};
  glm::vec3 m_Forward{0.0f, 0.0f, -1.0f};
  glm::vec3 m_Up{0.0f, 1.0f, 0.0f};
  glm::vec3 m_Right{1.0f, 0.0f, 0.0f};

  float m_Yaw = -90.0f;
  float m_Pitch = 0.0f;
  float m_MoveSpeed = 5.0f;
  float m_LookSpeed = 0.1f;

  bool m_FirstMouse = true;
};

} // namespace PyEngine
