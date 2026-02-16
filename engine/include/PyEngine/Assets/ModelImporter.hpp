#pragma once

#include <string>

namespace PyEngine {
class Scene;

class ModelImporter {
public:
    static void ImportGLTF(const std::string& path, Scene* scene);
};
}  // namespace PyEngine
