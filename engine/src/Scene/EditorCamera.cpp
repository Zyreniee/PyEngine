#include "PyEngine/Scene/EditorCamera.hpp"

#include <algorithm>
#include <cmath>

namespace PyEngine {

EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
    : m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip) {
    UpdateProjectionMatrix();
    UpdateViewMatrix();
}

void EditorCamera::OnUpdate(float deltaTime) {
    if (m_RightMouseDown) {
        // FPS-style WASD movement (only when right-click is held, like Unity)
        float speed = m_MoveSpeed * (m_Boost ? 4.0f : 1.0f);
        float velocity = speed * deltaTime;

        glm::vec3 forward = GetForwardDirection();
        glm::vec3 right = GetRightDirection();
        glm::vec3 worldUp = {0.0f, 1.0f, 0.0f};

        if (m_MoveForward)
            m_Position += forward * velocity;
        if (m_MoveBackward)
            m_Position -= forward * velocity;
        if (m_MoveRight)
            m_Position += right * velocity;
        if (m_MoveLeft)
            m_Position -= right * velocity;
        if (m_MoveUp)
            m_Position += worldUp * velocity;
        if (m_MoveDown)
            m_Position -= worldUp * velocity;
    }

    if (m_MiddleMouseDown) {
        // Middle mouse for panning (Alt behavior like Unity)
        // Handled via OnMouseMove when middle is down
    }

    UpdateViewMatrix();
}

void EditorCamera::OnMouseMove(float xOffset, float yOffset) {
    if (m_RightMouseDown) {
        // FPS look (right-click held)
        m_Yaw += xOffset * m_MouseSensitivity;
        m_Pitch -= yOffset * m_MouseSensitivity;
        m_Pitch = std::clamp(m_Pitch, -89.0f, 89.0f);
        UpdateViewMatrix();
    } else if (m_MiddleMouseDown) {
        // Pan (middle mouse)
        float panSpeed = 0.01f * m_MoveSpeed;
        glm::vec3 right = GetRightDirection();
        glm::vec3 up = GetUpDirection();
        m_Position -= right * xOffset * panSpeed;
        m_Position += up * yOffset * panSpeed;
        UpdateViewMatrix();
    }
}

void EditorCamera::OnMouseScroll(float yOffset) {
    // Scroll to zoom (move along forward direction)
    glm::vec3 forward = GetForwardDirection();
    m_Position += forward * yOffset * m_ScrollSpeed;
    UpdateViewMatrix();
}

void EditorCamera::SetAspectRatio(float ratio) {
    if (ratio > 0.0f) {
        m_AspectRatio = ratio;
        UpdateProjectionMatrix();
    }
}

void EditorCamera::UpdateProjectionMatrix() {
    m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
    // Vulkan clip space: flip Y
    m_ProjectionMatrix[1][1] *= -1.0f;
}

void EditorCamera::UpdateViewMatrix() {
    glm::vec3 front = GetForwardDirection();
    m_ViewMatrix = glm::lookAt(m_Position, m_Position + front, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 EditorCamera::GetForwardDirection() const {
    float yawRad = glm::radians(m_Yaw);
    float pitchRad = glm::radians(m_Pitch);
    return glm::normalize(
        glm::vec3(std::cos(yawRad) * std::cos(pitchRad), std::sin(pitchRad), std::sin(yawRad) * std::cos(pitchRad)));
}

glm::vec3 EditorCamera::GetRightDirection() const {
    return glm::normalize(glm::cross(GetForwardDirection(), glm::vec3(0.0f, 1.0f, 0.0f)));
}

glm::vec3 EditorCamera::GetUpDirection() const {
    return glm::normalize(glm::cross(GetRightDirection(), GetForwardDirection()));
}

}  // namespace PyEngine
