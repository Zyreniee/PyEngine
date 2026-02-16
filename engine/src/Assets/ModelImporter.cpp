#include "PyEngine/Assets/ModelImporter.hpp"

#include "PyEngine/Assets/Mesh.hpp"
#include "PyEngine/Core/Application.hpp"
#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Renderer/Renderer.hpp"
#include "PyEngine/Scene/Components.hpp"
#include "PyEngine/Scene/Entity.hpp"
#include "PyEngine/Scene/Scene.hpp"

// Define these only in ONE .cpp file
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// tinygltf include
#include <tiny_gltf.h>

namespace PyEngine {

void ModelImporter::ImportGLTF(const std::string& path, Scene* scene) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);
    // If failed, try binary
    if (!ret) {
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);
    }

    if (!warn.empty()) {
        PYENGINE_CORE_WARN("GLTF Warning: {}", warn);
    }

    if (!err.empty()) {
        PYENGINE_CORE_ERROR("GLTF Error: {}", err);
    }

    if (!ret) {
        PYENGINE_CORE_ERROR("Failed to load GLTF: {}", path);
        return;
    }

    PYENGINE_CORE_INFO("Loaded GLTF: {}", path);

    // Iterate over meshes
    for (const auto& mesh : model.meshes) {
        // For simplicity, we only process the first primitive of each mesh for now
        if (mesh.primitives.empty())
            continue;

        const auto& primitive = mesh.primitives[0];

        // Extract attributes
        // Require POSITION
        if (primitive.attributes.find("POSITION") == primitive.attributes.end())
            continue;

        const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
        const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
        const tinygltf::Buffer& posBuffer = model.buffers[posView.buffer];

        const float* positions =
            reinterpret_cast<const float*>(&posBuffer.data[posView.byteOffset + posAccessor.byteOffset]);

        std::vector<Vertex> vertices;
        size_t vertexCount = posAccessor.count;

        // Normals
        const float* normals = nullptr;
        if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
            const tinygltf::Accessor& normAccessor = model.accessors[primitive.attributes.at("NORMAL")];
            const tinygltf::BufferView& normView = model.bufferViews[normAccessor.bufferView];
            const tinygltf::Buffer& normBuffer = model.buffers[normView.buffer];
            normals = reinterpret_cast<const float*>(&normBuffer.data[normView.byteOffset + normAccessor.byteOffset]);
        }

        // UVs
        const float* uvs = nullptr;
        if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
            const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
            const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
            const tinygltf::Buffer& uvBuffer = model.buffers[uvView.buffer];
            uvs = reinterpret_cast<const float*>(&uvBuffer.data[uvView.byteOffset + uvAccessor.byteOffset]);
        }

        for (size_t i = 0; i < vertexCount; ++i) {
            Vertex v;
            v.Position = glm::vec3(positions[i * 3 + 0], positions[i * 3 + 1], positions[i * 3 + 2]);

            if (normals) {
                v.Normal = glm::vec3(normals[i * 3 + 0], normals[i * 3 + 1], normals[i * 3 + 2]);
            } else {
                v.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
            }

            if (uvs) {
                v.TexCoord = glm::vec2(uvs[i * 2 + 0], uvs[i * 2 + 1]);
            } else {
                v.TexCoord = glm::vec2(0.0f, 0.0f);
            }

            vertices.push_back(v);
        }

        // Indices
        std::vector<uint32_t> indices;
        if (primitive.indices >= 0) {
            const tinygltf::Accessor& frontAccessor = model.accessors[primitive.indices];
            const tinygltf::BufferView& frontView = model.bufferViews[frontAccessor.bufferView];
            const tinygltf::Buffer& buffer = model.buffers[frontView.buffer];

            // indices can be byte, short, or int
            if (frontAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                const uint16_t* buf =
                    reinterpret_cast<const uint16_t*>(&buffer.data[frontView.byteOffset + frontAccessor.byteOffset]);
                for (size_t i = 0; i < frontAccessor.count; ++i) {
                    indices.push_back(buf[i]);
                }
            } else if (frontAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                const uint32_t* buf =
                    reinterpret_cast<const uint32_t*>(&buffer.data[frontView.byteOffset + frontAccessor.byteOffset]);
                for (size_t i = 0; i < frontAccessor.count; ++i) {
                    indices.push_back(buf[i]);
                }
            } else if (frontAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                const uint8_t* buf =
                    reinterpret_cast<const uint8_t*>(&buffer.data[frontView.byteOffset + frontAccessor.byteOffset]);
                for (size_t i = 0; i < frontAccessor.count; ++i) {
                    indices.push_back(buf[i]);
                }
            }
        }

        // Create Entity
        auto entity = scene->CreateEntity(mesh.name.empty() ? "Imported Mesh" : mesh.name);
        auto& app = Application::Get();

        // Create Mesh
        std::shared_ptr<Mesh> engineMesh =
            std::make_shared<Mesh>(app.GetRenderer().GetContext(), app.GetRenderer().GetAllocator(), vertices, indices);

        auto& mrc = entity.AddComponent<MeshRendererComponent>();
        mrc.CustomMesh = engineMesh;
        mrc.MeshID = -1;  // Use custom mesh
    }
}
}  // namespace PyEngine
