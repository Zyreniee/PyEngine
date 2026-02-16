#include "Panels/ProjectPanel.hpp"

namespace fs = std::filesystem;

ProjectPanel::ProjectPanel() {
    m_BaseDirectory = fs::current_path() / "assets";
    m_CurrentDirectory = m_BaseDirectory;
}

void ProjectPanel::SetBaseDirectory(const std::string& path) {
    m_BaseDirectory = path;
    m_CurrentDirectory = m_BaseDirectory;
}

void ProjectPanel::OnImGuiRender() {
    ImGui::Begin("\xef\x81\xbb  Project");  // Icon: folder

    // Two-pane layout
    ImGui::Columns(2, "ProjectColumns", true);
    static bool firstFrame = true;
    if (firstFrame) {
        ImGui::SetColumnWidth(0, 200.0f);
        firstFrame = false;
    }

    // Left pane: directory tree
    ImGui::BeginChild("DirectoryTree");
    if (fs::exists(m_BaseDirectory)) {
        DrawDirectoryTree(m_BaseDirectory);
    } else {
        ImGui::TextDisabled("No assets directory found");
    }
    ImGui::EndChild();

    ImGui::NextColumn();

    // Right pane: file grid
    ImGui::BeginChild("FileGrid");

    // Breadcrumb navigation
    if (m_CurrentDirectory != m_BaseDirectory) {
        if (ImGui::Button("<-")) {
            m_CurrentDirectory = m_CurrentDirectory.parent_path();
        }
        ImGui::SameLine();
    }

    // Show relative path
    auto relativePath = fs::relative(m_CurrentDirectory, m_BaseDirectory);
    ImGui::Text("Assets/%s", relativePath.string().c_str());
    ImGui::Separator();

    DrawFileGrid();
    ImGui::EndChild();

    ImGui::Columns(1);

    ImGui::End();
}

void ProjectPanel::DrawDirectoryTree(const fs::path& path) {
    if (!fs::exists(path) || !fs::is_directory(path))
        return;

    std::string name = path.filename().string();
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

    if (m_CurrentDirectory == path) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    bool hasSubDirs = false;
    if (fs::exists(path)) {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.is_directory()) {
                hasSubDirs = true;
                break;
            }
        }
    }
    if (!hasSubDirs) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    bool opened = ImGui::TreeNodeEx(name.c_str(), flags);

    if (ImGui::IsItemClicked()) {
        m_CurrentDirectory = path;
    }

    if (opened) {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.is_directory()) {
                DrawDirectoryTree(entry.path());
            }
        }
        ImGui::TreePop();
    }
}

void ProjectPanel::DrawFileGrid() {
    if (!fs::exists(m_CurrentDirectory))
        return;

    float cellSize = m_ThumbnailSize + m_Padding;
    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = std::max(1, (int)(panelWidth / cellSize));

    ImGui::Columns(columnCount, nullptr, false);

    for (const auto& entry : fs::directory_iterator(m_CurrentDirectory)) {
        const auto& path = entry.path();
        std::string filename = path.filename().string();

        ImGui::PushID(filename.c_str());

        // Icon
        if (entry.is_directory()) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.3f, 0.5f, 0.5f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 0.3f));
        }

        ImGui::Button(entry.is_directory() ? "\xef\x81\xbb" : "\xef\x85\x9b", ImVec2(m_ThumbnailSize, m_ThumbnailSize));
        ImGui::PopStyleColor();

        // Double click to enter directory
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            if (entry.is_directory()) {
                m_CurrentDirectory = path;
            }
        }

        // Drag source for files
        if (!entry.is_directory() && ImGui::BeginDragDropSource()) {
            std::string pathStr = path.string();
            ImGui::SetDragDropPayload("ASSET_PATH", pathStr.c_str(), pathStr.size() + 1);
            ImGui::Text("%s", filename.c_str());
            ImGui::EndDragDropSource();
        }

        // Truncated filename
        ImGui::TextWrapped("%s", filename.c_str());

        ImGui::NextColumn();
        ImGui::PopID();
    }

    ImGui::Columns(1);
}
