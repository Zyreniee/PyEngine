# CompileShaders.cmake - Compile GLSL shaders to SPIR-V

find_program(GLSLC_EXECUTABLE glslc REQUIRED)

# Function to compile a single shader
# Usage: compile_shader(TARGET_NAME SHADER_SOURCE OUTPUT_DIR)
function(compile_shader TARGET_NAME SHADER_SOURCE OUTPUT_DIR)
    get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME)
    set(SPIRV_OUTPUT "${OUTPUT_DIR}/${SHADER_NAME}.spv")
    
    # Determine shader stage from extension
    get_filename_component(SHADER_EXT ${SHADER_SOURCE} LAST_EXT)
    # Strip the leading dot from the extension (e.g., .vert -> vert)
    string(SUBSTRING ${SHADER_EXT} 1 -1 SHADER_STAGE)
    
    # Add custom command to compile shader
    add_custom_command(
        OUTPUT ${SPIRV_OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
        COMMAND ${GLSLC_EXECUTABLE}
            -fshader-stage=${SHADER_STAGE}
            -o ${SPIRV_OUTPUT}
            ${SHADER_SOURCE}
        DEPENDS ${SHADER_SOURCE}
        COMMENT "Compiling shader: ${SHADER_NAME}"
        VERBATIM
    )
    
    # Add to target dependencies
    target_sources(${TARGET_NAME} PRIVATE ${SPIRV_OUTPUT})
endfunction()

# Function to compile multiple shaders
# Usage: compile_shaders(TARGET_NAME OUTPUT_DIR SHADER1 SHADER2 ...)
function(compile_shaders TARGET_NAME OUTPUT_DIR)
    foreach(SHADER ${ARGN})
        compile_shader(${TARGET_NAME} ${SHADER} ${OUTPUT_DIR})
    endforeach()
endfunction()
