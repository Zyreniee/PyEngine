#pragma once

#include <imgui.h>

#include <filesystem>
#include <string>

class ProjectPanel {
public:
    ProjectPanel();

    void OnImGuiRender();
    void SetBaseDirectory(const std::string& path);

private:
    void DrawDirectoryTree(const std::filesystem::path& path);
    void DrawFileGrid();

    std::filesystem::path m_BaseDirectory;
    std::filesystem::path m_CurrentDirectory;
    float m_ThumbnailSize = 80.0f;
    float m_Padding = 16.0f;
};
