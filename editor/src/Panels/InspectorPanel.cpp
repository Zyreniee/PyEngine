#include "Panels/InspectorPanel.hpp"

#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>

#include "PyEngine/Scene/Components.hpp"

void InspectorPanel::OnImGuiRender() {
    ImGui::Begin("\xef\x81\x9a  Inspector");  // Icon: info-circle

    if (m_SelectedEntity) {
        DrawComponents(m_SelectedEntity);
    } else {
        ImGui::TextDisabled("No entity selected");
    }

    ImGui::End();
}

void InspectorPanel::DrawVec3Control(const char* label, glm::vec3& values, float resetValue, float columnWidth) {
    ImGui::PushID(label);

    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::Text("%s", label);
    ImGui::NextColumn();

    float totalWidth = ImGui::GetContentRegionAvail().x;
    float itemWidth = (totalWidth - 3.0f * ImGui::GetStyle().ItemInnerSpacing.x) / 3.0f;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{2, 0}); // Tighter spacing between X/Y/Z

    float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
    ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

    // X
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.2f, 0.2f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.3f, 0.3f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.2f, 0.2f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0f, 1.0f, 1.0f, 1.0f});
    if (ImGui::Button("X", buttonSize)) values.x = resetValue;
    ImGui::PopStyleColor(4);
    ImGui::SameLine();
    ImGui::PushItemWidth(itemWidth - buttonSize.x);
    ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    // Y
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.8f, 0.2f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.9f, 0.3f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.8f, 0.2f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0f, 1.0f, 1.0f, 1.0f});
    if (ImGui::Button("Y", buttonSize)) values.y = resetValue;
    ImGui::PopStyleColor(4);
    ImGui::SameLine();
    ImGui::PushItemWidth(itemWidth - buttonSize.x);
    ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    // Z
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.4f, 1.0f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.5f, 1.0f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.4f, 1.0f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0f, 1.0f, 1.0f, 1.0f});
    if (ImGui::Button("Z", buttonSize)) values.z = resetValue;
    ImGui::PopStyleColor(4);
    ImGui::SameLine();
    ImGui::PushItemWidth(itemWidth - buttonSize.x);
    ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();

    ImGui::PopStyleVar();
    ImGui::Columns(1);

    ImGui::PopID();
}

void InspectorPanel::DrawComponents(PyEngine::Entity entity) {
    // ─── Name / Tag ──────────────────────────────────────────
    if (entity.HasComponent<PyEngine::TagComponent>()) {
        auto& tag = entity.GetComponent<PyEngine::TagComponent>().Tag;

        char buffer[256];
        std::strncpy(buffer, tag.c_str(), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';

        ImGui::PushItemWidth(-1);
        if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
            tag = std::string(buffer);
        }
        ImGui::PopItemWidth();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ─── Transform ───────────────────────────────────────────
    if (entity.HasComponent<PyEngine::TransformComponent>()) {
        const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
        if (ImGui::TreeNodeEx("Transform", treeNodeFlags)) {
            auto& tc = entity.GetTransform();
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 4));
            DrawVec3Control("Position", tc.Position, 0.0f, 70.0f);
            DrawVec3Control("Rotation", tc.Rotation, 0.0f, 70.0f);
            DrawVec3Control("Scale", tc.Scale, 1.0f, 70.0f);
            ImGui::PopStyleVar();
            ImGui::TreePop();
        }
    }

    // ─── Camera ──────────────────────────────────────────────
    if (entity.HasComponent<PyEngine::CameraComponent>()) {
        if (ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                            ImGuiTreeNodeFlags_SpanAvailWidth)) {
            auto& cc = entity.GetComponent<PyEngine::CameraComponent>();
            ImGui::Checkbox("Primary", &cc.Primary);
            ImGui::DragFloat("FOV", &cc.FieldOfView, 1.0f, 1.0f, 179.0f);
            ImGui::DragFloat("Near Clip", &cc.NearPlane, 0.01f, 0.001f, 100.0f);
            ImGui::DragFloat("Far Clip", &cc.FarPlane, 1.0f, 1.0f, 10000.0f);
            ImGui::TreePop();
        }
    }

    // ─── MeshRenderer ────────────────────────────────────────
    if (entity.HasComponent<PyEngine::MeshRendererComponent>()) {
        if (ImGui::TreeNodeEx("Mesh Renderer", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                                   ImGuiTreeNodeFlags_SpanAvailWidth)) {
            auto& mrc = entity.GetComponent<PyEngine::MeshRendererComponent>();

            const char* meshNames[] = {"Cube", "Sphere", "Plane", "Cylinder", "Capsule"};
            int currentMesh = static_cast<int>(mrc.MeshID);
            if (currentMesh >= 5)
                currentMesh = 0;
            if (ImGui::Combo("Mesh", &currentMesh, meshNames, 5)) {
                mrc.MeshID = currentMesh;
            }

            ImGui::ColorEdit4("Color", glm::value_ptr(mrc.ColorTint));

            ImGui::Spacing();
            ImGui::Text("PBR Material");
            ImGui::Separator();
            ImGui::SliderFloat("Metallic", &mrc.Metallic, 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("Roughness", &mrc.Roughness, 0.04f, 1.0f, "%.2f");
            ImGui::SliderFloat("AO", &mrc.AO, 0.0f, 1.0f, "%.2f");
            ImGui::Spacing();

            ImGui::Checkbox("Cast Shadows", &mrc.CastShadows);
            ImGui::Checkbox("Receive Shadows", &mrc.ReceiveShadows);
            ImGui::TreePop();
        }
    }

    // ─── SpriteRenderer ──────────────────────────────────────
    if (entity.HasComponent<PyEngine::SpriteRendererComponent>()) {
        if (ImGui::TreeNodeEx("Sprite Renderer", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                                     ImGuiTreeNodeFlags_SpanAvailWidth)) {
            auto& src = entity.GetComponent<PyEngine::SpriteRendererComponent>();
            ImGui::ColorEdit4("Color", glm::value_ptr(src.Color));
            ImGui::DragFloat2("Tiling", glm::value_ptr(src.Tiling), 0.1f, 0.0f, 100.0f);
            ImGui::DragInt("Sorting Order", &src.SortingOrder);
            ImGui::TreePop();
        }
    }

    // ─── Light ───────────────────────────────────────────────
    if (entity.HasComponent<PyEngine::LightComponent>()) {
        if (ImGui::TreeNodeEx("Light", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                           ImGuiTreeNodeFlags_SpanAvailWidth)) {
            auto& lc = entity.GetComponent<PyEngine::LightComponent>();

            const char* lightTypes[] = {"Directional", "Point", "Spot"};
            int currentType = static_cast<int>(lc.LightType);
            if (ImGui::Combo("Type", &currentType, lightTypes, 3)) {
                lc.LightType = static_cast<PyEngine::LightComponent::Type>(currentType);
            }

            ImGui::ColorEdit3("Color", glm::value_ptr(lc.Color));
            ImGui::DragFloat("Intensity", &lc.Intensity, 0.1f, 0.0f, 100.0f);

            if (lc.LightType != PyEngine::LightComponent::Type::Directional) {
                ImGui::DragFloat("Range", &lc.Range, 0.1f, 0.0f, 1000.0f);
            }
            if (lc.LightType == PyEngine::LightComponent::Type::Spot) {
                ImGui::DragFloat("Inner Angle", &lc.InnerConeAngle, 0.5f, 0.0f, 90.0f);
                ImGui::DragFloat("Outer Angle", &lc.OuterConeAngle, 0.5f, 0.0f, 90.0f);
            }
            ImGui::Checkbox("Cast Shadows", &lc.CastShadows);
            ImGui::TreePop();
        }
    }

    // ─── Rigidbody ───────────────────────────────────────────
    if (entity.HasComponent<PyEngine::RigidBodyComponent>()) {
        if (ImGui::TreeNodeEx("Rigidbody", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                               ImGuiTreeNodeFlags_SpanAvailWidth)) {
            auto& rb = entity.GetComponent<PyEngine::RigidBodyComponent>();

            const char* bodyTypes[] = {"Static", "Dynamic", "Kinematic"};
            int currentType = static_cast<int>(rb.Type);
            if (ImGui::Combo("Body Type", &currentType, bodyTypes, 3)) {
                rb.Type = static_cast<PyEngine::RigidBodyComponent::BodyType>(currentType);
            }

            ImGui::DragFloat("Mass", &rb.Mass, 0.1f, 0.0f, 10000.0f);
            ImGui::DragFloat("Linear Damping", &rb.LinearDamping, 0.01f, 0.0f, 100.0f);
            ImGui::DragFloat("Angular Damping", &rb.AngularDamping, 0.01f, 0.0f, 100.0f);
            ImGui::Checkbox("Use Gravity", &rb.UseGravity);
            ImGui::Checkbox("Freeze X", &rb.FreezeRotationX);
            ImGui::Checkbox("Freeze Y", &rb.FreezeRotationY);
            ImGui::Checkbox("Freeze Z", &rb.FreezeRotationZ);
            ImGui::TreePop();
        }
    }

    // ─── BoxCollider ─────────────────────────────────────────
    if (entity.HasComponent<PyEngine::BoxColliderComponent>()) {
        if (ImGui::TreeNodeEx("Box Collider", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                                  ImGuiTreeNodeFlags_SpanAvailWidth)) {
            auto& bc = entity.GetComponent<PyEngine::BoxColliderComponent>();
            DrawVec3Control("Size", bc.Size, 1.0f);
            DrawVec3Control("Offset", bc.Center);
            ImGui::Checkbox("Is Trigger", &bc.IsTrigger);
            ImGui::TreePop();
        }
    }

    // ─── SphereCollider ──────────────────────────────────────
    if (entity.HasComponent<PyEngine::SphereColliderComponent>()) {
        if (ImGui::TreeNodeEx("Sphere Collider", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                                     ImGuiTreeNodeFlags_SpanAvailWidth)) {
            auto& sc = entity.GetComponent<PyEngine::SphereColliderComponent>();
            ImGui::DragFloat("Radius", &sc.Radius, 0.01f, 0.0f, 100.0f);
            DrawVec3Control("Offset", sc.Center);
            ImGui::Checkbox("Is Trigger", &sc.IsTrigger);
            ImGui::TreePop();
        }
    }

    // ─── Script ──────────────────────────────────────────────
    if (entity.HasComponent<PyEngine::ScriptComponent>()) {
        if (ImGui::TreeNodeEx("Script", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                            ImGuiTreeNodeFlags_SpanAvailWidth)) {
            auto& sc = entity.GetComponent<PyEngine::ScriptComponent>();
            // Scripts are now a list
            if (!sc.ScriptClassNames.empty()) {
                char buffer[256];
                std::strncpy(buffer, sc.ScriptClassNames[0].c_str(), sizeof(buffer) - 1);
                buffer[sizeof(buffer) - 1] = '\0';
                if (ImGui::InputText("Class Name", buffer, sizeof(buffer))) {
                    sc.ScriptClassNames[0] = std::string(buffer);
                }
            } else {
                char buffer[256] = "";
                if (ImGui::InputText("Add Class", buffer, sizeof(buffer))) {
                    sc.ScriptClassNames.push_back(std::string(buffer));
                }
            }
            ImGui::Checkbox("Enabled", &sc.Enabled);
            ImGui::TreePop();
        }
    }

    // ─── AudioSource ─────────────────────────────────────────
    if (entity.HasComponent<PyEngine::AudioSourceComponent>()) {
        if (ImGui::TreeNodeEx("Audio Source", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                                  ImGuiTreeNodeFlags_SpanAvailWidth)) {
            auto& as = entity.GetComponent<PyEngine::AudioSourceComponent>();
            char buffer[256];
            std::strncpy(buffer, as.ClipName.c_str(), sizeof(buffer) - 1);
            buffer[sizeof(buffer) - 1] = '\0';
            if (ImGui::InputText("Audio Clip", buffer, sizeof(buffer))) {
                as.ClipName = std::string(buffer);
            }
            ImGui::DragFloat("Volume", &as.Volume, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Pitch", &as.Pitch, 0.01f, 0.0f, 3.0f);
            ImGui::Checkbox("Play On Awake", &as.PlayOnAwake);
            ImGui::Checkbox("Loop", &as.Loop);
            ImGui::Checkbox("Spatial", &as.Spatial);
            ImGui::TreePop();
        }
    }

    // ─── PythonScript ────────────────────────────────────────
    if (entity.HasComponent<PyEngine::PythonScriptComponent>()) {
        if (ImGui::TreeNodeEx("\xf0\x9f\x90\x8d Python Script", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                            ImGuiTreeNodeFlags_SpanAvailWidth)) {
            auto& psc = entity.GetComponent<PyEngine::PythonScriptComponent>();
            char buffer[512];
            std::strncpy(buffer, psc.ScriptPath.c_str(), sizeof(buffer) - 1);
            buffer[sizeof(buffer) - 1] = '\0';
            if (ImGui::InputText("Script Path", buffer, sizeof(buffer))) {
                psc.ScriptPath = std::string(buffer);
            }
            ImGui::Checkbox("Enabled", &psc.Enabled);
            ImGui::Checkbox("Auto Reload", &psc.AutoReload);
            if (ImGui::Button("Reload Script")) {
                // TODO: Trigger hot-reload for this entity's PythonScript
            }
            ImGui::TreePop();
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // ─── Add Component Button ────────────────────────────────
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Center the Add Component button
    float buttonWidth = 200.0f;
    float availWidth = ImGui::GetContentRegionAvail().x;
    ImGui::SetCursorPosX((availWidth - buttonWidth) * 0.5f);
    
    // Style the button slightly differently like Unity
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.25f, 0.25f, 0.25f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.35f, 0.35f, 0.35f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.20f, 0.20f, 0.20f, 1.0f});
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 15.0f); // Rounded capsule button
    if (ImGui::Button("Add Component", ImVec2{buttonWidth, 25})) {
        ImGui::OpenPopup("AddComponent");
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    if (ImGui::BeginPopup("AddComponent")) {
        DrawAddComponentMenu(entity);
        ImGui::EndPopup();
    }
}

void InspectorPanel::DrawAddComponentMenu(PyEngine::Entity entity) {
    if (!entity.HasComponent<PyEngine::CameraComponent>()) {
        if (ImGui::MenuItem("Camera")) {
            entity.AddComponent<PyEngine::CameraComponent>();
            ImGui::CloseCurrentPopup();
        }
    }
    if (!entity.HasComponent<PyEngine::MeshRendererComponent>()) {
        if (ImGui::MenuItem("Mesh Renderer")) {
            entity.AddComponent<PyEngine::MeshRendererComponent>();
            ImGui::CloseCurrentPopup();
        }
    }
    if (!entity.HasComponent<PyEngine::SpriteRendererComponent>()) {
        if (ImGui::MenuItem("Sprite Renderer")) {
            entity.AddComponent<PyEngine::SpriteRendererComponent>();
            ImGui::CloseCurrentPopup();
        }
    }
    if (!entity.HasComponent<PyEngine::LightComponent>()) {
        if (ImGui::MenuItem("Light")) {
            entity.AddComponent<PyEngine::LightComponent>();
            ImGui::CloseCurrentPopup();
        }
    }
    if (!entity.HasComponent<PyEngine::RigidBodyComponent>()) {
        if (ImGui::MenuItem("Rigidbody")) {
            entity.AddComponent<PyEngine::RigidBodyComponent>();
            ImGui::CloseCurrentPopup();
        }
    }
    if (!entity.HasComponent<PyEngine::BoxColliderComponent>()) {
        if (ImGui::MenuItem("Box Collider")) {
            entity.AddComponent<PyEngine::BoxColliderComponent>();
            ImGui::CloseCurrentPopup();
        }
    }
    if (!entity.HasComponent<PyEngine::SphereColliderComponent>()) {
        if (ImGui::MenuItem("Sphere Collider")) {
            entity.AddComponent<PyEngine::SphereColliderComponent>();
            ImGui::CloseCurrentPopup();
        }
    }
    if (!entity.HasComponent<PyEngine::ScriptComponent>()) {
        if (ImGui::MenuItem("Script")) {
            entity.AddComponent<PyEngine::ScriptComponent>();
            ImGui::CloseCurrentPopup();
        }
    }
    if (!entity.HasComponent<PyEngine::AudioSourceComponent>()) {
        if (ImGui::MenuItem("Audio Source")) {
            entity.AddComponent<PyEngine::AudioSourceComponent>();
            ImGui::CloseCurrentPopup();
        }
    }
    if (!entity.HasComponent<PyEngine::AudioListenerComponent>()) {
        if (ImGui::MenuItem("Audio Listener")) {
            entity.AddComponent<PyEngine::AudioListenerComponent>();
            ImGui::CloseCurrentPopup();
        }
    }
    if (!entity.HasComponent<PyEngine::ParticleSystemComponent>()) {
        if (ImGui::MenuItem("Particle System")) {
            entity.AddComponent<PyEngine::ParticleSystemComponent>();
            ImGui::CloseCurrentPopup();
        }
    }
    if (!entity.HasComponent<PyEngine::PythonScriptComponent>()) {
        if (ImGui::MenuItem("Python Script")) {
            entity.AddComponent<PyEngine::PythonScriptComponent>();
            ImGui::CloseCurrentPopup();
        }
    }
}
