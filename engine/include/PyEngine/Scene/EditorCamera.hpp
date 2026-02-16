#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace PyEngine {

class EditorCamera {
public:
    EditorCamera() = default;
    EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

    void OnUpdate(float deltaTime);

    // Mouse input
    void OnMouseMove(float xOffset, float yOffset);
    void OnMouseScroll(float yOffset);

    // Key states
    void SetMoveForward(bool v) { m_MoveForward = v; }
    void SetMoveBackward(bool v) { m_MoveBackward = v; }
    void SetMoveLeft(bool v) { m_MoveLeft = v; }
    void SetMoveRight(bool v) { m_MoveRight = v; }
    void SetMoveUp(bool v) { m_MoveUp = v; }
    void SetMoveDown(bool v) { m_MoveDown = v; }
    void SetRightMouseButton(bool v) { m_RightMouseDown = v; }
    void SetMiddleMouseButton(bool v) { m_MiddleMouseDown = v; }

    // Getters
    const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
    const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
    glm::mat4 GetViewProjectionMatrix() const { return m_ProjectionMatrix * m_ViewMatrix; }

    const glm::vec3& GetPosition() const { return m_Position; }
    float GetYaw() const { return m_Yaw; }
    float GetPitch() const { return m_Pitch; }

    void SetAspectRatio(float ratio);
    void SetPosition(const glm::vec3& pos) {
        m_Position = pos;
        UpdateViewMatrix();
    }

    // Movement speed
    float GetMoveSpeed() const { return m_MoveSpeed; }
    void SetMoveSpeed(float speed) { m_MoveSpeed = speed; }

    void SetBoost(bool boost) { m_Boost = boost; }

    // Directions
    glm::vec3 GetForwardDirection() const;
    glm::vec3 GetRightDirection() const;
    glm::vec3 GetUpDirection() const;

private:
    void UpdateProjectionMatrix();
    void UpdateViewMatrix();

private:
    float m_FOV = 60.0f;
    float m_AspectRatio = 16.0f / 9.0f;
    float m_NearClip = 0.1f;
    float m_FarClip = 1000.0f;

    glm::vec3 m_Position = {0.0f, 2.0f, 5.0f};
    float m_Yaw = -90.0f;    // Look toward -Z initially
    float m_Pitch = -15.0f;  // Slight downward angle

    float m_MoveSpeed = 8.0f;
    float m_MouseSensitivity = 0.15f;
    float m_ScrollSpeed = 3.0f;

    // Movement key states
    bool m_MoveForward = false;
    bool m_MoveBackward = false;
    bool m_MoveLeft = false;
    bool m_MoveRight = false;
    bool m_MoveUp = false;
    bool m_MoveDown = false;
    bool m_RightMouseDown = false;
    bool m_MiddleMouseDown = false;
    bool m_Boost = false;

    glm::mat4 m_ViewMatrix{1.0f};
    glm::mat4 m_ProjectionMatrix{1.0f};
};

}  // namespace PyEngine
