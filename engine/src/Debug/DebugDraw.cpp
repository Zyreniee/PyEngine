#include "PyEngine/Debug/DebugDraw.hpp"

#include <algorithm>
#include <cmath>

namespace PyEngine {

void DebugDraw::Line(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color, float duration) {
    if (!m_Enabled)
        return;
    m_Lines.push_back({start, end, color, duration});
}

void DebugDraw::Ray(const glm::vec3& origin, const glm::vec3& direction, float length, const glm::vec4& color,
                    float duration) {
    Line(origin, origin + glm::normalize(direction) * length, color, duration);
}

void DebugDraw::Box(const glm::vec3& center, const glm::vec3& size, const glm::vec4& color, float duration,
                    bool /*filled*/) {
    glm::vec3 half = size * 0.5f;
    WireBox(center - half, center + half, color, duration);
}

void DebugDraw::WireBox(const glm::vec3& min, const glm::vec3& max, const glm::vec4& color, float duration) {
    // Bottom face
    Line({min.x, min.y, min.z}, {max.x, min.y, min.z}, color, duration);
    Line({max.x, min.y, min.z}, {max.x, min.y, max.z}, color, duration);
    Line({max.x, min.y, max.z}, {min.x, min.y, max.z}, color, duration);
    Line({min.x, min.y, max.z}, {min.x, min.y, min.z}, color, duration);

    // Top face
    Line({min.x, max.y, min.z}, {max.x, max.y, min.z}, color, duration);
    Line({max.x, max.y, min.z}, {max.x, max.y, max.z}, color, duration);
    Line({max.x, max.y, max.z}, {min.x, max.y, max.z}, color, duration);
    Line({min.x, max.y, max.z}, {min.x, max.y, min.z}, color, duration);

    // Pillars
    Line({min.x, min.y, min.z}, {min.x, max.y, min.z}, color, duration);
    Line({max.x, min.y, min.z}, {max.x, max.y, min.z}, color, duration);
    Line({max.x, min.y, max.z}, {max.x, max.y, max.z}, color, duration);
    Line({min.x, min.y, max.z}, {min.x, max.y, max.z}, color, duration);
}

void DebugDraw::Sphere(const glm::vec3& center, float radius, const glm::vec4& color, int segments, float duration) {
    WireSphere(center, radius, color, segments, duration);
}

void DebugDraw::WireSphere(const glm::vec3& center, float radius, const glm::vec4& color, int segments,
                           float duration) {
    // Draw 3 circles (XY, XZ, YZ planes)
    Circle(center, radius, {0, 0, 1}, color, segments, duration);  // XY
    Circle(center, radius, {0, 1, 0}, color, segments, duration);  // XZ
    Circle(center, radius, {1, 0, 0}, color, segments, duration);  // YZ
}

void DebugDraw::Circle(const glm::vec3& center, float radius, const glm::vec3& normal, const glm::vec4& color,
                       int segments, float duration) {
    glm::vec3 n = glm::normalize(normal);

    // Find perpendicular vectors
    glm::vec3 right;
    if (std::abs(n.y) < 0.999f) {
        right = glm::normalize(glm::cross(n, glm::vec3(0, 1, 0)));
    } else {
        right = glm::normalize(glm::cross(n, glm::vec3(1, 0, 0)));
    }
    glm::vec3 up = glm::cross(right, n);

    float step = 2.0f * 3.14159265f / static_cast<float>(segments);
    for (int i = 0; i < segments; i++) {
        float a0 = step * static_cast<float>(i);
        float a1 = step * static_cast<float>(i + 1);

        glm::vec3 p0 = center + (right * std::cos(a0) + up * std::sin(a0)) * radius;
        glm::vec3 p1 = center + (right * std::cos(a1) + up * std::sin(a1)) * radius;

        Line(p0, p1, color, duration);
    }
}

void DebugDraw::Capsule(const glm::vec3& start, const glm::vec3& end, float radius, const glm::vec4& color,
                        int segments, float duration) {
    glm::vec3 dir = end - start;
    float height = glm::length(dir);

    if (height < 0.001f) {
        WireSphere(start, radius, color, segments, duration);
        return;
    }

    glm::vec3 n = dir / height;

    // Cylinder body
    Cylinder(start, end, radius, color, segments, duration);

    // Hemisphere caps - simplified as circles
    Circle(start, radius, n, color, segments, duration);
    Circle(end, radius, n, color, segments, duration);
}

void DebugDraw::Cylinder(const glm::vec3& start, const glm::vec3& end, float radius, const glm::vec4& color,
                         int segments, float duration) {
    glm::vec3 dir = glm::normalize(end - start);

    // Find perpendicular vectors
    glm::vec3 right;
    if (std::abs(dir.y) < 0.999f) {
        right = glm::normalize(glm::cross(dir, glm::vec3(0, 1, 0)));
    } else {
        right = glm::normalize(glm::cross(dir, glm::vec3(1, 0, 0)));
    }
    glm::vec3 up = glm::cross(right, dir);

    // Draw circles at both ends
    Circle(start, radius, dir, color, segments, duration);
    Circle(end, radius, dir, color, segments, duration);

    // Connect with vertical lines
    int numLines = std::min(segments, 8);
    float step = 2.0f * 3.14159265f / static_cast<float>(numLines);
    for (int i = 0; i < numLines; i++) {
        float angle = step * static_cast<float>(i);
        glm::vec3 offset = (right * std::cos(angle) + up * std::sin(angle)) * radius;
        Line(start + offset, end + offset, color, duration);
    }
}

void DebugDraw::Cone(const glm::vec3& apex, const glm::vec3& direction, float height, float angle,
                     const glm::vec4& color, int segments, float duration) {
    glm::vec3 dir = glm::normalize(direction);
    glm::vec3 baseCenter = apex + dir * height;
    float baseRadius = height * std::tan(glm::radians(angle));

    // Base circle
    Circle(baseCenter, baseRadius, dir, color, segments, duration);

    // Find perpendicular
    glm::vec3 right;
    if (std::abs(dir.y) < 0.999f) {
        right = glm::normalize(glm::cross(dir, glm::vec3(0, 1, 0)));
    } else {
        right = glm::normalize(glm::cross(dir, glm::vec3(1, 0, 0)));
    }
    glm::vec3 up = glm::cross(right, dir);

    // Lines from apex to base
    int numLines = std::min(segments, 8);
    float step = 2.0f * 3.14159265f / static_cast<float>(numLines);
    for (int i = 0; i < numLines; i++) {
        float a = step * static_cast<float>(i);
        glm::vec3 basePoint = baseCenter + (right * std::cos(a) + up * std::sin(a)) * baseRadius;
        Line(apex, basePoint, color, duration);
    }
}

void DebugDraw::Arrow(const glm::vec3& start, const glm::vec3& end, float headSize, const glm::vec4& color,
                      float duration) {
    Line(start, end, color, duration);

    glm::vec3 dir = glm::normalize(end - start);
    glm::vec3 right;
    if (std::abs(dir.y) < 0.999f) {
        right = glm::normalize(glm::cross(dir, glm::vec3(0, 1, 0)));
    } else {
        right = glm::normalize(glm::cross(dir, glm::vec3(1, 0, 0)));
    }
    glm::vec3 up = glm::cross(right, dir);

    glm::vec3 headBase = end - dir * headSize;
    Line(end, headBase + right * headSize * 0.5f, color, duration);
    Line(end, headBase - right * headSize * 0.5f, color, duration);
    Line(end, headBase + up * headSize * 0.5f, color, duration);
    Line(end, headBase - up * headSize * 0.5f, color, duration);
}

void DebugDraw::Axes(const glm::vec3& position, float size, float duration) {
    Line(position, position + glm::vec3(size, 0, 0), {1, 0, 0, 1}, duration);  // X = red
    Line(position, position + glm::vec3(0, size, 0), {0, 1, 0, 1}, duration);  // Y = green
    Line(position, position + glm::vec3(0, 0, size), {0, 0, 1, 1}, duration);  // Z = blue
}

void DebugDraw::Grid(const glm::vec3& center, float size, int divisions, const glm::vec4& color, float duration) {
    float half = size * 0.5f;
    float step = size / static_cast<float>(divisions);

    for (int i = 0; i <= divisions; i++) {
        float offset = -half + step * static_cast<float>(i);

        // X lines
        Line(center + glm::vec3(-half, 0, offset), center + glm::vec3(half, 0, offset), color, duration);
        // Z lines
        Line(center + glm::vec3(offset, 0, -half), center + glm::vec3(offset, 0, half), color, duration);
    }
}

void DebugDraw::CrossHair(const glm::vec3& position, float size, const glm::vec4& color, float duration) {
    float half = size * 0.5f;
    Line(position - glm::vec3(half, 0, 0), position + glm::vec3(half, 0, 0), color, duration);
    Line(position - glm::vec3(0, half, 0), position + glm::vec3(0, half, 0), color, duration);
    Line(position - glm::vec3(0, 0, half), position + glm::vec3(0, 0, half), color, duration);
}

void DebugDraw::Text3D(const glm::vec3& position, const std::string& text, const glm::vec4& color, float duration) {
    m_Texts3D.push_back({position, text, color, duration});
}

void DebugDraw::Text2D(const glm::vec2& screenPos, const std::string& text, const glm::vec4& color, float duration) {
    m_Texts2D.push_back({screenPos, text, color, duration});
}

void DebugDraw::Frustum(const glm::mat4& viewProjection, const glm::vec4& color, float duration) {
    glm::mat4 inv = glm::inverse(viewProjection);

    glm::vec3 corners[8];
    int i = 0;
    for (float z : {-1.0f, 1.0f}) {
        for (float y : {-1.0f, 1.0f}) {
            for (float x : {-1.0f, 1.0f}) {
                glm::vec4 p = inv * glm::vec4(x, y, z, 1.0f);
                corners[i++] = glm::vec3(p) / p.w;
            }
        }
    }

    // Near face
    Line(corners[0], corners[1], color, duration);
    Line(corners[1], corners[3], color, duration);
    Line(corners[3], corners[2], color, duration);
    Line(corners[2], corners[0], color, duration);

    // Far face
    Line(corners[4], corners[5], color, duration);
    Line(corners[5], corners[7], color, duration);
    Line(corners[7], corners[6], color, duration);
    Line(corners[6], corners[4], color, duration);

    // Edges
    Line(corners[0], corners[4], color, duration);
    Line(corners[1], corners[5], color, duration);
    Line(corners[2], corners[6], color, duration);
    Line(corners[3], corners[7], color, duration);
}

void DebugDraw::AABB(const glm::vec3& min, const glm::vec3& max, const glm::vec4& color, float duration) {
    WireBox(min, max, color, duration);
}

void DebugDraw::BoundingSphere(const glm::vec3& center, float radius, const glm::vec4& color, float duration) {
    WireSphere(center, radius, color, 16, duration);
}

void DebugDraw::Path(const std::vector<glm::vec3>& points, const glm::vec4& color, bool closed, float duration) {
    for (size_t i = 0; i + 1 < points.size(); i++) {
        Line(points[i], points[i + 1], color, duration);
    }
    if (closed && points.size() >= 3) {
        Line(points.back(), points.front(), color, duration);
    }
}

void DebugDraw::Spline(const std::vector<glm::vec3>& controlPoints, const glm::vec4& color, int segments,
                       float duration) {
    if (controlPoints.size() < 4) {
        Path(controlPoints, color, false, duration);
        return;
    }

    // Catmull-Rom spline
    std::vector<glm::vec3> points;
    for (size_t i = 0; i + 3 < controlPoints.size(); i++) {
        for (int s = 0; s <= segments; s++) {
            float t = static_cast<float>(s) / static_cast<float>(segments);
            float t2 = t * t;
            float t3 = t2 * t;

            glm::vec3 p = 0.5f * ((2.0f * controlPoints[i + 1]) + (-controlPoints[i] + controlPoints[i + 2]) * t +
                                  (2.0f * controlPoints[i] - 5.0f * controlPoints[i + 1] + 4.0f * controlPoints[i + 2] -
                                   controlPoints[i + 3]) *
                                      t2 +
                                  (-controlPoints[i] + 3.0f * controlPoints[i + 1] - 3.0f * controlPoints[i + 2] +
                                   controlPoints[i + 3]) *
                                      t3);

            points.push_back(p);
        }
    }

    Path(points, color, false, duration);
}

void DebugDraw::Update(float deltaTime) {
    // Remove expired lines
    m_Lines.erase(std::remove_if(m_Lines.begin(), m_Lines.end(),
                                 [deltaTime](DebugLine& line) {
                                     if (line.RemainingDuration <= 0.0f)
                                         return true;  // One-frame line
                                     line.RemainingDuration -= deltaTime;
                                     return line.RemainingDuration <= 0.0f;
                                 }),
                  m_Lines.end());

    m_Texts3D.erase(std::remove_if(m_Texts3D.begin(), m_Texts3D.end(),
                                   [deltaTime](DebugText3D& text) {
                                       if (text.RemainingDuration <= 0.0f)
                                           return true;
                                       text.RemainingDuration -= deltaTime;
                                       return text.RemainingDuration <= 0.0f;
                                   }),
                    m_Texts3D.end());

    m_Texts2D.erase(std::remove_if(m_Texts2D.begin(), m_Texts2D.end(),
                                   [deltaTime](DebugText2D& text) {
                                       if (text.RemainingDuration <= 0.0f)
                                           return true;
                                       text.RemainingDuration -= deltaTime;
                                       return text.RemainingDuration <= 0.0f;
                                   }),
                    m_Texts2D.end());
}

void DebugDraw::Render() {
    // This would normally submit to the GPU
    // For now, the data is accessible via GetLines/GetTexts for ImGui overlay rendering
}

void DebugDraw::Clear() {
    m_Lines.clear();
    m_Texts3D.clear();
    m_Texts2D.clear();
}

}  // namespace PyEngine
