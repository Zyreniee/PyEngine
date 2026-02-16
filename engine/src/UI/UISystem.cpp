#include "PyEngine/UI/UISystem.hpp"

#include "PyEngine/Core/Application.hpp"
#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Core/MouseCodes.hpp"
#include "PyEngine/Platform/Input.hpp"
#include "PyEngine/Renderer/Renderer.hpp"
#include "PyEngine/UI/UIComponents.hpp"

namespace PyEngine {

UISystem::UISystem() {}
UISystem::~UISystem() {}

void UISystem::Init() {
    PYENGINE_CORE_INFO("UI System Initialized");
}

void UISystem::Update(float deltaTime, Scene* scene) {
    UpdateLayout(scene);
    UpdateInput(scene);
}

void UISystem::Render(Scene* scene) {
    auto& app = Application::Get();

    // Draw Images
    auto group = scene->GetRegistry().group<UIImageComponent>(entt::get<RectTransformComponent>);
    for (auto entity : group) {
        auto [image, rect] = group.get<UIImageComponent, RectTransformComponent>(entity);
        if (!image.IsVisible)
            continue;

        // DrawQuad(rect.Position, rect.Size, image.Texture, image.Color);
    }
}

void UISystem::UpdateLayout(Scene* scene) {
    auto view = scene->GetRegistry().view<RectTransformComponent, UICanvasComponent>();
    for (auto entity : view) {
        // Handle canvas scaling
    }
}

void UISystem::UpdateInput(Scene* scene) {
    auto mousePos = Input::GetMousePosition();

    auto view = scene->GetRegistry().view<UIButtonComponent, RectTransformComponent>();
    for (auto entity : view) {
        auto& button = view.get<UIButtonComponent>(entity);
        auto& rect = view.get<RectTransformComponent>(entity);

        bool hovered =
            IsPointInRect({mousePos.x, mousePos.y}, rect.Position, rect.Size);  // Fixed .first/.second to .x/.y

        if (hovered) {
            if (Input::IsMouseButtonPressed(Mouse::ButtonLeft)) {
                button.IsClicked = true;
            } else {
                button.IsClicked = false;
            }
        }
    }
}

bool UISystem::IsPointInRect(const glm::vec2& point, const glm::vec2& rectPos, const glm::vec2& rectSize) {
    return point.x >= rectPos.x && point.x <= rectPos.x + rectSize.x && point.y >= rectPos.y &&
           point.y <= rectPos.y + rectSize.y;
}

}  // namespace PyEngine
