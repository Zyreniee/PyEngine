#include "PyEngine/Resources/ResourceManager.hpp"

#include <fstream>
#include <functional>

namespace PyEngine {

void ResourceManager::Initialize(const std::string& projectPath) {
    m_ProjectPath = projectPath;
    m_Stats = {};
}

void ResourceManager::Shutdown() {
    UnloadAll();
    m_PathToID.clear();
    m_Metadata.clear();
}

void ResourceManager::Unload(ResourceID id) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    auto it = m_Resources.find(id);
    if (it != m_Resources.end()) {
        m_Resources.erase(it);
        if (auto* meta = GetMetadata(id)) {
            meta->IsLoaded = false;
            meta->ReferenceCount = 0;
        }
        m_Stats.LoadedResources--;
    }
}

void ResourceManager::UnloadAll() {
    std::lock_guard<std::mutex> lock(m_Mutex);

    m_Resources.clear();
    for (auto& [id, meta] : m_Metadata) {
        meta.IsLoaded = false;
        meta.ReferenceCount = 0;
    }
    m_Stats.LoadedResources = 0;
    m_Stats.TotalMemoryUsage = 0;
}

bool ResourceManager::IsLoaded(const std::string& path) const {
    auto it = m_PathToID.find(path);
    if (it == m_PathToID.end())
        return false;
    return IsLoaded(it->second);
}

bool ResourceManager::IsLoaded(ResourceID id) const {
    auto it = m_Metadata.find(id);
    if (it == m_Metadata.end())
        return false;
    return it->second.IsLoaded;
}

ResourceMetadata* ResourceManager::GetMetadata(ResourceID id) {
    auto it = m_Metadata.find(id);
    return (it != m_Metadata.end()) ? &it->second : nullptr;
}

ResourceMetadata* ResourceManager::GetMetadata(const std::string& path) {
    auto it = m_PathToID.find(path);
    if (it == m_PathToID.end())
        return nullptr;
    return GetMetadata(it->second);
}

void ResourceManager::CheckForChanges() {
    if (!m_HotReloadEnabled)
        return;

    for (auto& [id, meta] : m_Metadata) {
        if (!meta.IsLoaded || meta.FilePath.empty())
            continue;

        try {
            auto currentModTime = std::filesystem::last_write_time(meta.FilePath);
            if (currentModTime > meta.LastModified) {
                meta.LastModified = currentModTime;
                m_Stats.HotReloads++;

                if (m_ReloadCallback) {
                    m_ReloadCallback(id, meta.FilePath);
                }
            }
        } catch (const std::filesystem::filesystem_error&) {
            // File may have been deleted
        }
    }
}

std::string ResourceManager::GetAbsolutePath(const std::string& relativePath) const {
    std::filesystem::path absPath = std::filesystem::path(m_ProjectPath) / relativePath;
    return absPath.string();
}

std::vector<std::string> ResourceManager::GetFilesInDirectory(const std::string& dir, bool recursive) const {
    std::vector<std::string> files;
    std::filesystem::path dirPath = dir;

    if (!std::filesystem::exists(dirPath))
        return files;

    try {
        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath)) {
                if (entry.is_regular_file()) {
                    files.push_back(entry.path().string());
                }
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
                if (entry.is_regular_file()) {
                    files.push_back(entry.path().string());
                }
            }
        }
    } catch (const std::filesystem::filesystem_error&) {
    }

    return files;
}

std::vector<std::string> ResourceManager::GetFilesByType(ResourceType type) const {
    std::vector<std::string> result;
    auto allFiles = GetFilesInDirectory(m_ProjectPath, true);

    for (const auto& file : allFiles) {
        std::filesystem::path p(file);
        if (GetResourceTypeFromExtension(p.extension().string()) == type) {
            result.push_back(file);
        }
    }

    return result;
}

bool ResourceManager::FileExists(const std::string& path) const {
    return std::filesystem::exists(path);
}

ResourceType ResourceManager::GetFileType(const std::string& path) const {
    std::filesystem::path p(path);
    return GetResourceTypeFromExtension(p.extension().string());
}

ResourceID ResourceManager::GenerateID(const std::string& path) {
    // Check if already registered
    auto it = m_PathToID.find(path);
    if (it != m_PathToID.end())
        return it->second;

    ResourceID id = m_NextID++;
    m_PathToID[path] = id;

    // Create metadata
    ResourceMetadata meta;
    meta.ID = id;
    meta.FilePath = path;
    meta.Name = std::filesystem::path(path).filename().string();
    meta.Extension = std::filesystem::path(path).extension().string();

    try {
        meta.LastModified = std::filesystem::last_write_time(path);
        meta.SizeInBytes = std::filesystem::file_size(path);
    } catch (...) {
    }

    m_Metadata[id] = meta;
    m_Stats.TotalResources++;

    return id;
}

}  // namespace PyEngine
