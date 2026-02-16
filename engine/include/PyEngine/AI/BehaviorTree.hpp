#pragma once

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// AI Behavior Tree System
// ═══════════════════════════════════════════════════════════════

enum class NodeStatus { Success, Failure, Running };

class Blackboard {
public:
    void SetBool(const std::string& key, bool value) { m_Bools[key] = value; }
    bool GetBool(const std::string& key) { return m_Bools.count(key) ? m_Bools[key] : false; }

    void SetFloat(const std::string& key, float value) { m_Floats[key] = value; }
    float GetFloat(const std::string& key) { return m_Floats.count(key) ? m_Floats[key] : 0.0f; }

    void SetInt(const std::string& key, int value) { m_Ints[key] = value; }
    int GetInt(const std::string& key) { return m_Ints.count(key) ? m_Ints[key] : 0; }

    void SetVector3(const std::string& key, const glm::vec3& value) { m_Vectors[key] = value; }
    glm::vec3 GetVector3(const std::string& key) { return m_Vectors.count(key) ? m_Vectors[key] : glm::vec3(0.0f); }

    void SetEntity(const std::string& key, uint32_t value) { m_Entities[key] = value; }
    uint32_t GetEntity(const std::string& key) { return m_Entities.count(key) ? m_Entities[key] : 0; }

    bool HasKey(const std::string& key) {
        return m_Bools.count(key) || m_Floats.count(key) || m_Ints.count(key) || m_Vectors.count(key) ||
               m_Entities.count(key);
    }

private:
    std::unordered_map<std::string, bool> m_Bools;
    std::unordered_map<std::string, float> m_Floats;
    std::unordered_map<std::string, int> m_Ints;
    std::unordered_map<std::string, glm::vec3> m_Vectors;
    std::unordered_map<std::string, uint32_t> m_Entities;
};

class BehaviorNode {
public:
    virtual ~BehaviorNode() = default;
    virtual NodeStatus Tick(float deltaTime) = 0;

    void SetBlackboard(std::shared_ptr<Blackboard> bb) { m_Blackboard = bb; }

protected:
    std::shared_ptr<Blackboard> m_Blackboard;
};

// ── Composites ──────────────────────────────────────────────

class CompositeNode : public BehaviorNode {
public:
    void AddChild(std::shared_ptr<BehaviorNode> child) {
        child->SetBlackboard(m_Blackboard);
        m_Children.push_back(child);
    }

protected:
    std::vector<std::shared_ptr<BehaviorNode>> m_Children;
};

class Selector : public CompositeNode {
public:
    NodeStatus Tick(float deltaTime) override {
        for (auto& child : m_Children) {
            NodeStatus status = child->Tick(deltaTime);
            if (status != NodeStatus::Failure)
                return status;
        }
        return NodeStatus::Failure;
    }
};

class Sequence : public CompositeNode {
public:
    NodeStatus Tick(float deltaTime) override {
        for (auto& child : m_Children) {
            NodeStatus status = child->Tick(deltaTime);
            if (status != NodeStatus::Success)
                return status;
        }
        return NodeStatus::Success;
    }
};

// ── Decorators ──────────────────────────────────────────────

class DecoratorNode : public BehaviorNode {
public:
    void SetChild(std::shared_ptr<BehaviorNode> child) {
        child->SetBlackboard(m_Blackboard);
        m_Child = child;
    }

protected:
    std::shared_ptr<BehaviorNode> m_Child;
};

class Inverter : public DecoratorNode {
public:
    NodeStatus Tick(float deltaTime) override {
        NodeStatus status = m_Child->Tick(deltaTime);
        if (status == NodeStatus::Success)
            return NodeStatus::Failure;
        if (status == NodeStatus::Failure)
            return NodeStatus::Success;
        return NodeStatus::Running;
    }
};

class Succeeder : public DecoratorNode {
public:
    NodeStatus Tick(float deltaTime) override {
        m_Child->Tick(deltaTime);
        return NodeStatus::Success;
    }
};

class Repeater : public DecoratorNode {
public:
    Repeater(int count = -1) : m_Count(count) {}  // -1 = infinite

    NodeStatus Tick(float deltaTime) override {
        if (m_Count > 0 && m_IterationCount >= m_Count)
            return NodeStatus::Success;

        NodeStatus status = m_Child->Tick(deltaTime);
        if (status != NodeStatus::Running) {
            m_IterationCount++;
            if (m_Count > 0 && m_IterationCount >= m_Count)
                return NodeStatus::Success;
            return NodeStatus::Running;  // Keep running next frame
        }
        return NodeStatus::Running;
    }

private:
    int m_Count;
    int m_IterationCount = 0;
};

// ── Leaf Nodes (Actions/Conditions) ─────────────────────────

class ActionNode : public BehaviorNode {
public:
    using ActionFunc = std::function<NodeStatus(float, Blackboard&)>;

    ActionNode(ActionFunc func) : m_Action(func) {}

    NodeStatus Tick(float deltaTime) override {
        if (m_Action)
            return m_Action(deltaTime, *m_Blackboard);
        return NodeStatus::Failure;
    }

private:
    ActionFunc m_Action;
};

class ConditionNode : public BehaviorNode {
public:
    using ConditionFunc = std::function<bool(Blackboard&)>;

    ConditionNode(ConditionFunc func) : m_Condition(func) {}

    NodeStatus Tick(float deltaTime) override {
        if (m_Condition && m_Condition(*m_Blackboard))
            return NodeStatus::Success;
        return NodeStatus::Failure;
    }

private:
    ConditionFunc m_Condition;
};

class WaitNode : public BehaviorNode {
public:
    WaitNode(float duration) : m_Duration(duration) {}

    NodeStatus Tick(float deltaTime) override {
        m_Elapsed += deltaTime;
        if (m_Elapsed >= m_Duration) {
            m_Elapsed = 0.0f;
            return NodeStatus::Success;
        }
        return NodeStatus::Running;
    }

private:
    float m_Duration;
    float m_Elapsed = 0.0f;
};

// ── Behavior Tree Runner ────────────────────────────────────

class BehaviorTree {
public:
    BehaviorTree() { m_Blackboard = std::make_shared<Blackboard>(); }

    void SetRoot(std::shared_ptr<BehaviorNode> root) {
        m_Root = root;
        m_Root->SetBlackboard(m_Blackboard);
    }

    void Tick(float deltaTime) {
        if (m_Root)
            m_Root->Tick(deltaTime);
    }

    std::shared_ptr<Blackboard> GetBlackboard() { return m_Blackboard; }

private:
    std::shared_ptr<BehaviorNode> m_Root;
    std::shared_ptr<Blackboard> m_Blackboard;
};

}  // namespace PyEngine
