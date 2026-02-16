#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// Mesh Data Structures
// ═══════════════════════════════════════════════════════════════

struct Vertex {
    glm::vec3 Position{0.0f};
    glm::vec3 Normal{0.0f, 1.0f, 0.0f};
    glm::vec2 TexCoords{0.0f};
    glm::vec3 Tangent{1.0f, 0.0f, 0.0f};
    glm::vec3 Bitangent{0.0f, 0.0f, 1.0f};
    glm::vec4 Color{1.0f};
};

struct MeshData {
    std::vector<Vertex> Vertices;
    std::vector<uint32_t> Indices;
    std::string Name;

    uint32_t GetVertexCount() const { return static_cast<uint32_t>(Vertices.size()); }
    uint32_t GetIndexCount() const { return static_cast<uint32_t>(Indices.size()); }
    uint32_t GetTriangleCount() const { return GetIndexCount() / 3; }

    void Clear() {
        Vertices.clear();
        Indices.clear();
    }
    void RecalculateNormals();
    void RecalculateTangents();
    void Merge(const MeshData& other);
    void Transform(const glm::mat4& matrix);
};

// ═══════════════════════════════════════════════════════════════
// MeshGenerator — Procedural mesh generation
// ═══════════════════════════════════════════════════════════════

class MeshGenerator {
public:
    // ── Basic Primitives ─────────────────────────────────────
    static MeshData CreatePlane(float width = 1.0f, float height = 1.0f, int widthSegments = 1, int heightSegments = 1);
    static MeshData CreateCube(float size = 1.0f);
    static MeshData CreateSphere(float radius = 0.5f, int rings = 32, int sectors = 32);
    static MeshData CreateCylinder(float radius = 0.5f, float height = 1.0f, int segments = 32);
    static MeshData CreateCone(float radius = 0.5f, float height = 1.0f, int segments = 32);
    static MeshData CreateCapsule(float radius = 0.5f, float height = 2.0f, int rings = 16, int segments = 32);
    static MeshData CreateTorus(float majorRadius = 1.0f, float minorRadius = 0.3f, int majorSegments = 32,
                                int minorSegments = 16);

    // ── Advanced Primitives ──────────────────────────────────
    static MeshData CreateIcosphere(float radius = 0.5f, int subdivisions = 3);
    static MeshData CreateUVSphere(float radius = 0.5f, int latSegments = 32, int lonSegments = 32);
    static MeshData CreateQuad(float width = 1.0f, float height = 1.0f);
    static MeshData CreateDisc(float radius = 0.5f, int segments = 32);
    static MeshData CreateRing(float innerRadius = 0.3f, float outerRadius = 0.5f, int segments = 32);
    static MeshData CreateArrow(float shaftLength = 0.8f, float shaftRadius = 0.05f, float headLength = 0.2f,
                                float headRadius = 0.1f, int segments = 16);

    // ── Terrain ──────────────────────────────────────────────
    static MeshData CreateHeightmap(const std::vector<float>& heights, int width, int depth, float worldWidth = 100.0f,
                                    float worldHeight = 20.0f, float worldDepth = 100.0f);
    static MeshData CreateFlatTerrain(float width = 100.0f, float depth = 100.0f, int widthSegments = 64,
                                      int depthSegments = 64);
};

}  // namespace PyEngine
