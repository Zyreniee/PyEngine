#pragma once

#include <glm/glm.hpp>

#include "PyEngine/Scene/Scene.hpp"

namespace PyEngine {

class UISystem {
public:
    UISystem();
    ~UISystem();

    void Init();
    void Update(float deltaTime, Scene* scene);
    void Render(Scene* scene);

private:
    void UpdateLayout(Scene* scene);
    void UpdateInput(Scene* scene);

    bool IsPointInRect(const glm::vec2& point, const glm::vec2& rectPos, const glm::vec2& rectSize);
};

}  // namespace PyEngine
