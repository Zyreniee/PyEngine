# PyEngine Architecture

Technical documentation of PyEngine's modular architecture and design decisions.

## Overview

PyEngine is a modern Vulkan-based game engine built with C++20, following clean architecture principles and leveraging modern C++ idioms.

### Design Philosophy

1. **Modularity**: Clear separation between Core, Platform, Renderer, Scene, and Assets
2. **Modern C++20**: Leveraging concepts, ranges, and smart pointers
3. **Data-Oriented**: ECS architecture with EnTT for cache-friendly iteration
4. **Explicitness**: Vulkan's explicit nature is embraced, not hidden

## Module Breakdown

### Core

**Purpose**: Foundation utilities used across the engine

**Components**:
- `Log`: spdlog wrapper with engine/client separation
- `Timer`: High-precision timing for delta time
- `FileSystem`: Cross-platform file I/O utilities
- `Assert`: Debug assertions with logging integration

**Dependencies**: spdlog, fmt, C++20 filesystem

---

### Platform

**Purpose**: Abstract windowing and input across platforms

**Components**:
- `Window`: GLFW window wrapper with Vulkan surface creation
- `Input`: Polling-based input system with mouse delta tracking

**Key Features**:
- Framebuffer resize callbacks
- Vulkan surface compatible
- ESC to exit

**Dependencies**: GLFW, Vulkan

---

### Renderer

**Purpose**: Vulkan rendering backend

**Architecture Flow**:

```
Window → VulkanContext → Swapchain → RenderPass
                ↓
         VMA Allocator → Buffers/Images
                ↓
         Pipeline → Descriptors → Rendering
```

**Components**:

#### VulkanContext
- Instance with validation layers (Debug only)
- Physical device selection (prefers discrete GPU)
- Logical device + queue families
- Surface management

#### Swapchain
- Image acquisition
- Presentation
- Depth buffer
- Framebuffers
- Automatic recreation on resize

#### Pipeline
- Shader module loading (SPIR-V)
- Vertex input layout (position, normal, texcoord)
- Rasterization state
- Depth testing
- Dynamic viewport/scissor

#### Buffer
- VMA-backed buffers
- Vertex/index/uniform buffer types
- Staging buffer transfers

#### Renderer (Main)
- Frame synchronization (fences/semaphores)
- Command buffer management
- Descriptor set allocation/updates
- Uniform buffer per-frame updates
- BeginFrame/EndFrame abstraction

**Vulkan Objects Created**:
- 1 VkInstance
- 1 VkDevice
- 1 VkSwapchainKHR
- 3 VkImage (swapchain - may vary)
- 1 VkRenderPass
- 1 VkPipeline
- 2 VkDescriptorSet (double buffered)
- N VkBuffer (vertex, index, uniform × 2)

**Memory Management**:
- VMA for all buffers and images
- Automatic host-visible mapping for uniform buffers
- GPU-only memory for vertex/index buffers

---

### Scene

**Purpose**: Entity-Component-System game object representation

**Components**:

#### Entity
- EnTT entity wrapper
- Type-safe component access
- Fluent API

#### Components
- `TransformComponent`: Position, rotation (Euler), scale
- `CameraComponent`: Projection, view matrices
- `MeshRendererComponent`: Mesh handle, color

#### Camera
- FPS controller
- WASD + Space/Ctrl movement
- Right-mouse-button look
- Configurable FOV, near/far planes

#### Scene
- EnTT registry ownership
- Entity creation/destruction
- Update loop

**ECS Benefits**:
- Cache-friendly iteration
- Easy serialization (future)
- Excellent performance for many entities

---

### Assets

**Purpose**: Load and manage game assets

**Components**:

#### Mesh
- Vertex buffer (position, normal, UV)
- Index buffer
- Procedural generation (cube)
- glTF loading support (tinygltf)

**Vertex Layout**:
```cpp
struct Vertex {
    glm::vec3 Position;  // Offset: 0
    glm::vec3 Normal;    // Offset: 12
    glm::vec2 TexCoord;  // Offset: 24
}; // Stride: 32 bytes
```

---

### ImGui

**Purpose**: Debug and editor UI

**Features**:
- Docking enabled
- Vulkan backend
- Custom dark theme
- FPS counter
- Camera telemetry
- GPU info

**Integration**:
- Separate descriptor pool
- Font texture upload
- Render in main render pass

---

## Rendering Flow

### Per-Frame Execution

1. **BeginFrame**
   - Wait for fence
   - Acquire swapchain image
   - Reset command buffer
   - Begin render pass

2. **Render**
   - Update uniform buffers (MVP matrices)
   - Bind pipeline
   - Bind descriptor sets
   - Draw mesh (bind vertex/index, issue draw call)
   - ImGui rendering

3. **EndFrame**
   - End render pass
   - Submit command buffer
   - Present swapchain image
   - Handle resize if needed

### Synchronization

- **Fences**: CPU-GPU sync (wait for frame completion)
- **Semaphores**: GPU-GPU sync (image available → render → present)
- **Double buffering**: 2 frames in flight for parallelism

---

## Shader Pipeline

### Compilation

```
GLSL → glslc → SPIR-V → VkShaderModule
```

- CMake custom command compiles on build
- Output: `build/bin/shaders/*.spv`
- Loaded at runtime via `FileSystem::ReadBinaryFile`

### Uniform Data

```glsl
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;
```

Updated per-frame with camera matrices.

---

## Threading Model

**Current**: Single-threaded rendering
**Future**: Multi-threaded command buffer recording

---

## Error Handling

- **Assertions**: Debug-only with logging
- **Validation Layers**: Enabled in Debug, log warnings/errors
- **VkResult checks**: All wrapped with assertions

---

## Build System

### CMake Structure

```
Root CMakeLists.txt
├── FetchContent (EnTT, ImGui, tinygltf, VMA)
├── engine/CMakeLists.txt (PyEngineLib static library)
└── sandbox/CMakeLists.txt (PyEngineSandbox executable)
```

### Shader Compilation Module

`cmake/CompileShaders.cmake` defines `compile_shaders()` function for automatic SPIR-V generation.

---

## Future Architecture

### Planned Improvements

1. **Render Graph**: High-level rendering abstraction
2. **Multi-threading**: Parallel command recording
3. **Asset Streaming**: Async loading
4. **ECS Systems**: Formalized system execution
5. **Material System**: Shader permutations, PBR
6. **GPU Culling**: Compute-based frustum/occlusion culling

See [ROADMAP.md](ROADMAP.md) for details.

---

## Performance Considerations

- **VMA**: Minimizes allocation overhead
- **Descriptor Sets**: Pre-allocated pool
- **Command Buffers**: Reused per-frame
- **Swapchain**: Mailbox mode preferred (triple buffering)

---

## Dependencies Rationale

| Library | Purpose | Why |
|---------|---------|-----|
| Vulkan | Graphics API | Explicit control, modern, cross-platform |
| GLFW | Windowing | Lightweight, Vulkan-native |
| GLM | Math | Header-only, Vulkan-compatible |
| EnTT | ECS | Header-only, fast, modern C++ |
| spdlog | Logging | Fast, customizable |
| ImGui | UI | Immediate mode, easy integration |
| VMA | Memory | Best Vulkan memory management |
| tinygltf | glTF | Header-only, complete glTF 2.0 |

---

## Diagram

```
┌─────────────────────────────────────────┐
│           Application                    │
│         (Sandbox Main Loop)              │
└──────────────┬──────────────────────────┘
               │
      ┌────────┴─────────┐
      │                  │
┌─────▼──────┐    ┌──────▼────────┐
│   Scene    │    │   Renderer    │
│   (ECS)    │    │   (Vulkan)    │
└─────┬──────┘    └──────┬────────┘
      │                  │
┌─────▼──────┐    ┌──────▼────────┐
│   Camera   │    │  VulkanContext│
│   Mesh     │    │   Swapchain   │
│Components  │    │   Pipeline    │
└────────────┘    └───────────────┘
```

---

## Conclusion

PyEngine prioritizes **clarity, modularity, and modern best practices** over premature optimization. The architecture is designed to evolve with features while maintaining clean interfaces.
