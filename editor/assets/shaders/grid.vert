#version 450

layout(location = 0) out float near; //0.1
layout(location = 1) out float far; //100
layout(location = 2) out vec3 nearPoint;
layout(location = 3) out vec3 farPoint;
layout(location = 4) out mat4 view;
layout(location = 8) out mat4 proj;

layout(push_constant) uniform PushConsts {
    mat4 view;
    mat4 proj;
} u_Camera;

vec3 UnprojectPoint(float x, float y, float z, mat4 view, mat4 projection) {
    mat4 viewInv = inverse(view);
    mat4 projInv = inverse(projection);
    vec4 unprojectedPoint =  viewInv * projInv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main() {
    vec3 p = vec3(
        (gl_VertexIndex << 1) & 2,
        (gl_VertexIndex & 2),
        0.0f
    );
    
    near = 0.1;
    far = 1000.0;
    view = u_Camera.view;
    proj = u_Camera.proj;
    
    nearPoint = UnprojectPoint(p.x, p.y, 0.0, view, proj);
    farPoint = UnprojectPoint(p.x, p.y, 1.0, view, proj);
    
    gl_Position = vec4(p, 1.0f);
}
