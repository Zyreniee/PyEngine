#include "EditorLayer.hpp"
#include "PyEngine/Core/Application.hpp"

class PyEngineEditor : public PyEngine::Application {
public:
    PyEngineEditor() : Application({"PyEngine Editor - Pyrena Studios", 1920, 1080}) { PushLayer(new EditorLayer()); }
};

PyEngine::Application* PyEngine::CreateApplication() {
    return new PyEngineEditor();
}

int main(int argc, char** argv) {
    auto app = PyEngine::CreateApplication();
    app->Run();
    delete app;
    return 0;
}
