#pragma once

#include <memory>

#include "PyEngine/Scene/Entity.hpp"

namespace PyEngine {
class Scene;
}

class HierarchyPanel {
public:
    HierarchyPanel() = default;
    explicit HierarchyPanel(const std::shared_ptr<PyEngine::Scene>& scene);

    void SetScene(const std::shared_ptr<PyEngine::Scene>& scene);
    void OnImGuiRender();

    PyEngine::Entity GetSelectedEntity() const { return m_SelectedEntity; }
    void SetSelectedEntity(PyEngine::Entity entity) { m_SelectedEntity = entity; }

private:
    void DrawEntityNode(PyEngine::Entity entity);
    void DrawCreateEntityMenu();

    std::shared_ptr<PyEngine::Scene> m_Scene;
    PyEngine::Entity m_SelectedEntity;
};
