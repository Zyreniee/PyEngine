<div align="center">

<img src="https://upload.wikimedia.org/wikipedia/commons/4/49/Pyrena_Studios.png" alt="PyEngine Logo" width="150" />

# PyEngine

**A Lightweight, High-Performance 3D Engine Architected in C++20 & Vulkan**

<p>
  <img src="https://img.shields.io/badge/C++20-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="C++20 Badge" />
  <img src="https://img.shields.io/badge/Vulkan-C40000?style=for-the-badge&logo=vulkan&logoColor=white" alt="Vulkan Badge" />
  <img src="https://img.shields.io/badge/Python_Scripting-3776AB?style=for-the-badge&logo=python&logoColor=white" alt="Python Badge" />
  <img src="https://img.shields.io/badge/License-MIT-4EAA25?style=for-the-badge" alt="MIT License Badge" />
</p>

*Built for speed, extensibility, and direct hardware control.*

</div>

---

## About PyEngine

**PyEngine** is a custom, open-source 3D game engine built entirely from scratch. Its core philosophy is to provide a highly efficient, modular framework for handling low-level game logic and rendering, without the bloated overhead of commercial engines. 

What sets PyEngine apart is its **hybrid architecture**:
*   **The Engine Core:** Heavy lifting (rendering pipelines, memory management, physics solvers, and systems) is handled strictly by native **C++20** and **Vulkan** for maximum execution speed.
*   **The Scripting Layer:** Gameplay logic, AI systems, and entity behaviors are fully programmable via an intuitive **Embedded Python Scripting** system using **pybind11**. 

It implements a modern Component-Based entity structure, enabling you to attach Python behavioral scripts, physics colliders, and mesh renderers directly to objects in the scene.

---

## Core Features

*   **Native Vulkan Renderer:** High-performance, explicit graphics pipeline designed for maximum GPU throughput, complete with dynamic viewport state, double-buffered frame synchronization (fences & semaphores), and memory management powered by **Vulkan Memory Allocator (VMA)**.
*   **Embedded Python Scripting:** Script your gameplay logic, AI behavior, and system states in Python. Enjoy near-instant iteration times while the underlying C++ framework compiles and runs at maximum machine efficiency.
*   **Component-Based Architecture:** A highly modular scene hierarchy. Seamlessly attach, detach, and inspect components (`Transform`, `Mesh`, `Script`, `Collider`) at runtime.
*   **Event-Driven Input System:** Low-latency keyboard and mouse handling seamlessly bridged between C++ events and the Python API.
*   **Basic Physics & Collisions:** A lightweight physics solver for rigidbodies, raycasting, and collision detection.

---

## Build Instructions

PyEngine is optimized for Linux systems and uses **CMake (v3.24+)** and **Ninja** for rapid compile times.

### 1. Prerequisites (Arch Linux)

Install the required build tools, Vulkan SDK components, Python 3 libraries, and windowing dependencies:

```bash
sudo pacman -S --needed \
  base-devel cmake ninja git \
  vulkan-icd-loader vulkan-headers vulkan-validation-layers \
  shaderc spirv-tools spirv-headers \
  glfw-x11 glm fmt spdlog
```
*(For other distributions, ensure your package manager installs the equivalent Vulkan development SDK, GLFW, GLM, spdlog, and Python development headers).*

### 2. Compiling the Engine

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/zyreniee/PyEngine.git
   cd PyEngine
   ```

2. **Configure with CMake:**
   ```bash
   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
   ```
   *For an optimized build without validation layers and debug assertions, use `-DCMAKE_BUILD_TYPE=Release`.*

3. **Build the Core & Shaders:**
   ```bash
   cmake --build build -j$(nproc)
   ```
   *(This automatically compiles the engine library, sandbox target, and processes GLSL shaders into SPIR-V binaries).*

4. **Run the Sandbox:**
   ```bash
   ./build/bin/PyEngineSandbox
   ```

---

## Current State & Roadmap (WIP)

PyEngine is currently in **Active Development**. While the core rendering loop, Python bindings, and scene systems are functional, the engine is continually evolving as a robust learning and prototyping framework.

### Development Roadmap
- [x] Integrated Entity Component System (ECS) backend using **EnTT** for excellent cache locality.
- [ ] Physically Based Rendering (PBR) pipeline integration.
- [x] Visual Scene Editor using **ImGui** docking framework.
- [ ] Expanded Python API exposing more core systems (Physics/Audio).

For a detailed technical dive, see the [Architecture Guide](docs/ARCHITECTURE.md) and [Roadmap Specification](docs/ROADMAP.md).

---

## The Architect

> [!NOTE]
> **Yusuf Güneş (Zyreniee)**  
> I started engineering **PyEngine** as a vocational high school software development student to deeply understand low-level API design, hardware memory layouts, and how modern engines bridge lightweight scripting with low-level execution.

If you are interested in low-level graphics programming, engine architecture, or graphics engineering, feel free to dive into the codebase!

---

## Contributing

Contributions, architectural discussions, and optimizations are highly welcome! Since the engine is in an active iterative phase, your ideas can play a major role in shaping its future.
*   Found a bug? Open an [Issue](../../issues).
*   Have an optimization or a feature? Submit a [Pull Request](../../pulls).

---

## License

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for details.
