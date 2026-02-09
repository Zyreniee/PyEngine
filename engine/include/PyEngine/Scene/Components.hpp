#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace PyEngine {

struct TransformComponent {
    glm::vec3 Position = glm::vec3(0.0f);
    glm::vec3 Rotation = glm::vec3(0.0f);  // Euler angles in radians
    glm::vec3 Scale = glm::vec3(1.0f);

    glm::mat4 GetTransform() const;
};

struct CameraComponent {
    glm::mat4 Projection;
    glm::mat4 View;

    float FOV = 45.0f;
    float Near = 0.1f;
    float Far = 1000.0f;

    void SetPerspective(float fov, float aspect, float near, float far);
};

struct MeshRendererComponent {
    uint32_t MeshID = 0;  // Index into asset manager
    glm::vec4 Color = glm::vec4(1.0f);
};

}  // namespace PyEngine
