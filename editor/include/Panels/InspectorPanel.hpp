#pragma once

#include "PyEngine/Scene/Entity.hpp"

class InspectorPanel {
public:
    InspectorPanel() = default;

    void OnImGuiRender();

    void SetSelectedEntity(PyEngine::Entity entity) { m_SelectedEntity = entity; }

private:
    void DrawComponents(PyEngine::Entity entity);
    void DrawAddComponentMenu(PyEngine::Entity entity);

    // Component drawing helpers
    void DrawVec3Control(const char* label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);

    PyEngine::Entity m_SelectedEntity;
};
