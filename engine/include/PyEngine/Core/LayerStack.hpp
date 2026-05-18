#pragma once

#include <memory>
#include <vector>

namespace PyEngine {

class Layer;

class LayerStack {
public:
    LayerStack() = default;
    ~LayerStack();

    void PushLayer(Layer* layer);
    void PushOverlay(Layer* overlay);
    void PopLayer(Layer* layer);
    void PopOverlay(Layer* overlay);

    std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
    std::vector<Layer*>::iterator end() { return m_Layers.end(); }
    std::vector<Layer*>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
    std::vector<Layer*>::reverse_iterator rend() { return m_Layers.rend(); }

private:
    std::vector<Layer*> m_Layers;
    size_t m_LayerInsertIndex = 0;
};

}  // namespace PyEngine
