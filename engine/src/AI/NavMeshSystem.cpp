#include "PyEngine/AI/NavMeshSystem.hpp"

#include <cmath>
#include <queue>
#include <unordered_map>

#include "PyEngine/Assets/Mesh.hpp"
#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Scene/Components.hpp"
#include "PyEngine/Scene/Scene.hpp"

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// NavMesh Implementation
// ═══════════════════════════════════════════════════════════════

void NavMesh::Build(const std::vector<float>& vertices, const std::vector<uint32_t>& indices) {
    m_Triangles.clear();

    // Convert flat mesh data to NavMeshTriangles
    for (size_t i = 0; i < indices.size(); i += 3) {
        NavMeshTriangle tri;
        uint32_t i0 = indices[i];
        uint32_t i1 = indices[i + 1];
        uint32_t i2 = indices[i + 2];

        // Ensure indices are within bounds
        if (i0 * 3 + 2 >= vertices.size() || i1 * 3 + 2 >= vertices.size() || i2 * 3 + 2 >= vertices.size())
            continue;

        tri.A = glm::vec3(vertices[i0 * 3], vertices[i0 * 3 + 1], vertices[i0 * 3 + 2]);
        tri.B = glm::vec3(vertices[i1 * 3], vertices[i1 * 3 + 1], vertices[i1 * 3 + 2]);
        tri.C = glm::vec3(vertices[i2 * 3], vertices[i2 * 3 + 1], vertices[i2 * 3 + 2]);

        tri.Center = (tri.A + tri.B + tri.C) / 3.0f;
        tri.Normal = glm::normalize(glm::cross(tri.B - tri.A, tri.C - tri.A));

        // Slope check: only walkable if normal is roughly up
        if (tri.Normal.y > 0.7f) {
            m_Triangles.push_back(tri);
        }
    }

    // Build adjacency graph (naive O(N^2) for now)
    for (size_t i = 0; i < m_Triangles.size(); ++i) {
        for (size_t j = i + 1; j < m_Triangles.size(); ++j) {
            // Check if triangles share an edge or are close enough
            float dist = glm::distance(m_Triangles[i].Center, m_Triangles[j].Center);
            if (dist < 5.0f) {  // Arbitrary adjacency threshold for this demo
                m_Triangles[i].Neighbors.push_back(j);
                m_Triangles[j].Neighbors.push_back(i);
            }
        }
    }

    PYENGINE_CORE_INFO("NavMesh baked: {} triangles", m_Triangles.size());
}

float NavMesh::Heuristic(const glm::vec3& a, const glm::vec3& b) {
    return glm::distance(a, b);
}

bool NavMesh::FindPath(const glm::vec3& start, const glm::vec3& end, std::vector<glm::vec3>& outPath) {
    if (m_Triangles.empty())
        return false;

    // Find closest start/end triangles
    int startIdx = -1;
    int endIdx = -1;
    float closestStart = std::numeric_limits<float>::max();
    float closestEnd = std::numeric_limits<float>::max();

    for (int i = 0; i < m_Triangles.size(); ++i) {
        float dS = glm::distance(m_Triangles[i].Center, start);
        float dE = glm::distance(m_Triangles[i].Center, end);

        if (dS < closestStart) {
            closestStart = dS;
            startIdx = i;
        }
        if (dE < closestEnd) {
            closestEnd = dE;
            endIdx = i;
        }
    }

    if (startIdx == -1 || endIdx == -1)
        return false;

    // A* Search
    std::priority_queue<std::pair<float, int>, std::vector<std::pair<float, int>>, std::greater<>> frontier;
    frontier.push({0.0f, startIdx});

    std::unordered_map<int, int> cameFrom;
    std::unordered_map<int, float> costSoFar;

    cameFrom[startIdx] = -1;
    costSoFar[startIdx] = 0.0f;

    bool found = false;

    while (!frontier.empty()) {
        int current = frontier.top().second;
        frontier.pop();

        if (current == endIdx) {
            found = true;
            break;
        }

        for (int next : m_Triangles[current].Neighbors) {
            float newCost = costSoFar[current] + glm::distance(m_Triangles[current].Center, m_Triangles[next].Center);

            if (costSoFar.find(next) == costSoFar.end() || newCost < costSoFar[next]) {
                costSoFar[next] = newCost;
                float priority = newCost + Heuristic(m_Triangles[next].Center, end);
                frontier.push({priority, next});
                cameFrom[next] = current;
            }
        }
    }

    if (found) {
        // Reconstruct path
        int curr = endIdx;
        while (curr != -1) {
            outPath.push_back(m_Triangles[curr].Center);
            curr = cameFrom[curr];
        }
        std::reverse(outPath.begin(), outPath.end());
        return true;
    }

    return false;
}

glm::vec3 NavMesh::SamplePoly(const glm::vec3& pos) {
    int bestIdx = -1;
    float bestDist = std::numeric_limits<float>::max();
    for (int i = 0; i < m_Triangles.size(); i++) {
        float d = glm::distance(m_Triangles[i].Center, pos);
        if (d < bestDist) {
            bestDist = d;
            bestIdx = i;
        }
    }
    if (bestIdx != -1)
        return m_Triangles[bestIdx].Center;
    return pos;
}

// ═══════════════════════════════════════════════════════════════
// NavMeshSystem Implementation
// ═══════════════════════════════════════════════════════════════

NavMeshSystem::NavMeshSystem() {
    m_NavMesh = std::make_shared<NavMesh>();
}

NavMeshSystem::~NavMeshSystem() {}

void NavMeshSystem::Init() {
    PYENGINE_CORE_INFO("NavMesh System Initialized");
}

void NavMeshSystem::Update(float deltaTime, Scene* scene) {
    if (!scene)
        return;

    // Assuming we have a NavMeshAgent component in Scene/Components.hpp
    auto view = scene->GetRegistry().view<NavMeshAgentComponent, TransformComponent>();
    for (auto entity : view) {
        auto& agent = view.get<NavMeshAgentComponent>(entity);
        auto& transform = view.get<TransformComponent>(entity);

        // Path following
        if (!agent.Path.empty() && agent.CurrentPathIndex < agent.Path.size()) {
            glm::vec3 target = agent.Path[agent.CurrentPathIndex];
            glm::vec3 dir = target - transform.Position;
            float dist = glm::length(dir);

            if (dist < 0.1f) {
                agent.CurrentPathIndex++;
            } else {
                dir = glm::normalize(dir);
                transform.Position += dir * agent.Speed * deltaTime;

                // Rotate to face movement
                if (glm::length(dir) > 0.001f) {
                    // quaternion rotation logic would go here
                    // For now just set rotation y (yaw)
                    float yaw = glm::degrees(std::atan2(dir.x, dir.z));
                    transform.Rotation.y = yaw;
                }
            }
        }
    }
}

void NavMeshSystem::BakeNavMesh(Scene* scene) {
    if (!scene)
        return;

    std::vector<float> aggregatedVertices;
    std::vector<uint32_t> aggregatedIndices;
    uint32_t indexOffset = 0;

    auto view = scene->GetRegistry().view<MeshRendererComponent, TransformComponent>();
    for (auto entity : view) {
        auto [mrc, transform] = view.get<MeshRendererComponent, TransformComponent>(entity);

        // Skip non-static or invisible objects if desired, for now bake everything
        if (!mrc.Visible)
            continue;

        // Retrieve Mesh
        // TODO: MeshID system is currently a simple int. We need a way to get the actual Mesh* from ResourceManager or
        // similar. For now, let's assume valid ID 0-4 are primitives and we might need to recreate them or access a
        // global mesh cache. Since we don't have a central MeshManager accessible here easily (it's in Scene or
        // Application?), AND Scene::Render creates meshes lazily... this is tricky.

        // Wait, Scene.cpp has m_Meshes or similar? No, Scene::Render creates them locally static or member?
        // Let's check Scene.cpp again. Scene::Render has:
        // static std::shared_ptr<Mesh> cubeMesh = ...
        // This is bad for access.

        // However, MeshRendererComponent now has `std::shared_ptr<Mesh> CustomMesh`.
        // If it's a primitive (MeshID), we might not have the mesh instance easily accessible unless we standardise it.

        std::shared_ptr<Mesh> mesh = mrc.CustomMesh;

        // If CustomMesh is null, we check primitives.
        // We can't easily get the primitive meshes if they are hidden in Scene::Render.
        // LIMITATION: For this phase, we will only bake entities that have a CustomMesh assigned, OR we strictly assume
        // the user has assigned a mesh to CustomMesh for terrain.
        // Actually, the TerrainSystem creates entities with CustomMesh. So Terrain baking should work!
        // For standard primitives (Cube etc) created via menu, they use MeshID.
        // We should really centralize primitive meshes.

        if (!mesh) {
            // Try to resolve primitive if possible, or skip
            // For now, skip MeshID entities to avoid crash, unless we fix Scene architecture.
            continue;
        }

        const auto& vertices = mesh->GetVertices();
        const auto& indices = mesh->GetIndices();
        glm::mat4 modelMatrix = transform.GetTransformMatrix();
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));

        for (const auto& v : vertices) {
            glm::vec4 worldPos = modelMatrix * glm::vec4(v.Position, 1.0f);
            aggregatedVertices.push_back(worldPos.x);
            aggregatedVertices.push_back(worldPos.y);
            aggregatedVertices.push_back(worldPos.z);
        }

        for (uint32_t idx : indices) {
            aggregatedIndices.push_back(idx + indexOffset);
        }

        indexOffset += (uint32_t)vertices.size();
    }

    if (aggregatedVertices.empty()) {
        PYENGINE_CORE_WARN("NavMesh Bake: No geometry found (ensure objects have CustomMesh assigned).");
        return;
    }

    m_NavMesh->Build(aggregatedVertices, aggregatedIndices);
    PYENGINE_CORE_INFO("NavMesh Baked with {} vertices and {} indices.", aggregatedVertices.size() / 3,
                       aggregatedIndices.size());
}

}  // namespace PyEngine
