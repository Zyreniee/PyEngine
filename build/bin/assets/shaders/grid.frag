#version 450

layout(location = 0) in float inAlpha;
layout(location = 1) in vec3 inWorldPos;

layout(location = 0) out vec4 outColor;

void main() {
    // Grid parameters
    float majorLines = 10.0;
    float minorLines = 1.0;
    float lineWidth = 0.02; // Thickness
    
    vec2 coord = inWorldPos.xz;
    vec2 derivative = fwidth(coord);
    
    // Grid function
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float alpha = 1.0 - min(line, 1.0);
    
    // Major lines
    vec2 gridMajor = abs(fract(coord / majorLines - 0.5) - 0.5) / derivative / majorLines;
    float lineMajor = min(gridMajor.x, gridMajor.y);
    float alphaMajor = 1.0 - min(lineMajor, 1.0);
    
    // Color
    vec3 color = vec3(0.2); // Dark gray
    
    // X axis (Red)
    if (abs(inWorldPos.z) < lineWidth) color = vec3(1.0, 0.0, 0.0);
    // Z axis (Blue)
    if (abs(inWorldPos.x) < lineWidth) color = vec3(0.0, 0.0, 1.0);
    
    // Combine
    float finalAlpha = max(alpha * 0.3, alphaMajor * 0.6);
    
    // Fade distance
    float dist = length(inWorldPos);
    float fade = 1.0 - smoothstep(100.0, 200.0, dist);
    finalAlpha *= fade;
    
    if (finalAlpha < 0.01) discard;
    
    outColor = vec4(color, finalAlpha);
}
