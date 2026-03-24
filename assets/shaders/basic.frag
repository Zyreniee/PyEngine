#version 450

// ═══════════════════════════════════════════════════════════════
// PyEngine PBR Fragment Shader — Cook-Torrance BRDF
// ═══════════════════════════════════════════════════════════════

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

#define MAX_LIGHTS 8

struct LightData {
    vec4 Position;     // xyz = position/direction, w = type (0=dir, 1=point, 2=spot)
    vec4 Direction;    // xyz = direction, w = range
    vec4 Color;        // xyz = color, w = intensity
    vec4 SpotParams;   // x = innerCone, y = outerCone
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 cameraPos;
    vec4 ambientColor;     // xyz = color, w = intensity
    LightData lights[MAX_LIGHTS];
    int lightCount;
} ubo;

layout(push_constant) uniform PushConstants {
    mat4 model;
    vec4 albedoColor;
    float metallic;
    float roughness;
    float ao;
    float _pad;
} pc;

// ─── PBR Math ───────────────────────────────────────────────

const float PI = 3.14159265359;

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / max(denom, 0.0000001);
}

// Geometry Function (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// Fresnel (Schlick Approximation)
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ─── Main ───────────────────────────────────────────────────

void main() {
    vec3 albedo = pc.albedoColor.rgb;
    float metallic = pc.metallic;
    float roughness = max(pc.roughness, 0.04); // clamp minimum roughness
    float ao = pc.ao;

    vec3 N = normalize(fragNormal);
    vec3 V = normalize(ubo.cameraPos.xyz - fragWorldPos);

    // Base reflectivity: dielectrics = 0.04, metals = albedo
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // ── Accumulate light contribution ──
    vec3 Lo = vec3(0.0);

    for (int i = 0; i < ubo.lightCount && i < MAX_LIGHTS; i++) {
        vec3 L;
        float attenuation = 1.0;
        float lightType = ubo.lights[i].Position.w;
        vec3 lightColor = ubo.lights[i].Color.rgb * ubo.lights[i].Color.w; // color * intensity

        if (lightType < 0.5) {
            // ── Directional Light ──
            L = normalize(-ubo.lights[i].Direction.xyz);
        } else if (lightType < 1.5) {
            // ── Point Light ──
            vec3 lightPos = ubo.lights[i].Position.xyz;
            L = lightPos - fragWorldPos;
            float distance = length(L);
            L = normalize(L);
            float range = ubo.lights[i].Direction.w;
            attenuation = clamp(1.0 - (distance * distance) / (range * range), 0.0, 1.0);
            attenuation *= attenuation;
        } else {
            // ── Spot Light ──
            vec3 lightPos = ubo.lights[i].Position.xyz;
            L = lightPos - fragWorldPos;
            float distance = length(L);
            L = normalize(L);
            float range = ubo.lights[i].Direction.w;
            attenuation = clamp(1.0 - (distance * distance) / (range * range), 0.0, 1.0);
            attenuation *= attenuation;

            float innerCone = cos(radians(ubo.lights[i].SpotParams.x));
            float outerCone = cos(radians(ubo.lights[i].SpotParams.y));
            float theta = dot(L, normalize(-ubo.lights[i].Direction.xyz));
            float epsilon = innerCone - outerCone;
            float spotFactor = clamp((theta - outerCone) / max(epsilon, 0.001), 0.0, 1.0);
            attenuation *= spotFactor;
        }

        vec3 H = normalize(V + L);
        vec3 radiance = lightColor * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        // Energy conservation
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic; // Metals have no diffuse

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // ── Ambient ──
    vec3 ambient = ubo.ambientColor.rgb * ubo.ambientColor.w * albedo * ao;

    vec3 color = ambient + Lo;

    // ── HDR tone mapping (Reinhard) + gamma correction ──
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, pc.albedoColor.a);
}
