#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Core/UUID.hpp"

namespace PyEngine {

enum class AssetType { None = 0, Texture, Mesh, Audio, Material, Scene, Script, Prefab };

struct AssetMetadata {
    UUID Handle;
    AssetType Type;
    std::filesystem::path FilePath;  // Relative to project root
    bool IsLoaded = false;
};

class AssetRegistry {
public:
    static AssetRegistry& Get() {
        static AssetRegistry instance;
        return instance;
    }

    void RegisterAsset(const std::filesystem::path& path, AssetType type) {
        UUID handle;  // Generate new UUID
        RegisterAsset(handle, path, type);
    }

    void RegisterAsset(UUID handle, const std::filesystem::path& path, AssetType type) {
        if (m_Assets.count(handle)) {
            PYENGINE_CORE_WARN("Asset with handle {} already exists!", (uint64_t)handle);
            return;
        }

        AssetMetadata metadata;
        metadata.Handle = handle;
        metadata.Type = type;
        metadata.FilePath = path;
        metadata.IsLoaded = false;

        m_Assets[handle] = metadata;
    }

    const AssetMetadata& GetMetadata(UUID handle) const {
        static AssetMetadata empty;
        if (m_Assets.count(handle) == 0)
            return empty;
        return m_Assets.at(handle);
    }

    bool HasAsset(UUID handle) const { return m_Assets.count(handle) > 0; }

    void RemoveAsset(UUID handle) { m_Assets.erase(handle); }

    const std::unordered_map<UUID, AssetMetadata>& GetAllAssets() const { return m_Assets; }

private:
    std::unordered_map<UUID, AssetMetadata> m_Assets;

    AssetRegistry() = default;
};

}  // namespace PyEngine
