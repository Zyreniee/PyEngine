#version 450

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    // Simple directional lighting
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 normal = normalize(fragNormal);
    
    float ambient = 0.3;
    float diffuse = max(dot(normal, lightDir), 0.0);
    
    vec3 baseColor = vec3(0.7, 0.5, 0.3);
    vec3 finalColor = baseColor * (ambient + diffuse);
    
    outColor = vec4(finalColor, 1.0);
}
