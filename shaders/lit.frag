#version 450 core

#define MAX_LIGHTS 8

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float radius;
};

in vec3 vWorldPosition;
in vec3 vWorldNormal;
in vec2 vUV;

out vec4 outColor;

uniform vec3 uAlbedo;
uniform int uLightCount;
uniform PointLight uLights[MAX_LIGHTS];

// Same Lambertian + simple attenuation model that the Deferred Shading and
// Visibility Buffer lighting passes should reuse, so the visual-correctness
// comparison between pipelines is measuring pipeline behavior, not different math.
vec3 evaluatePointLight(PointLight light, vec3 worldPosition, vec3 normal, vec3 albedo) {
    vec3 toLight = light.position - worldPosition;
    float distance = length(toLight);
    vec3 lightDir = toLight / max(distance, 0.0001);

    float attenuation = clamp(1.0 - (distance / light.radius), 0.0, 1.0);
    attenuation *= attenuation;

    float ndotl = max(dot(normal, lightDir), 0.0);

    return albedo * light.color * light.intensity * ndotl * attenuation;
}

void main() {
    vec3 normal = normalize(vWorldNormal);

    vec3 result = uAlbedo * 0.05; // flat ambient term, kept minimal on purpose
    for (int i = 0; i < uLightCount; ++i) {
        result += evaluatePointLight(uLights[i], vWorldPosition, normal, uAlbedo);
    }

    outColor = vec4(result, 1.0);
}
