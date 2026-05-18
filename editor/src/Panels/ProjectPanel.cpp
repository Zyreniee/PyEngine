#include "Panels/ProjectPanel.hpp"

#include <algorithm>
#include <cstdlib>
#include <vector>

namespace fs = std::filesystem;

ProjectPanel::ProjectPanel() {
    m_BaseDirectory = fs::current_path() / "assets";
    m_CurrentDirectory = m_BaseDirectory;
}

void ProjectPanel::SetBaseDirectory(const std::string& path) {
    m_BaseDirectory = path;
    m_CurrentDirectory = m_BaseDirectory;
}

// Get a display-friendly icon for a file extension
static const char* GetFileIcon(const std::string& ext) {
    if (ext == ".py")    return "[PY]";
    if (ext == ".lua")   return "[LU]";
    if (ext == ".glsl" || ext == ".vert" || ext == ".frag" || ext == ".spv")
        return "[SH]";
    if (ext == ".gltf" || ext == ".glb" || ext == ".obj" || ext == ".fbx")
        return "[3D]";
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
        return "[TX]";
    if (ext == ".wav" || ext == ".mp3" || ext == ".ogg")
        return "[AU]";
    if (ext == ".pyscene" || ext == ".json" || ext == ".yaml")
        return "[SC]";
    return "[--]";
}

// Get file type color for visual distinction
static ImVec4 GetFileColor(const std::string& ext) {
    if (ext == ".py")    return ImVec4(0.3f, 0.7f, 0.4f, 1.0f);  // Green for scripts
    if (ext == ".lua")   return ImVec4(0.2f, 0.4f, 0.9f, 1.0f);  // Blue
    if (ext == ".glsl" || ext == ".vert" || ext == ".frag" || ext == ".spv")
        return ImVec4(0.9f, 0.5f, 0.2f, 1.0f);  // Orange for shaders
    if (ext == ".gltf" || ext == ".glb" || ext == ".obj" || ext == ".fbx")
        return ImVec4(0.6f, 0.3f, 0.9f, 1.0f);  // Purple for models
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
        return ImVec4(0.2f, 0.6f, 0.9f, 1.0f);  // Light blue for textures
    if (ext == ".wav" || ext == ".mp3" || ext == ".ogg")
        return ImVec4(0.9f, 0.8f, 0.2f, 1.0f);  // Yellow for audio
    return ImVec4(0.6f, 0.6f, 0.6f, 1.0f);       // Gray default
}

// Check if a file is openable in an external editor
static bool IsEditableFile(const std::string& ext) {
    return ext == ".py" || ext == ".lua" || ext == ".glsl" || ext == ".vert" ||
           ext == ".frag" || ext == ".json" || ext == ".yaml" || ext == ".txt" ||
           ext == ".cfg" || ext == ".ini" || ext == ".md" || ext == ".pyscene";
}

// Open a file in the system's default text editor
static void OpenInEditor(const fs::path& filepath) {
    std::string command;
#ifdef _WIN32
    command = "start \"\" \"" + filepath.string() + "\"";
#elif __APPLE__
    command = "open \"" + filepath.string() + "\"";
#else
    // Linux: try common editors in order of preference
    command = "xdg-open \"" + filepath.string() + "\" &";
#endif
    std::system(command.c_str());
}

void ProjectPanel::OnImGuiRender() {
    ImGui::Begin("Project");

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

    // Thumbnail size slider
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 120);
    ImGui::PushItemWidth(100);
    ImGui::SliderFloat("##Size", &m_ThumbnailSize, 48.0f, 128.0f, "%.0f");
    ImGui::PopItemWidth();

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

    // Collect and sort entries (directories first, then files alphabetically)
    std::vector<fs::directory_entry> entries;
    for (const auto& entry : fs::directory_iterator(m_CurrentDirectory)) {
        entries.push_back(entry);
    }
    std::sort(entries.begin(), entries.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {
        if (a.is_directory() != b.is_directory())
            return a.is_directory() > b.is_directory();
        return a.path().filename().string() < b.path().filename().string();
    });

    for (const auto& entry : entries) {
        const auto& path = entry.path();
        std::string filename = path.filename().string();
        std::string ext = path.extension().string();

        // Convert extension to lowercase for matching
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        ImGui::PushID(filename.c_str());

        // Icon color based on file type
        if (entry.is_directory()) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.3f, 0.5f, 0.5f));
        } else {
            ImVec4 col = GetFileColor(ext);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(col.x * 0.3f, col.y * 0.3f, col.z * 0.3f, 0.5f));
        }

        // File type label
        const char* icon = entry.is_directory() ? "\xef\x81\xbb" : GetFileIcon(ext);
        ImGui::Button(icon, ImVec2(m_ThumbnailSize, m_ThumbnailSize));
        ImGui::PopStyleColor();

        // Double click behavior
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            if (entry.is_directory()) {
                m_CurrentDirectory = path;
            } else if (IsEditableFile(ext)) {
                // Open script/text files in system editor
                OpenInEditor(path);
            }
        }

        // Tooltip with file info
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", filename.c_str());
            if (!entry.is_directory()) {
                auto fileSize = fs::file_size(path);
                if (fileSize < 1024)
                    ImGui::Text("Size: %zu B", fileSize);
                else if (fileSize < 1024 * 1024)
                    ImGui::Text("Size: %.1f KB", fileSize / 1024.0f);
                else
                    ImGui::Text("Size: %.1f MB", fileSize / (1024.0f * 1024.0f));

                if (IsEditableFile(ext)) {
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Double-click to edit");
                }
            }
            ImGui::EndTooltip();
        }

        // Drag source for files
        if (!entry.is_directory() && ImGui::BeginDragDropSource()) {
            std::string pathStr = path.string();
            ImGui::SetDragDropPayload("ASSET_PATH", pathStr.c_str(), pathStr.size() + 1);
            ImGui::Text("%s", filename.c_str());
            ImGui::EndDragDropSource();
        }

        // Color-coded filename
        if (!entry.is_directory()) {
            ImVec4 textColor = GetFileColor(ext);
            ImGui::PushStyleColor(ImGuiCol_Text, textColor);
        }
        ImGui::TextWrapped("%s", filename.c_str());
        if (!entry.is_directory()) {
            ImGui::PopStyleColor();
        }

        ImGui::NextColumn();
        ImGui::PopID();
    }

    ImGui::Columns(1);
}
