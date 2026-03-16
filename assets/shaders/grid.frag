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
    
    // Grid function for thin lines
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float alpha = 1.0 - min(line, 1.0);
    
    // Major lines
    vec2 gridMajor = abs(fract(coord / majorLines - 0.5) - 0.5) / derivative / majorLines;
    float lineMajor = min(gridMajor.x, gridMajor.y);
    float alphaMajor = 1.0 - min(lineMajor, 1.0);
    
    // Color (Subtle white/gray for lines, matching the image)
    vec3 color = vec3(0.6); 
    
    // Axes (Vibrant, bright like in the screenshot)
    bool isZAxis = abs(inWorldPos.x) < lineWidth * 1.5;
    bool isXAxis = abs(inWorldPos.z) < lineWidth * 1.5;
    
    if (isZAxis) { color = vec3(0.1, 0.2, 1.0); alphaMajor = max(alphaMajor, 1.0); }
    if (isXAxis) { color = vec3(1.0, 0.1, 0.1); alphaMajor = max(alphaMajor, 1.0); }
    
    // Combine
    float finalAlpha = max(alpha * 0.1, alphaMajor * 0.4); 
    if (isZAxis || isXAxis) {
        finalAlpha = max(finalAlpha, 0.8);
    }
    
    // Fade distance
    float dist = length(inWorldPos);
    float fade = 1.0 - smoothstep(15.0, 60.0, dist);
    finalAlpha *= fade;
    
    if (finalAlpha < 0.01) discard;
    
    outColor = vec4(color, finalAlpha);
}
