#pragma once

#include <imgui.h>

#include "PyEngine/Core/Application.hpp"

class ToolbarPanel {
public:
    ToolbarPanel() = default;

    void OnImGuiRender();

private:
    PyEngine::Application::RuntimeState m_CachedState = PyEngine::Application::RuntimeState::Edit;
};
