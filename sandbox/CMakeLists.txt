cmake_minimum_required(VERSION 3.24)

# Sandbox executable
add_executable(PyEngineSandbox
    src/main.cpp
)

# Link engine library
target_link_libraries(PyEngineSandbox PRIVATE
    PyEngineLib
)

# Shader directory
set(SHADER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/assets/shaders)
set(SHADER_OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/shaders)

# Compile shaders
compile_shaders(PyEngineSandbox ${SHADER_OUTPUT_DIR}
    ${SHADER_SOURCE_DIR}/basic.vert
    ${SHADER_SOURCE_DIR}/basic.frag
)

# Copy assets to build directory (if needed)
add_custom_command(TARGET PyEngineSandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/bin/assets
    COMMENT "Copying assets to build directory"
)
