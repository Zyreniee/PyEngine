#include "PyEngine/Scene/TerrainSystem.hpp"

#include <algorithm>
#include <cmath>

#include "PyEngine/Assets/Mesh.hpp"
#include "PyEngine/Core/Application.hpp"
#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Renderer/Renderer.hpp"
#include "PyEngine/Scene/Components.hpp"
#include "PyEngine/Scene/Entity.hpp"
#include "PyEngine/Scene/Scene.hpp"

namespace PyEngine {

// Simple noise function if none provided
static float DefaultHeightFunc(float x, float z) {
    return std::sin(x * 0.1f) * std::cos(z * 0.1f) * 5.0f + std::sin(x * 0.03f + z * 0.03f) * 10.0f;
}

TerrainSystem::TerrainSystem() {
    m_HeightFunc = DefaultHeightFunc;
}

TerrainSystem::~TerrainSystem() {
    m_Chunks.clear();
}

void TerrainSystem::Init() {
    PYENGINE_CORE_INFO("Terrain System Initialized");
}

void TerrainSystem::Update(float deltaTime, Scene* scene) {
    if (!scene)
        return;

    // Find viewer (main camera)
    glm::vec3 viewerPos(0.0f);
    auto view = scene->GetRegistry().view<TransformComponent, CameraComponent>();
    for (auto entity : view) {
        auto& cc = view.get<CameraComponent>(entity);
        if (cc.IsPrimary) {
            viewerPos = view.get<TransformComponent>(entity).Position;
            break;
        }
    }
    UpdateChunks(scene, viewerPos);
}

void TerrainSystem::Render(Scene* scene) {
    // Entities are rendered by Scene::Render/Renderer
}

void TerrainSystem::SetHeightFunction(float (*func)(float x, float z)) {
    m_HeightFunc = func;
}

void TerrainSystem::GenerateTerrain(Scene* scene) {
    if (scene)
        UpdateChunks(scene, glm::vec3(0.0f));
}

float TerrainSystem::GetHeightAt(float x, float z) {
    if (m_HeightFunc)
        return m_HeightFunc(x, z);
    return 0.0f;
}

void TerrainSystem::UpdateChunks(Scene* scene, const glm::vec3& viewerPosition) {
    int currentChunkX = static_cast<int>(std::round(viewerPosition.x / m_ChunkSize));
    int currentChunkZ = static_cast<int>(std::round(viewerPosition.z / m_ChunkSize));

    // Add new chunks
    for (int zOffset = -m_MaxChunksVisible; zOffset <= m_MaxChunksVisible; zOffset++) {
        for (int xOffset = -m_MaxChunksVisible; xOffset <= m_MaxChunksVisible; xOffset++) {
            glm::vec2 viewChunkCoord = {static_cast<float>(currentChunkX + xOffset),
                                        static_cast<float>(currentChunkZ + zOffset)};

            bool alreadyExists = false;
            for (const auto& chunk : m_Chunks) {
                if (chunk.Position == viewChunkCoord) {
                    alreadyExists = true;
                    break;
                }
            }

            if (!alreadyExists) {
                CreateChunk(scene, viewChunkCoord);
            }
        }
    }
}

void TerrainSystem::CreateChunk(Scene* scene, const glm::vec2& gridPos) {
    if (!scene)
        return;

    TerrainChunk chunk;
    chunk.Position = gridPos;
    chunk.WorldPosition = {gridPos.x * m_ChunkSize, 0.0f, gridPos.y * m_ChunkSize};
    chunk.Width = static_cast<int>(m_ChunkSize) + 1;
    chunk.Depth = static_cast<int>(m_ChunkSize) + 1;
    chunk.Visible = true;

    // Generate height data
    chunk.Heights.resize(chunk.Width * chunk.Depth);
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (int z = 0; z < chunk.Depth; z++) {
        for (int x = 0; x < chunk.Width; x++) {
            float worldX = chunk.WorldPosition.x + x;
            float worldZ = chunk.WorldPosition.z + z;
            float y = GetHeightAt(worldX, worldZ);
            chunk.Heights[z * chunk.Width + x] = y;

            Vertex vertex{};
            vertex.Position = {worldX, y, worldZ};
            vertex.Normal = {0.0f, 1.0f, 0.0f};  // Placeholder normal
            vertex.TexCoord = {static_cast<float>(x), static_cast<float>(z)};
            vertices.push_back(vertex);
        }
    }

    // Indices
    for (int z = 0; z < chunk.Depth - 1; z++) {
        for (int x = 0; x < chunk.Width - 1; x++) {
            int topLeft = (z * chunk.Width) + x;
            int topRight = topLeft + 1;
            int bottomLeft = ((z + 1) * chunk.Width) + x;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    // Create Mesh
    auto& app = Application::Get();
    auto& renderer = app.GetRenderer();

    // Using simple vector-based Mesh constructor we saw earlier or similar.
    // Assuming Mesh constructor takes vectors.
    std::shared_ptr<Mesh> mesh(new Mesh(renderer.GetContext(), renderer.GetAllocator(), vertices, indices));

    // Create Entity
    Entity entity = scene->CreateEntity("Terrain Chunk");
    auto& mrc = entity.AddComponent<MeshRendererComponent>();
    mrc.CustomMesh = mesh;
    mrc.ColorTint = {0.3f, 0.8f, 0.3f, 1.0f};  // Greenish

    auto& tc = entity.GetComponent<TransformComponent>();
    tc.Position = {0.0f, 0.0f, 0.0f};

    m_Chunks.push_back(chunk);
}

}  // namespace PyEngine
