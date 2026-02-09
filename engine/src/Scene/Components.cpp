#include "PyEngine/Scene/Components.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace PyEngine {

glm::mat4 TransformComponent::GetTransform() const {
  glm::mat4 translation = glm::translate(glm::mat4(1.0f), Position);
  glm::mat4 rotation = glm::eulerAngleXYZ(Rotation.x, Rotation.y, Rotation.z);
  glm::mat4 scale = glm::scale(glm::mat4(1.0f), Scale);
  return translation * rotation * scale;
}

void CameraComponent::SetPerspective(float fov, float aspect, float near,
                                     float far) {
  FOV = fov;
  Near = near;
  Far = far;
  Projection = glm::perspective(glm::radians(fov), aspect, near, far);
}

} // namespace PyEngine
