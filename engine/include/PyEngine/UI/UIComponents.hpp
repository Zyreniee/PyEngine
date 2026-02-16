#pragma once

#include <glm/glm.hpp>
#include <string>

namespace PyEngine {

// UI anchor presets
enum class UIAnchor {
    TopLeft,
    TopCenter,
    TopRight,
    MiddleLeft,
    MiddleCenter,
    MiddleRight,
    BottomLeft,
    BottomCenter,
    BottomRight,
    Stretch
};

struct RectTransformComponent {
    glm::vec2 Position = {0.0f, 0.0f};
    glm::vec2 Size = {100.0f, 100.0f};
    glm::vec2 AnchorMin = {0.5f, 0.5f};
    glm::vec2 AnchorMax = {0.5f, 0.5f};
    glm::vec2 Pivot = {0.5f, 0.5f};
    float Rotation = 0.0f;
};

struct UIImageComponent {
    std::string TexturePath;
    glm::vec4 Color = {1.0f, 1.0f, 1.0f, 1.0f};
    bool IsVisible = true;
};

struct UITextComponent {
    std::string Text = "New Text";
    std::string FontPath = "Arial";  // Default system font
    float FontSize = 14.0f;
    glm::vec4 Color = {0.0f, 0.0f, 0.0f, 1.0f};
    bool IsVisible = true;
};

struct UIButtonComponent {
    glm::vec4 NormalColor = {1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 HoverColor = {0.9f, 0.9f, 0.9f, 1.0f};
    glm::vec4 PressedColor = {0.7f, 0.7f, 0.7f, 1.0f};
    bool IsClicked = false;
};

struct UICanvasComponent {
    bool ScreenSpace = true;
    glm::vec2 ReferenceResolution = {1920.0f, 1080.0f};
    float ScaleFactor = 1.0f;
};

}  // namespace PyEngine
