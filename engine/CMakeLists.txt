cmake_minimum_required(VERSION 3.24)

# Collect all engine source files
file(GLOB_RECURSE ENGINE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Create engine library
add_library(PyEngineLib STATIC ${ENGINE_SOURCES})

# Public include directory
target_include_directories(PyEngineLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Third-party includes
target_include_directories(PyEngineLib PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party
    ${CMAKE_BINARY_DIR}/_deps/entt-src/single_include
    ${CMAKE_BINARY_DIR}/_deps/vulkanmemoryallocator-src/include
)

# Link libraries
target_link_libraries(PyEngineLib PUBLIC
    Vulkan::Vulkan
    glfw
    glm::glm
    spdlog::spdlog
    fmt::fmt
    EnTT::EnTT
    imgui
    VulkanMemoryAllocator
)

# Add preprocessor definitions
target_compile_definitions(PyEngineLib PUBLIC
    GLM_FORCE_RADIANS
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_ENABLE_EXPERIMENTAL
)

# Debug-specific definitions
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(PyEngineLib PUBLIC
        PYENGINE_DEBUG
        PYENGINE_ENABLE_ASSERTS
    )
endif()

# STB and VMA implementation files
target_sources(PyEngineLib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty/stb_image_impl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty/vma_impl.cpp
)
