#include "PyEngine/Scene/Camera.hpp"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "PyEngine/Platform/Input.hpp"

namespace PyEngine {

Camera::Camera(float fov, float aspect, float near, float far) {
    SetProjection(fov, aspect, near, far);
    UpdateView();
}

void Camera::OnUpdate(float deltaTime) {
    ProcessInput(deltaTime);
    UpdateView();
}

void Camera::SetProjection(float fov, float aspect, float near, float far) {
    m_Projection = glm::perspective(glm::radians(fov), aspect, near, far);
}

void Camera::UpdateView() {
    m_View = glm::lookAt(m_Position, m_Position + m_Forward, m_Up);
}

void Camera::ProcessInput(float deltaTime) {
    // Movement
    glm::vec3 velocity(0.0f);

    if (Input::IsKeyPressed(GLFW_KEY_W))
        velocity += m_Forward;
    if (Input::IsKeyPressed(GLFW_KEY_S))
        velocity -= m_Forward;
    if (Input::IsKeyPressed(GLFW_KEY_A))
        velocity -= m_Right;
    if (Input::IsKeyPressed(GLFW_KEY_D))
        velocity += m_Right;
    if (Input::IsKeyPressed(GLFW_KEY_SPACE))
        velocity += m_Up;
    if (Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
        velocity -= m_Up;

    if (glm::length(velocity) > 0.0f) {
        m_Position += glm::normalize(velocity) * m_MoveSpeed * deltaTime;
    }

    // Mouse look
    glm::vec2 mouseDelta = Input::GetMouseDelta();

    if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        m_Yaw += mouseDelta.x * m_LookSpeed;
        m_Pitch -= mouseDelta.y * m_LookSpeed;

        m_Pitch = std::clamp(m_Pitch, -89.0f, 89.0f);

        glm::vec3 forward;
        forward.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        forward.y = sin(glm::radians(m_Pitch));
        forward.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        m_Forward = glm::normalize(forward);

        m_Right = glm::normalize(glm::cross(m_Forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        m_Up = glm::normalize(glm::cross(m_Right, m_Forward));
    }
}

}  // namespace PyEngine
