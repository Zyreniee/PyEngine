cmake_minimum_required(VERSION 3.24)
project(PyEngine VERSION 0.1.0 LANGUAGES CXX)

# C++20 Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Global compile options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -w
    )
endif()

# Find required packages
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# FetchContent for third-party libraries
include(FetchContent)

# EnTT (Entity Component System)
FetchContent_Declare(
    EnTT
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.13.2
)

# ImGui with docking
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.1-docking
)

# tinygltf for glTF loading
FetchContent_Declare(
    tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
    GIT_TAG v2.8.21
)

# VulkanMemoryAllocator (header-only — we provide our own implementation in vma_impl.cpp)
FetchContent_Declare(
    VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG v3.0.1
)

# nlohmann/json for scene serialization
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/archive/refs/tags/v3.11.3.tar.gz
)

# pybind11 for Python scripting
FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v2.12.0
)

# Disable VMA building its own library (we provide our own implementation in vma_impl.cpp)
set(VMA_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
set(VMA_BUILD_SAMPLE OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(EnTT tinygltf json pybind11)

# VMA: only fetch, don't build its library (we have our own vma_impl.cpp)
FetchContent_GetProperties(VulkanMemoryAllocator)
if(NOT vulkanmemoryallocator_POPULATED)
    FetchContent_Populate(VulkanMemoryAllocator)
endif()

# ImGui needs manual setup (header-only with implementation files)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
    
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
    )
    
    target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    
    target_link_libraries(imgui PUBLIC
        Vulkan::Vulkan
        glfw
    )
    
    target_compile_definitions(imgui PUBLIC
        IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING=1
    )
endif()

# Include shader compilation module
include(cmake/CompileShaders.cmake)

# Add subdirectories
add_subdirectory(engine)
add_subdirectory(sandbox)
add_subdirectory(editor)
