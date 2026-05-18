#include "Panels/HierarchyPanel.hpp"

#include <imgui.h>

#include "PyEngine/Scene/Components.hpp"
#include "PyEngine/Scene/Scene.hpp"

HierarchyPanel::HierarchyPanel(const std::shared_ptr<PyEngine::Scene>& scene) : m_Scene(scene) {}

void HierarchyPanel::SetScene(const std::shared_ptr<PyEngine::Scene>& scene) {
    m_Scene = scene;
    m_SelectedEntity = {};
}

// Get a display icon for entity type
static const char* GetEntityIcon(PyEngine::Entity entity) {
    if (entity.HasComponent<PyEngine::CameraComponent>())       return "[CAM] ";
    if (entity.HasComponent<PyEngine::LightComponent>())        return "[LGT] ";
    if (entity.HasComponent<PyEngine::MeshRendererComponent>()) return "[MESH] ";
    if (entity.HasComponent<PyEngine::PythonScriptComponent>()) return "[PY] ";
    if (entity.HasComponent<PyEngine::TerrainComponent>())      return "[TRN] ";
    return "";
}

void HierarchyPanel::OnImGuiRender() {
    ImGui::Begin("Hierarchy");

    if (m_Scene) {
        // Scene name header
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.8f, 1.0f, 1.0f));
        ImGui::Text("Scene: %s", m_Scene->GetName().c_str());
        ImGui::PopStyleColor();
        ImGui::Separator();
        ImGui::Spacing();

        auto entities = m_Scene->GetAllEntities();

        // Terrain group
        bool hasTerrain = false;
        for (auto& entity : entities) {
            auto& tag = entity.GetComponent<PyEngine::TagComponent>().Tag;
            if (tag.find("Terrain Chunk") != std::string::npos) {
                hasTerrain = true;
                break;
            }
        }

        // Draw normal entities first
        for (auto entity : entities) {
            auto& tag = entity.GetComponent<PyEngine::TagComponent>().Tag;
            if (tag.find("Terrain Chunk") != std::string::npos) continue;
            DrawEntityNode(entity);
        }

        // Draw terrain entities in a group
        if (hasTerrain) {
            if (ImGui::TreeNodeEx("Terrain", ImGuiTreeNodeFlags_SpanAvailWidth)) {
                for (auto entity : entities) {
                    auto& tag = entity.GetComponent<PyEngine::TagComponent>().Tag;
                    if (tag.find("Terrain Chunk") != std::string::npos) {
                        DrawEntityNode(entity);
                    }
                }
                ImGui::TreePop();
            }
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

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                               ImGuiTreeNodeFlags_SpanAvailWidth |
                               ImGuiTreeNodeFlags_Leaf;

    if (m_SelectedEntity == entity) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    // Build display name with icon prefix
    std::string displayName = std::string(GetEntityIcon(entity)) + tag;

    bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", displayName.c_str());

    if (ImGui::IsItemClicked()) {
        m_SelectedEntity = entity;
    }

    // Right-click context menu on entity
    bool entityDeleted = false;
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Duplicate", "Ctrl+D")) {
            // TODO: Implement duplication
        }
        if (ImGui::MenuItem("Rename")) {
            // TODO
        }
        ImGui::Separator();
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
            entity.GetComponent<PyEngine::TransformComponent>().Position = {0.0f, 0.5f, 0.0f};
            SetSelectedEntity(entity);
        }
        if (ImGui::MenuItem("Sphere")) {
            auto entity = m_Scene->CreateEntity("Sphere");
            entity.AddComponent<PyEngine::MeshRendererComponent>().MeshID = 1;
            entity.GetComponent<PyEngine::TransformComponent>().Position = {0.0f, 0.5f, 0.0f};
            SetSelectedEntity(entity);
        }
        if (ImGui::MenuItem("Capsule")) {
            auto entity = m_Scene->CreateEntity("Capsule");
            entity.AddComponent<PyEngine::MeshRendererComponent>().MeshID = 4;
            entity.GetComponent<PyEngine::TransformComponent>().Position = {0.0f, 0.75f, 0.0f};
            SetSelectedEntity(entity);
        }
        if (ImGui::MenuItem("Cylinder")) {
            auto entity = m_Scene->CreateEntity("Cylinder");
            entity.AddComponent<PyEngine::MeshRendererComponent>().MeshID = 3;
            entity.GetComponent<PyEngine::TransformComponent>().Position = {0.0f, 0.5f, 0.0f};
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
            entity.GetComponent<PyEngine::TransformComponent>().Scale = {10.0f, 1.0f, 10.0f};
            entity.AddComponent<PyEngine::TerrainComponent>();
            SetSelectedEntity(entity);
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Light")) {
        if (ImGui::MenuItem("Directional Light")) {
            auto entity = m_Scene->CreateEntity("Directional Light");
            auto& lc = entity.AddComponent<PyEngine::LightComponent>();
            lc.Intensity = 1.0f;
            entity.GetComponent<PyEngine::TransformComponent>().Rotation = {-50.0f, -30.0f, 0.0f};
            SetSelectedEntity(entity);
        }
        if (ImGui::MenuItem("Point Light")) {
            auto entity = m_Scene->CreateEntity("Point Light");
            auto& light = entity.AddComponent<PyEngine::LightComponent>();
            light.LightType = PyEngine::LightComponent::Type::Point;
            light.Intensity = 1.5f;
            light.Range = 15.0f;
            entity.GetComponent<PyEngine::TransformComponent>().Position = {0.0f, 3.0f, 0.0f};
            SetSelectedEntity(entity);
        }
        if (ImGui::MenuItem("Spot Light")) {
            auto entity = m_Scene->CreateEntity("Spot Light");
            auto& light = entity.AddComponent<PyEngine::LightComponent>();
            light.LightType = PyEngine::LightComponent::Type::Spot;
            light.Intensity = 2.0f;
            entity.GetComponent<PyEngine::TransformComponent>().Position = {0.0f, 3.0f, 0.0f};
            entity.GetComponent<PyEngine::TransformComponent>().Rotation = {-45.0f, 0.0f, 0.0f};
            SetSelectedEntity(entity);
        }
        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Camera")) {
        auto entity = m_Scene->CreateEntity("Camera");
        entity.AddComponent<PyEngine::CameraComponent>();
        entity.GetComponent<PyEngine::TransformComponent>().Position = {0.0f, 2.0f, 5.0f};
        entity.GetComponent<PyEngine::TransformComponent>().Rotation = {-10.0f, 0.0f, 0.0f};
        SetSelectedEntity(entity);
    }
}
