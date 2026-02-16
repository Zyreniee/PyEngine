#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out float outAlpha;
layout(location = 1) out vec3 outWorldPos;

layout(push_constant) uniform PushConstants {
    mat4 view;
    mat4 proj;
} u_Camera;

void main() {
    // Generate a huge quad on XZ plane centered at camera?
    // Or just a fixed huge quad.
    // For now, let's just draw a large fixed quad.
    
    // Grid size 
    float gridSize = 1000.0;
    
    vec3 pos = inPosition * gridSize;
    outWorldPos = pos;
    outAlpha = 1.0;
    
    // Fade out based on distance? handled in frag
    
    gl_Position = u_Camera.proj * u_Camera.view * vec4(pos, 1.0);
}
