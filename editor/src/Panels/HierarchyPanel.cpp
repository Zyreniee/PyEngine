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

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf;

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
        if (ImGui::MenuItem("Copy")) {
            // TODO
        }
        if (ImGui::MenuItem("Paste")) {
            // TODO
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Rename")) {
            // TODO
        }
        if (ImGui::MenuItem("Duplicate", "Ctrl+D")) {
            // TODO: Implement duplication
        }
        if (ImGui::MenuItem("Delete", "Del")) {
            entityDeleted = true;
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
        auto e = m_Scene->CreateEntity("GameObject");
        SetSelectedEntity(e);
    }
    
    ImGui::Separator();

    if (ImGui::BeginMenu("3D Object")) {
        if (ImGui::MenuItem("Cube")) {
            auto entity = m_Scene->CreateEntity("Cube");
            entity.AddComponent<PyEngine::MeshRendererComponent>().MeshID = 0;
            SetSelectedEntity(entity);
        }
        if (ImGui::MenuItem("Sphere")) {
            auto entity = m_Scene->CreateEntity("Sphere");
            entity.AddComponent<PyEngine::MeshRendererComponent>().MeshID = 1;
            SetSelectedEntity(entity);
        }
        if (ImGui::MenuItem("Capsule")) {
            auto entity = m_Scene->CreateEntity("Capsule");
            entity.AddComponent<PyEngine::MeshRendererComponent>().MeshID = 4;
            SetSelectedEntity(entity);
        }
        if (ImGui::MenuItem("Cylinder")) {
            auto entity = m_Scene->CreateEntity("Cylinder");
            entity.AddComponent<PyEngine::MeshRendererComponent>().MeshID = 3;
            SetSelectedEntity(entity);
        }
        if (ImGui::MenuItem("Plane")) {
            auto entity = m_Scene->CreateEntity("Plane");
            entity.AddComponent<PyEngine::MeshRendererComponent>().MeshID = 2;
            SetSelectedEntity(entity);
        }
        if (ImGui::MenuItem("Terrain")) {
            auto entity = m_Scene->CreateEntity("Terrain");
            entity.AddComponent<PyEngine::MeshRendererComponent>().MeshID = 2;
            entity.AddComponent<PyEngine::TerrainComponent>();
            SetSelectedEntity(entity);
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Light")) {
        if (ImGui::MenuItem("Directional Light")) {
            auto entity = m_Scene->CreateEntity("Directional Light");
            entity.AddComponent<PyEngine::LightComponent>();
            SetSelectedEntity(entity);
        }
        if (ImGui::MenuItem("Point Light")) {
            auto entity = m_Scene->CreateEntity("Point Light");
            auto& light = entity.AddComponent<PyEngine::LightComponent>();
            light.LightType = PyEngine::LightComponent::Type::Point;
            SetSelectedEntity(entity);
        }
        if (ImGui::MenuItem("Spot Light")) {
            auto entity = m_Scene->CreateEntity("Spot Light");
            auto& light = entity.AddComponent<PyEngine::LightComponent>();
            light.LightType = PyEngine::LightComponent::Type::Spot;
            SetSelectedEntity(entity);
        }
        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Camera")) {
        auto entity = m_Scene->CreateEntity("Camera");
        entity.AddComponent<PyEngine::CameraComponent>();
        SetSelectedEntity(entity);
    }
}
