#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// DebugDraw — Immediate mode debug rendering
// ═══════════════════════════════════════════════════════════════
class DebugDraw {
public:
    static DebugDraw& Get() {
        static DebugDraw instance;
        return instance;
    }

    // ── Primitives ───────────────────────────────────────────
    void Line(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = {1, 1, 1, 1},
              float duration = 0.0f);
    void Ray(const glm::vec3& origin, const glm::vec3& direction, float length = 1.0f,
             const glm::vec4& color = {1, 1, 1, 1}, float duration = 0.0f);

    void Box(const glm::vec3& center, const glm::vec3& size, const glm::vec4& color = {1, 1, 1, 1},
             float duration = 0.0f, bool filled = false);
    void WireBox(const glm::vec3& min, const glm::vec3& max, const glm::vec4& color = {1, 1, 1, 1},
                 float duration = 0.0f);

    void Sphere(const glm::vec3& center, float radius, const glm::vec4& color = {1, 1, 1, 1}, int segments = 16,
                float duration = 0.0f);
    void WireSphere(const glm::vec3& center, float radius, const glm::vec4& color = {1, 1, 1, 1}, int segments = 16,
                    float duration = 0.0f);

    void Circle(const glm::vec3& center, float radius, const glm::vec3& normal = {0, 1, 0},
                const glm::vec4& color = {1, 1, 1, 1}, int segments = 32, float duration = 0.0f);

    void Capsule(const glm::vec3& start, const glm::vec3& end, float radius, const glm::vec4& color = {1, 1, 1, 1},
                 int segments = 8, float duration = 0.0f);

    void Cylinder(const glm::vec3& start, const glm::vec3& end, float radius, const glm::vec4& color = {1, 1, 1, 1},
                  int segments = 16, float duration = 0.0f);

    void Cone(const glm::vec3& apex, const glm::vec3& direction, float height, float angle,
              const glm::vec4& color = {1, 1, 1, 1}, int segments = 16, float duration = 0.0f);

    void Arrow(const glm::vec3& start, const glm::vec3& end, float headSize = 0.1f,
               const glm::vec4& color = {1, 1, 1, 1}, float duration = 0.0f);

    // ── Coordinate system ────────────────────────────────────
    void Axes(const glm::vec3& position, float size = 1.0f, float duration = 0.0f);
    void Grid(const glm::vec3& center, float size = 10.0f, int divisions = 10,
              const glm::vec4& color = {0.3f, 0.3f, 0.3f, 0.5f}, float duration = 0.0f);
    void CrossHair(const glm::vec3& position, float size = 0.2f, const glm::vec4& color = {1, 1, 0, 1},
                   float duration = 0.0f);

    // ── Text ─────────────────────────────────────────────────
    void Text3D(const glm::vec3& position, const std::string& text, const glm::vec4& color = {1, 1, 1, 1},
                float duration = 0.0f);
    void Text2D(const glm::vec2& screenPos, const std::string& text, const glm::vec4& color = {1, 1, 1, 1},
                float duration = 0.0f);

    // ── Complex ──────────────────────────────────────────────
    void Frustum(const glm::mat4& viewProjection, const glm::vec4& color = {1, 1, 0, 0.5f}, float duration = 0.0f);
    void AABB(const glm::vec3& min, const glm::vec3& max, const glm::vec4& color = {0, 1, 0, 1}, float duration = 0.0f);
    void BoundingSphere(const glm::vec3& center, float radius, const glm::vec4& color = {0, 1, 1, 1},
                        float duration = 0.0f);

    // ── Path ─────────────────────────────────────────────────
    void Path(const std::vector<glm::vec3>& points, const glm::vec4& color = {1, 1, 1, 1}, bool closed = false,
              float duration = 0.0f);
    void Spline(const std::vector<glm::vec3>& controlPoints, const glm::vec4& color = {1, 0.5f, 0, 1},
                int segments = 32, float duration = 0.0f);

    // ── Update & Render ──────────────────────────────────────
    void Update(float deltaTime);
    void Render();
    void Clear();

    // ── Settings ─────────────────────────────────────────────
    void SetEnabled(bool enabled) { m_Enabled = enabled; }
    bool IsEnabled() const { return m_Enabled; }
    void SetDepthTest(bool depthTest) { m_DepthTest = depthTest; }
    bool GetDepthTest() const { return m_DepthTest; }
    void SetLineWidth(float width) { m_DefaultLineWidth = width; }

    // Stats
    uint32_t GetLineCount() const { return static_cast<uint32_t>(m_Lines.size()); }
    uint32_t GetTextCount() const { return static_cast<uint32_t>(m_Texts3D.size() + m_Texts2D.size()); }

private:
    DebugDraw() = default;

    struct DebugLine {
        glm::vec3 Start, End;
        glm::vec4 Color;
        float RemainingDuration;
    };

    struct DebugText3D {
        glm::vec3 Position;
        std::string Text;
        glm::vec4 Color;
        float RemainingDuration;
    };

    struct DebugText2D {
        glm::vec2 ScreenPos;
        std::string Text;
        glm::vec4 Color;
        float RemainingDuration;
    };

    std::vector<DebugLine> m_Lines;
    std::vector<DebugText3D> m_Texts3D;
    std::vector<DebugText2D> m_Texts2D;

    bool m_Enabled = true;
    bool m_DepthTest = true;
    float m_DefaultLineWidth = 1.0f;
};

}  // namespace PyEngine
