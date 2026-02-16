#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(push_constant) uniform PushConsts {
    mat4 view;
    mat4 proj;
} u_Camera;

void main() {
    gl_Position = u_Camera.proj * u_Camera.view * vec4(inPosition, 1.0);
}
