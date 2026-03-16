cmake_minimum_required(VERSION 3.24)

# Collect editor sources
file(GLOB_RECURSE EDITOR_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Create editor executable
add_executable(PyEngineEditor ${EDITOR_SOURCES})

# Include directories
target_include_directories(PyEngineEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link engine library (brings all transitive dependencies)
target_link_libraries(PyEngineEditor PRIVATE
    PyEngineLib
)

# Shader compilation
set(SHADER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/assets/shaders)
set(SHADER_OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/shaders)

compile_shaders(PyEngineEditor ${SHADER_OUTPUT_DIR}
    ${SHADER_SOURCE_DIR}/basic.vert
    ${SHADER_SOURCE_DIR}/basic.frag
    ${SHADER_SOURCE_DIR}/grid.vert
    ${SHADER_SOURCE_DIR}/grid.frag
    ${SHADER_SOURCE_DIR}/navmesh_debug.vert
    ${SHADER_SOURCE_DIR}/navmesh_debug.frag
)

# Copy assets to build directory
add_custom_command(TARGET PyEngineEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/bin/assets
    COMMENT "Copying assets to build directory"
)
