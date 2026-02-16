#pragma once

#include <any>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// ResourceHandle — Type-erased handle to a resource
// ═══════════════════════════════════════════════════════════════
using ResourceID = uint64_t;

template <typename T>
class ResourceHandle {
public:
    ResourceHandle() = default;
    ResourceHandle(ResourceID id, std::shared_ptr<T> ptr) : m_ID(id), m_Resource(ptr) {}

    bool IsValid() const { return m_Resource != nullptr && !m_Resource->expired(); }
    T* Get() const { return m_Resource ? m_Resource.get() : nullptr; }
    T* operator->() const { return Get(); }
    T& operator*() const { return *Get(); }
    operator bool() const { return IsValid(); }
    ResourceID GetID() const { return m_ID; }

private:
    ResourceID m_ID = 0;
    std::shared_ptr<T> m_Resource;
};

// ═══════════════════════════════════════════════════════════════
// ResourceMetadata — Info about a loaded resource
// ═══════════════════════════════════════════════════════════════
struct ResourceMetadata {
    ResourceID ID = 0;
    std::string Name;
    std::string FilePath;
    std::string Extension;
    std::type_index TypeInfo = typeid(void);
    size_t SizeInBytes = 0;
    uint32_t ReferenceCount = 0;
    bool IsLoaded = false;
    bool IsDefault = false;

    std::filesystem::file_time_type LastModified;
    std::filesystem::file_time_type LoadedAt;
};

// ═══════════════════════════════════════════════════════════════
// ResourceType — Known resource types
// ═══════════════════════════════════════════════════════════════
enum class ResourceType {
    Unknown,
    Texture,
    Mesh,
    Material,
    Shader,
    AudioClip,
    AnimationClip,
    Scene,
    Script,
    Font,
    Prefab,
    PhysicsMaterial
};

inline ResourceType GetResourceTypeFromExtension(const std::string& ext) {
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga" || ext == ".hdr")
        return ResourceType::Texture;
    if (ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb")
        return ResourceType::Mesh;
    if (ext == ".mat" || ext == ".material")
        return ResourceType::Material;
    if (ext == ".vert" || ext == ".frag" || ext == ".glsl" || ext == ".spv")
        return ResourceType::Shader;
    if (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac")
        return ResourceType::AudioClip;
    if (ext == ".anim")
        return ResourceType::AnimationClip;
    if (ext == ".scene" || ext == ".json")
        return ResourceType::Scene;
    if (ext == ".py" || ext == ".lua" || ext == ".cs")
        return ResourceType::Script;
    if (ext == ".ttf" || ext == ".otf")
        return ResourceType::Font;
    if (ext == ".prefab")
        return ResourceType::Prefab;
    return ResourceType::Unknown;
}

inline const char* ResourceTypeToString(ResourceType type) {
    switch (type) {
        case ResourceType::Texture:
            return "Texture";
        case ResourceType::Mesh:
            return "Mesh";
        case ResourceType::Material:
            return "Material";
        case ResourceType::Shader:
            return "Shader";
        case ResourceType::AudioClip:
            return "AudioClip";
        case ResourceType::AnimationClip:
            return "AnimationClip";
        case ResourceType::Scene:
            return "Scene";
        case ResourceType::Script:
            return "Script";
        case ResourceType::Font:
            return "Font";
        case ResourceType::Prefab:
            return "Prefab";
        case ResourceType::PhysicsMaterial:
            return "PhysicsMaterial";
        default:
            return "Unknown";
    }
}

// ═══════════════════════════════════════════════════════════════
// ImportSettings — Per-type import configuration
// ═══════════════════════════════════════════════════════════════
struct TextureImportSettings {
    bool GenerateMipmaps = true;
    bool SRGB = true;
    bool FlipVertical = true;
    int MaxSize = 4096;
    enum class FilterMode { Point, Bilinear, Trilinear } Filter = FilterMode::Trilinear;
    enum class WrapMode { Repeat, Clamp, Mirror } Wrap = WrapMode::Repeat;
    enum class TextureType { Default, NormalMap, Sprite, CubeMap, Lightmap } Type = TextureType::Default;
    float AnisotropicLevel = 16.0f;
};

struct MeshImportSettings {
    bool RecalculateNormals = false;
    bool RecalculateTangents = true;
    bool OptimizeMesh = true;
    bool GenerateColliders = false;
    float ScaleFactor = 1.0f;
    bool ImportMaterials = true;
    bool ImportAnimations = true;
    bool SwapYZ = false;
};

struct AudioImportSettings {
    bool ForceToMono = false;
    bool Normalize = true;
    float Quality = 1.0f;
    bool LoadInBackground = false;
    bool PreloadAudioData = true;
    uint32_t TargetSampleRate = 44100;
    enum class CompressionFormat { PCM, Vorbis, ADPCM } Compression = CompressionFormat::Vorbis;
};

// ═══════════════════════════════════════════════════════════════
// ResourceManager — Central asset management system
// ═══════════════════════════════════════════════════════════════
class ResourceManager {
public:
    static ResourceManager& Get() {
        static ResourceManager instance;
        return instance;
    }

    void Initialize(const std::string& projectPath);
    void Shutdown();

    // ── Loading ──────────────────────────────────────────────
    template <typename T>
    ResourceHandle<T> Load(const std::string& path);

    template <typename T>
    ResourceHandle<T> LoadAsync(const std::string& path, std::function<void(ResourceHandle<T>)> callback = nullptr);

    void Unload(ResourceID id);
    void UnloadAll();

    // ── Queries ──────────────────────────────────────────────
    bool IsLoaded(const std::string& path) const;
    bool IsLoaded(ResourceID id) const;
    ResourceMetadata* GetMetadata(ResourceID id);
    ResourceMetadata* GetMetadata(const std::string& path);

    // ── Hot reload ───────────────────────────────────────────
    void EnableHotReload(bool enable) { m_HotReloadEnabled = enable; }
    bool IsHotReloadEnabled() const { return m_HotReloadEnabled; }
    void CheckForChanges();

    using ReloadCallback = std::function<void(ResourceID, const std::string&)>;
    void SetReloadCallback(ReloadCallback cb) { m_ReloadCallback = cb; }

    // ── File system ──────────────────────────────────────────
    void SetProjectPath(const std::string& path) { m_ProjectPath = path; }
    const std::string& GetProjectPath() const { return m_ProjectPath; }
    std::string GetAbsolutePath(const std::string& relativePath) const;

    std::vector<std::string> GetFilesInDirectory(const std::string& dir, bool recursive = false) const;
    std::vector<std::string> GetFilesByType(ResourceType type) const;

    bool FileExists(const std::string& path) const;
    ResourceType GetFileType(const std::string& path) const;

    // ── Stats ────────────────────────────────────────────────
    struct Stats {
        uint32_t TotalResources = 0;
        uint32_t LoadedResources = 0;
        size_t TotalMemoryUsage = 0;
        uint32_t PendingLoads = 0;
        uint32_t HotReloads = 0;
    };
    const Stats& GetStats() const { return m_Stats; }

private:
    ResourceManager() = default;
    ResourceID GenerateID(const std::string& path);

private:
    std::string m_ProjectPath;
    std::unordered_map<ResourceID, ResourceMetadata> m_Metadata;
    std::unordered_map<ResourceID, std::any> m_Resources;
    std::unordered_map<std::string, ResourceID> m_PathToID;

    bool m_HotReloadEnabled = true;
    ReloadCallback m_ReloadCallback;
    mutable std::mutex m_Mutex;
    Stats m_Stats;
    ResourceID m_NextID = 1;
};

}  // namespace PyEngine
