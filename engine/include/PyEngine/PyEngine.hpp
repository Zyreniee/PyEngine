#pragma once

// Core
#include "PyEngine/Core/Application.hpp"
#include "PyEngine/Core/Assert.hpp"
#include "PyEngine/Core/EventSystem.hpp"
#include "PyEngine/Core/Input.hpp"
#include "PyEngine/Core/KeyCodes.h"
#include "PyEngine/Core/Layer.hpp"
#include "PyEngine/Core/LayerSystem.hpp"
#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Core/MouseCodes.h"
#include "PyEngine/Core/ProjectSerializer.hpp"
#include "PyEngine/Core/SystemManager.hpp"
#include "PyEngine/Core/Timestep.hpp"
#include "PyEngine/Core/UUID.hpp"
#include "PyEngine/Core/Utilities.hpp"

// Scene & ECS
#include "PyEngine/Scene/Components.hpp"
#include "PyEngine/Scene/Entity.hpp"
#include "PyEngine/Scene/Scene.hpp"
#include "PyEngine/Scene/SceneSerializer.hpp"
#include "PyEngine/Scene/TerrainSystem.hpp"

// Renderer
#include "PyEngine/Renderer/Buffer.hpp"
#include "PyEngine/Renderer/MaterialLibrary.hpp"
#include "PyEngine/Renderer/MeshGenerator.hpp"
#include "PyEngine/Renderer/OrthographicCamera.hpp"
#include "PyEngine/Renderer/RenderCommand.hpp"
#include "PyEngine/Renderer/Renderer.hpp"
#include "PyEngine/Renderer/Shader.hpp"
#include "PyEngine/Renderer/Texture.hpp"
#include "PyEngine/Renderer/VertexArray.hpp"

// Physics
#include "PyEngine/Physics/PhysicsMaterial.hpp"
#include "PyEngine/Physics/PhysicsSystem.hpp"
#include "PyEngine/Physics/Raycast.hpp"

// Audio
#include "PyEngine/Audio/AudioSystem.hpp"

// Animation
#include "PyEngine/Animation/AnimationSystem.hpp"

// Particles
#include "PyEngine/Particles/ParticleSystem.hpp"

// AI
#include "PyEngine/AI/BehaviorTree.hpp"
#include "PyEngine/AI/NavMeshSystem.hpp"

// Resources
#include "PyEngine/Resources/AssetRegistry.hpp"
#include "PyEngine/Resources/Prefab.hpp"
#include "PyEngine/Resources/ResourceManager.hpp"

// Debug
#include "PyEngine/Debug/DebugRenderer.hpp"
#include "PyEngine/Debug/Profiler.hpp"
#include "PyEngine/Debug/RuntimeConsole.hpp"

// Entry Point
#include "PyEngine/Core/EntryPoint.hpp"
