#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace PyEngine {

struct PhysicsMaterial {
    std::string Name;
    float StaticFriction = 0.6f;
    float DynamicFriction = 0.6f;
    float Bounciness = 0.0f;
    float FrictionCombine = 0.0f;  // 0=Average, 1=Min, 2=Max, 3=Multiply
    float BounceCombine = 0.0f;
};

class PhysicsMaterialLibrary {
public:
    static void Add(const std::string& name, const PhysicsMaterial& mat) {
        GetMaterials()[name] = std::make_shared<PhysicsMaterial>(mat);
    }

    static std::shared_ptr<PhysicsMaterial> Get(const std::string& name) {
        if (GetMaterials().count(name))
            return GetMaterials()[name];
        return nullptr;
    }

private:
    static std::unordered_map<std::string, std::shared_ptr<PhysicsMaterial>>& GetMaterials() {
        static std::unordered_map<std::string, std::shared_ptr<PhysicsMaterial>> materials;
        return materials;
    }
};

}  // namespace PyEngine
