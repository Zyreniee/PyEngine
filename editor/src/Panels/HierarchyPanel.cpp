#include "Panels/HierarchyPanel.hpp"

#include <imgui.h>

#include "PyEngine/Scene/Components.hpp"
#include "PyEngine/Scene/Scene.hpp"

HierarchyPanel::HierarchyPanel(const std::shared_ptr<PyEngine::Scene>& scene) : m_Scene(scene) {}

void HierarchyPanel::SetScene(const std::shared_ptr<PyEngine::Scene>& scene) {
    m_Scene = scene;
    m_SelectedEntity = {};
}

void HierarchyPanel::OnImGuiRender() {
    ImGui::Begin("\xef\x80\xa8  Hierarchy");  // Icon: sitemap

    if (m_Scene) {
        bool terrainNodeOpened = false;
        if (ImGui::TreeNodeEx("Terrain", ImGuiTreeNodeFlags_SpanAvailWidth)) {
            terrainNodeOpened = true;
        }

        auto entities = m_Scene->GetAllEntities();
        for (auto entity : entities) {
            auto& tag = entity.GetComponent<PyEngine::TagComponent>().Tag;

            // Check if it's a terrain chunk
            if (tag.find("Terrain Chunk") != std::string::npos) {
                if (terrainNodeOpened) {
                    DrawEntityNode(entity);
                }
            } else {
                // Normal entity
                DrawEntityNode(entity);
            }
        }

        if (terrainNodeOpened) {
            ImGui::TreePop();
        }

        // Deselect on empty space click
        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
            m_SelectedEntity = {};
        }

        // Right-click context menu on empty space
        if (ImGui::BeginPopupContextWindow(nullptr,
                                           ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight)) {
            DrawCreateEntityMenu();
            ImGui::EndPopup();
        }
    }

    ImGui::End();
}

void HierarchyPanel::DrawEntityNode(PyEngine::Entity entity) {
    auto& tag = entity.GetComponent<PyEngine::TagComponent>().Tag;

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf;

    if (m_SelectedEntity == entity) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.c_str());

    if (ImGui::IsItemClicked()) {
        m_SelectedEntity = entity;
    }

    // Right-click context menu on entity
    bool entityDeleted = false;
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Delete Entity")) {
            entityDeleted = true;
        }
        if (ImGui::MenuItem("Duplicate Entity")) {
            // TODO: Implement duplication
        }
        ImGui::Separator();
        DrawCreateEntityMenu();
        ImGui::EndPopup();
    }

    if (opened) {
        ImGui::TreePop();
    }

    if (entityDeleted) {
        if (m_SelectedEntity == entity) {
            m_SelectedEntity = {};
        }
        m_Scene->DestroyEntity(entity);
    }
}

void HierarchyPanel::DrawCreateEntityMenu() {
    if (ImGui::MenuItem("Create Empty")) {
        m_Scene->CreateEntity("GameObject");
    }

    if (ImGui::BeginMenu("3D Object")) {
        if (ImGui::MenuItem("Cube")) {
            auto entity = m_Scene->CreateEntity("Cube");
            entity.AddComponent<PyEngine::MeshRendererComponent>().MeshID = 0;
        }
        if (ImGui::MenuItem("Sphere")) {
            auto entity = m_Scene->CreateEntity("Sphere");
            entity.AddComponent<PyEngine::MeshRendererComponent>().MeshID = 1;
        }
        if (ImGui::MenuItem("Plane")) {
            auto entity = m_Scene->CreateEntity("Plane");
            entity.AddComponent<PyEngine::MeshRendererComponent>().MeshID = 2;
        }
        if (ImGui::MenuItem("Cylinder")) {
            auto entity = m_Scene->CreateEntity("Cylinder");
            entity.AddComponent<PyEngine::MeshRendererComponent>().MeshID = 3;
        }
        if (ImGui::MenuItem("Capsule")) {
            auto entity = m_Scene->CreateEntity("Capsule");
            entity.AddComponent<PyEngine::MeshRendererComponent>().MeshID = 4;
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Light")) {
        if (ImGui::MenuItem("Directional Light")) {
            auto entity = m_Scene->CreateEntity("Directional Light");
            entity.AddComponent<PyEngine::LightComponent>();
        }
        if (ImGui::MenuItem("Point Light")) {
            auto entity = m_Scene->CreateEntity("Point Light");
            auto& light = entity.AddComponent<PyEngine::LightComponent>();
            light.LightType = PyEngine::LightComponent::Type::Point;
        }
        if (ImGui::MenuItem("Spot Light")) {
            auto entity = m_Scene->CreateEntity("Spot Light");
            auto& light = entity.AddComponent<PyEngine::LightComponent>();
            light.LightType = PyEngine::LightComponent::Type::Spot;
        }
        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Camera")) {
        auto entity = m_Scene->CreateEntity("Camera");
        entity.AddComponent<PyEngine::CameraComponent>();
    }
}
