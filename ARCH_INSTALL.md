# Arch Linux Dependency Installation for PyEngine

This document provides the exact commands to install all required dependencies for building and running PyEngine on Arch Linux.

## Required Dependencies

Install all required packages using `pacman`:
```bash
sudo pacman -S --needed \
  base-devel \
  cmake \
  ninja \
  git \
  vulkan-icd-loader \
  vulkan-headers \
  vulkan-validation-layers \
  shaderc \
  spirv-tools \
  spirv-headers \
  glfw-x11 \
  glm \
  fmt \
  spdlog
```

### Package Breakdown

- **base-devel**: Essential build tools (gcc, make, etc.)
- **cmake**: Build system generator (>=3.24 recommended)
- **ninja**: Fast build tool
- **git**: Version control
- **vulkan-icd-loader**: Vulkan runtime loader
- **vulkan-headers**: Vulkan API headers
- **vulkan-validation-layers**: Debug/validation layers for development
- **shaderc**: Google's shader compiler (glslc)
- **spirv-tools**: SPIR-V optimization and validation
- **spirv-headers**: SPIR-V headers
- **glfw-x11**: Windowing library with X11 support
- **glm**: OpenGL Mathematics library (header-only)
- **fmt**: Fast formatting library
- **spdlog**: Fast logging library

## Optional Development Tools

For enhanced debugging and development experience:

```bash
sudo pacman -S --needed \
  renderdoc \
  gdb \
  clang \
  lld \
  ccache
```

### Optional Package Breakdown

- **renderdoc**: Graphics debugger for Vulkan/OpenGL
- **gdb**: GNU debugger
- **clang**: Alternative C++ compiler (faster than GCC in some cases)
- **lld**: LLVM linker (faster than GNU ld)
- **ccache**: Compiler cache to speed up rebuilds

## Vulkan Driver Setup

### NVIDIA

```bash
sudo pacman -S --needed nvidia nvidia-utils vulkan-tools
```

### AMD

```bash
sudo pacman -S --needed mesa vulkan-radeon vulkan-tools
```

### Intel

```bash
sudo pacman -S --needed mesa vulkan-intel vulkan-tools
```

## Verify Installation

Check Vulkan installation:

```bash
vulkaninfo | head -n 30
```

Check available devices:

```bash
vulkaninfo | grep deviceName
```

## Environment Variables (Optional)

For enabling validation layers and debugging:

```bash
# Add to ~/.bashrc or ~/.zshrc
export VK_LAYER_PATH=/usr/share/vulkan/explicit_layer.d
export VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation
```

**Note**: Validation layers are automatically enabled in Debug builds of PyEngine.

## Next Steps

After installing dependencies, see [docs/BUILDING_ARCH.md](docs/BUILDING_ARCH.md) for build instructions.
