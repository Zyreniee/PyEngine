#pragma once

#include <array>
#include <bitset>
#include <string>

namespace PyEngine {

class LayerSystem {
public:
    static LayerSystem& Get() {
        static LayerSystem instance;
        return instance;
    }

    void SetLayerName(int index, const std::string& name) {
        if (index >= 0 && index < 32)
            m_LayerNames[index] = name;
    }

    const std::string& GetLayerName(int index) const {
        if (index >= 0 && index < 32)
            return m_LayerNames[index];
        static std::string unknown = "Unknown";
        return unknown;
    }

    // Collision Matrix
    void SetCollision(int layerA, int layerB, bool canCollide) {
        if (layerA < 0 || layerA >= 32 || layerB < 0 || layerB >= 32)
            return;
        m_CollisionMatrix[layerA][layerB] = canCollide;
        m_CollisionMatrix[layerB][layerA] = canCollide;
    }

    bool CanCollide(int layerA, int layerB) const {
        if (layerA < 0 || layerA >= 32 || layerB < 0 || layerB >= 32)
            return true;
        return m_CollisionMatrix[layerA][layerB];
    }

private:
    LayerSystem() {
        // Default names
        m_LayerNames[0] = "Default";
        m_LayerNames[1] = "Transparent";
        m_LayerNames[2] = "Ignore Raycast";
        m_LayerNames[3] = "Water";
        m_LayerNames[4] = "UI";

        // Default collision: everything collides
        for (int i = 0; i < 32; i++)
            m_CollisionMatrix[i].set();
    }

    std::array<std::string, 32> m_LayerNames;
    std::array<std::bitset<32>, 32> m_CollisionMatrix;
};

}  // namespace PyEngine
