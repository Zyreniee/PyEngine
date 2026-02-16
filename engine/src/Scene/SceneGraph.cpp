#include "PyEngine/Scene/SceneGraph.hpp"

#include <queue>

namespace PyEngine {

uint32_t SceneNode::s_NextID = 1;

SceneNode::SceneNode(const std::string& name) : m_Name(name), m_ID(s_NextID++) {}

glm::vec3 SceneNode::GetWorldPosition() const {
    return glm::vec3(GetWorldTransformMatrix()[3]);
}

glm::quat SceneNode::GetWorldOrientation() const {
    auto parent = m_Parent.lock();
    if (parent) {
        return parent->GetWorldOrientation() * m_LocalOrientation;
    }
    return m_LocalOrientation;
}

glm::vec3 SceneNode::GetWorldScale() const {
    auto parent = m_Parent.lock();
    if (parent) {
        return parent->GetWorldScale() * m_LocalScale;
    }
    return m_LocalScale;
}

const glm::mat4& SceneNode::GetLocalTransformMatrix() const {
    if (m_Dirty) {
        UpdateTransformMatrix();
    }
    return m_LocalMatrix;
}

const glm::mat4& SceneNode::GetWorldTransformMatrix() const {
    if (m_Dirty) {
        UpdateTransformMatrix();
    }
    return m_WorldMatrix;
}

glm::vec3 SceneNode::GetForward() const {
    return glm::normalize(glm::vec3(GetWorldTransformMatrix()[2]));
}

glm::vec3 SceneNode::GetRight() const {
    return glm::normalize(glm::vec3(GetWorldTransformMatrix()[0]));
}

glm::vec3 SceneNode::GetUp() const {
    return glm::normalize(glm::vec3(GetWorldTransformMatrix()[1]));
}

void SceneNode::LookAt(const glm::vec3& target, const glm::vec3& up) {
    glm::vec3 worldPos = GetWorldPosition();
    glm::vec3 direction = glm::normalize(target - worldPos);

    if (glm::length(direction) < 0.001f)
        return;

    glm::mat4 lookMatrix = glm::lookAt(worldPos, target, up);
    m_LocalOrientation = glm::quat_cast(glm::inverse(lookMatrix));
    m_Dirty = true;
}

void SceneNode::AddChild(std::shared_ptr<SceneNode> child) {
    if (!child)
        return;
    if (child.get() == this)
        return;

    // Remove from previous parent
    child->DetachFromParent();

    child->m_Parent = shared_from_this();
    m_Children.push_back(child);
    child->SetDirty();
}

void SceneNode::RemoveChild(std::shared_ptr<SceneNode> child) {
    if (!child)
        return;

    auto it = std::find(m_Children.begin(), m_Children.end(), child);
    if (it != m_Children.end()) {
        (*it)->m_Parent.reset();
        (*it)->SetDirty();
        m_Children.erase(it);
    }
}

void SceneNode::RemoveAllChildren() {
    for (auto& child : m_Children) {
        child->m_Parent.reset();
        child->SetDirty();
    }
    m_Children.clear();
}

void SceneNode::SetParent(std::shared_ptr<SceneNode> parent) {
    if (!parent) {
        DetachFromParent();
        return;
    }
    parent->AddChild(shared_from_this());
}

void SceneNode::DetachFromParent() {
    if (auto parent = m_Parent.lock()) {
        parent->RemoveChild(shared_from_this());
    }
}

std::shared_ptr<SceneNode> SceneNode::FindChild(const std::string& name, bool recursive) const {
    for (const auto& child : m_Children) {
        if (child->GetName() == name)
            return child;
        if (recursive) {
            auto found = child->FindChild(name, true);
            if (found)
                return found;
        }
    }
    return nullptr;
}

std::shared_ptr<SceneNode> SceneNode::FindByID(uint32_t id, bool recursive) const {
    for (const auto& child : m_Children) {
        if (child->GetID() == id)
            return child;
        if (recursive) {
            auto found = child->FindByID(id, true);
            if (found)
                return found;
        }
    }
    return nullptr;
}

bool SceneNode::IsDescendantOf(const SceneNode* ancestor) const {
    auto parent = m_Parent.lock();
    while (parent) {
        if (parent.get() == ancestor)
            return true;
        parent = parent->m_Parent.lock();
    }
    return false;
}

int SceneNode::GetDepth() const {
    int depth = 0;
    auto parent = m_Parent.lock();
    while (parent) {
        depth++;
        parent = parent->m_Parent.lock();
    }
    return depth;
}

void SceneNode::TraverseDepthFirst(const std::function<void(SceneNode&)>& visitor) {
    visitor(*this);
    for (auto& child : m_Children) {
        child->TraverseDepthFirst(visitor);
    }
}

void SceneNode::TraverseBreadthFirst(const std::function<void(SceneNode&)>& visitor) {
    std::queue<SceneNode*> queue;
    queue.push(this);

    while (!queue.empty()) {
        SceneNode* node = queue.front();
        queue.pop();
        visitor(*node);

        for (auto& child : node->m_Children) {
            queue.push(child.get());
        }
    }
}

void SceneNode::SetDirty() {
    m_Dirty = true;
    MarkChildrenDirty();
}

void SceneNode::UpdateTransformMatrix() const {
    // Build local matrix from TRS
    m_LocalMatrix = glm::translate(glm::mat4(1.0f), m_LocalPosition) * glm::mat4_cast(m_LocalOrientation) *
                    glm::scale(glm::mat4(1.0f), m_LocalScale);

    // Build world matrix
    auto parent = m_Parent.lock();
    if (parent) {
        m_WorldMatrix = parent->GetWorldTransformMatrix() * m_LocalMatrix;
    } else {
        m_WorldMatrix = m_LocalMatrix;
    }

    m_Dirty = false;
}

void SceneNode::MarkChildrenDirty() {
    for (auto& child : m_Children) {
        child->m_Dirty = true;
        child->MarkChildrenDirty();
    }
}

// ── SceneGraph ───────────────────────────────────────────────
SceneGraph::SceneGraph() {
    m_Root = std::make_shared<SceneNode>("Root");
}

std::shared_ptr<SceneNode> SceneGraph::CreateNode(const std::string& name) {
    auto node = std::make_shared<SceneNode>(name);
    m_Root->AddChild(node);
    return node;
}

void SceneGraph::DestroyNode(std::shared_ptr<SceneNode> node) {
    if (!node)
        return;
    node->RemoveAllChildren();
    node->DetachFromParent();
}

std::shared_ptr<SceneNode> SceneGraph::FindNode(const std::string& name) const {
    return m_Root->FindChild(name, true);
}

std::shared_ptr<SceneNode> SceneGraph::FindNodeByID(uint32_t id) const {
    return m_Root->FindByID(id, true);
}

std::vector<std::shared_ptr<SceneNode>> SceneGraph::FindNodesByTag(const std::string& tag) const {
    std::vector<std::shared_ptr<SceneNode>> result;
    m_Root->TraverseDepthFirst([&](SceneNode& node) {
        if (node.GetTag() == tag) {
            // Can't get shared_from_this from const ref easily, so use FindByID
            auto found = m_Root->FindByID(node.GetID(), true);
            if (found)
                result.push_back(found);
        }
    });
    return result;
}

std::vector<std::shared_ptr<SceneNode>> SceneGraph::FindNodesByLayer(uint32_t layer) const {
    std::vector<std::shared_ptr<SceneNode>> result;
    m_Root->TraverseDepthFirst([&](SceneNode& node) {
        if (node.GetLayer() == layer) {
            auto found = m_Root->FindByID(node.GetID(), true);
            if (found)
                result.push_back(found);
        }
    });
    return result;
}

void SceneGraph::Update() {
    m_Root->TraverseDepthFirst([](SceneNode& node) {
        // Forces matrix recalculation by accessing it
        node.GetWorldTransformMatrix();
    });
}

size_t SceneGraph::GetNodeCount() const {
    size_t count = 0;
    const_cast<SceneNode*>(m_Root.get())->TraverseDepthFirst([&](SceneNode&) { count++; });
    return count;
}

void SceneGraph::Clear() {
    m_Root->RemoveAllChildren();
}

}  // namespace PyEngine
