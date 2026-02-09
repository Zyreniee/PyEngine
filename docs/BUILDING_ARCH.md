# Building PyEngine on Arch Linux

Complete build instructions for PyEngine on Arch Linux.

## Prerequisites

### Install Dependencies

See [../ARCH_INSTALL.md](../ARCH_INSTALL.md) for the complete dependency installation guide. Quick summary:

```bash
sudo pacman -S --needed base-devel cmake ninja git \
  vulkan-icd-loader vulkan-headers vulkan-validation-layers \
  shaderc spirv-tools spirv-headers \
  glfw-x11 glm fmt spdlog
```

### Verify Vulkan Installation

```bash
# Check Vulkan is working
vulkaninfo | head -n 30

# Verify your GPU is detected
vulkaninfo | grep deviceName
```

## Building

### 1. Clone the Repository

```bash
cd ~/Belgeler/GitHub
# Repository should already exist at PyEngine/
```

### 2. Configure with CMake

```bash
cd PyEngine
mkdir -p build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
```

Configuration options:
- `-DCMAKE_BUILD_TYPE=Debug` - Build with debug symbols and validation layers
- `-DCMAKE_BUILD_TYPE=Release` - Optimized release build (validation layers disabled)

### 3. Build the Project

```bash
cmake --build build
```

Or for parallel build with all cores:

```bash
cmake --build build -j$(nproc)
```

Build output:
- Engine library: `build/lib/libPyEngineLib.a`
- Sandbox executable: `build/bin/PyEngineSandbox`
- Compiled shaders: `build/bin/shaders/*.spv`

### 4. Run the Sandbox

```bash
./build/bin/PyEngineSandbox
```

## Build Troubleshooting

### Vulkan Not Found

If CMake cannot find Vulkan:

```bash
# Ensure vulkan-headers is installed
sudo pacman -S vulkan-headers

# Set Vulkan SDK path manually (if needed)
export VULKAN_SDK=/usr
```

### glslc Not Found

If shader compilation fails:

```bash
# Install shaderc
sudo pacman -S shaderc

# Verify glslc is in PATH
which glslc
```

### GLFW Not Found

```bash
sudo pacman -S glfw-x11
```

### FetchContent Fails

If FetchContent cannot download dependencies, check your internet connection. The following are downloaded:

- EnTT (v3.13.2)
- ImGui (v1.90.1-docking branch)
- tinygltf (v2.8.21)
- VulkanMemoryAllocator (v3.0.1)

You can also clone these manually to the build directory if needed.

## Environment Variables

### Enable Vulkan Validation Layers

Validation layers are automatically enabled in Debug builds. To manually control:

```bash
# Enable validation layers
export VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation
export VK_LAYER_PATH=/usr/share/vulkan/explicit_layer.d

# Run sandbox
./build/bin/PyEngineSandbox
```

### Vulkan ICD Loader

If you have multiple GPUs:

```bash
# Force specific GPU (example for NVIDIA)
export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/nvidia_icd.json

# Force AMD
export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/radeon_icd.x86_64.json

# Force Intel
export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/intel_icd.x86_64.json
```

## Development Workflow

### Iterative Development

```bash
# Make code changes, then rebuild
cmake --build build

# Run
./build/bin/PyEngineSandbox
```

### Clean Build

```bash
rm -rf build
mkdir -p build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Shader-Only Rebuild

Shaders are automatically recompiled if modified:

```bash
# Modify assets/shaders/basic.vert or basic.frag
cmake --build build
```

## IDE Support

### Compile Commands

`compile_commands.json` is automatically generated in the build directory:

```bash
# For clangd/VSCode
ln -s build/compile_commands.json compile_commands.json
```

### CLion

Open the project root directory directly in CLion. It will auto-detect CMake configuration.

### VSCode

Install C/C++ and CMake Tools extensions. Open project root and select the CMake kit.

## Release Build

For optimized performance:

```bash
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
./build-release/bin/PyEngineSandbox
```

## Next Steps

- See [ARCHITECTURE.md](ARCHITECTURE.md) for engine design
- See [ROADMAP.md](ROADMAP.md) for future features
- Check logs in `PyEngine.log` for debugging

## Common Issues

**Problem**: Window doesn't open
- **Solution**: Check Vulkan drivers are installed for your GPU

**Problem**: Validation errors in console
- **Solution**: These are informational in Debug mode. Switch to Release to disable.

**Problem**: Shaders don't compile
- **Solution**: Ensure `glslc` is in PATH (`which glslc`)

**Problem**: Black screen
- **Solution**: Check logs for Vulkan resource creation errors
