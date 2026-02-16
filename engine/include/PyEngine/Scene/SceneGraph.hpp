#pragma once

#include <algorithm>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// SceneNode — Hierarchical node in the scene graph
// ═══════════════════════════════════════════════════════════════
class SceneNode : public std::enable_shared_from_this<SceneNode> {
public:
    SceneNode(const std::string& name = "Node");
    ~SceneNode() = default;

    // Identity
    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }
    uint32_t GetID() const { return m_ID; }
    bool IsActive() const { return m_Active; }
    void SetActive(bool active) { m_Active = active; }

    // ── Local transform ──────────────────────────────────────
    const glm::vec3& GetLocalPosition() const { return m_LocalPosition; }
    void SetLocalPosition(const glm::vec3& pos) {
        m_LocalPosition = pos;
        m_Dirty = true;
    }
    const glm::vec3& GetLocalRotation() const { return m_LocalEulerAngles; }
    void SetLocalRotation(const glm::vec3& euler) {
        m_LocalEulerAngles = euler;
        m_Dirty = true;
    }
    const glm::quat& GetLocalOrientation() const { return m_LocalOrientation; }
    void SetLocalOrientation(const glm::quat& quat) {
        m_LocalOrientation = quat;
        m_Dirty = true;
    }
    const glm::vec3& GetLocalScale() const { return m_LocalScale; }
    void SetLocalScale(const glm::vec3& scale) {
        m_LocalScale = scale;
        m_Dirty = true;
    }

    void Translate(const glm::vec3& delta) {
        m_LocalPosition += delta;
        m_Dirty = true;
    }
    void Rotate(const glm::vec3& eulerDelta) {
        m_LocalEulerAngles += eulerDelta;
        m_Dirty = true;
    }
    void Scale(const glm::vec3& scaleDelta) {
        m_LocalScale *= scaleDelta;
        m_Dirty = true;
    }

    // ── World transform ──────────────────────────────────────
    glm::vec3 GetWorldPosition() const;
    glm::quat GetWorldOrientation() const;
    glm::vec3 GetWorldScale() const;
    const glm::mat4& GetLocalTransformMatrix() const;
    const glm::mat4& GetWorldTransformMatrix() const;

    glm::vec3 GetForward() const;
    glm::vec3 GetRight() const;
    glm::vec3 GetUp() const;

    void LookAt(const glm::vec3& target, const glm::vec3& up = {0, 1, 0});

    // ── Hierarchy ────────────────────────────────────────────
    std::shared_ptr<SceneNode> GetParent() const { return m_Parent.lock(); }
    const std::vector<std::shared_ptr<SceneNode>>& GetChildren() const { return m_Children; }
    size_t GetChildCount() const { return m_Children.size(); }

    void AddChild(std::shared_ptr<SceneNode> child);
    void RemoveChild(std::shared_ptr<SceneNode> child);
    void RemoveAllChildren();
    void SetParent(std::shared_ptr<SceneNode> parent);
    void DetachFromParent();

    std::shared_ptr<SceneNode> FindChild(const std::string& name, bool recursive = true) const;
    std::shared_ptr<SceneNode> FindByID(uint32_t id, bool recursive = true) const;

    bool IsDescendantOf(const SceneNode* ancestor) const;
    int GetDepth() const;

    // ── Traversal ────────────────────────────────────────────
    void TraverseDepthFirst(const std::function<void(SceneNode& node)>& visitor);
    void TraverseBreadthFirst(const std::function<void(SceneNode& node)>& visitor);

    // ── Tags & Layers ────────────────────────────────────────
    const std::string& GetTag() const { return m_Tag; }
    void SetTag(const std::string& tag) { m_Tag = tag; }
    uint32_t GetLayer() const { return m_Layer; }
    void SetLayer(uint32_t layer) { m_Layer = layer; }

    // Force matrix recalculation
    void SetDirty();

private:
    void UpdateTransformMatrix() const;
    void MarkChildrenDirty();

private:
    static uint32_t s_NextID;

    std::string m_Name;
    uint32_t m_ID;
    bool m_Active = true;
    std::string m_Tag = "Untagged";
    uint32_t m_Layer = 0;

    // Local transform
    glm::vec3 m_LocalPosition{0.0f};
    glm::vec3 m_LocalEulerAngles{0.0f};
    glm::quat m_LocalOrientation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 m_LocalScale{1.0f};

    // Cached matrices
    mutable glm::mat4 m_LocalMatrix{1.0f};
    mutable glm::mat4 m_WorldMatrix{1.0f};
    mutable bool m_Dirty = true;

    // Hierarchy
    std::weak_ptr<SceneNode> m_Parent;
    std::vector<std::shared_ptr<SceneNode>> m_Children;
};

// ═══════════════════════════════════════════════════════════════
// SceneGraph — Root container
// ═══════════════════════════════════════════════════════════════
class SceneGraph {
public:
    SceneGraph();

    std::shared_ptr<SceneNode>& GetRoot() { return m_Root; }
    const std::shared_ptr<SceneNode>& GetRoot() const { return m_Root; }

    std::shared_ptr<SceneNode> CreateNode(const std::string& name = "Node");
    void DestroyNode(std::shared_ptr<SceneNode> node);

    std::shared_ptr<SceneNode> FindNode(const std::string& name) const;
    std::shared_ptr<SceneNode> FindNodeByID(uint32_t id) const;
    std::vector<std::shared_ptr<SceneNode>> FindNodesByTag(const std::string& tag) const;
    std::vector<std::shared_ptr<SceneNode>> FindNodesByLayer(uint32_t layer) const;

    void Update();
    size_t GetNodeCount() const;

    void Clear();

private:
    std::shared_ptr<SceneNode> m_Root;
};

}  // namespace PyEngine
