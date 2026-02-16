#pragma once

#include <algorithm>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace PyEngine {

class Scene;

struct NavMeshTriangle {
    glm::vec3 A, B, C;
    glm::vec3 Center;
    glm::vec3 Normal;
    std::vector<int> Neighbors;  // Indices of neighbor triangles
};

class NavMesh {
public:
    void Build(const std::vector<float>& vertices, const std::vector<uint32_t>& indices);
    bool FindPath(const glm::vec3& start, const glm::vec3& end, std::vector<glm::vec3>& outPath);
    glm::vec3 SamplePoly(const glm::vec3& pos);

    const std::vector<NavMeshTriangle>& GetTriangles() const { return m_Triangles; }

private:
    std::vector<NavMeshTriangle> m_Triangles;
    float Heuristic(const glm::vec3& a, const glm::vec3& b);
};

class NavMeshSystem {
public:
    NavMeshSystem();
    ~NavMeshSystem();

    void Init();
    void Update(float deltaTime, Scene* scene);

    void BakeNavMesh(Scene* scene);
    std::shared_ptr<NavMesh> GetNavMesh() { return m_NavMesh; }

private:
    std::shared_ptr<NavMesh> m_NavMesh;
};

}  // namespace PyEngine
