#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace PyEngine {

struct TerrainChunk {
    glm::vec2 Position;  // Grid position (x, z)
    glm::vec3 WorldPosition;
    int LevelOfDetail = 0;
    bool Visible = false;

    // Mesh data would typically be here or referenced
    // For now we just store height data
    std::vector<float> Heights;
    int Width = 0;
    int Depth = 0;
};

class Scene;

class TerrainSystem {
public:
    TerrainSystem();
    ~TerrainSystem();

    void Init();
    void Update(float deltaTime, Scene* scene);
    void Render(Scene* scene);

    void SetHeightFunction(float (*func)(float x, float z));
    void GenerateTerrain(Scene* scene);
    float GetHeightAt(float x, float z);

private:
    void UpdateChunks(Scene* scene, const glm::vec3& viewerPosition);
    void CreateChunk(Scene* scene, const glm::vec2& gridPos);

private:
    int m_ChunkSize = 256;
    int m_MaxChunksVisible = 4;  // View distance in chunks
    float m_Scale = 1.0f;

    std::vector<TerrainChunk> m_Chunks;

    // Procedural generation
    float (*m_HeightFunc)(float x, float z) = nullptr;
};

}  // namespace PyEngine
