// Shared by every pipeline's shading pass (Forward, Deferred, later Visibility Buffer),
// spliced in via Shader's #include preprocessing (see Shader::resolveIncludes).
// Keeping this in exactly one place is what makes "identical lighting math across
// pipelines" true rather than just claimed -- there is no second copy to drift out of sync.

// Must match kMaxLights in include/core/Light.h.
#define MAX_LIGHTS 64

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float radius;
};

vec3 evaluatePointLight(PointLight light, vec3 worldPosition, vec3 normal, vec3 albedo) {
    vec3 toLight = light.position - worldPosition;
    float distance = length(toLight);
    vec3 lightDir = toLight / max(distance, 0.0001);

    float attenuation = clamp(1.0 - (distance / light.radius), 0.0, 1.0);
    attenuation *= attenuation;

    float ndotl = max(dot(normal, lightDir), 0.0);

    return albedo * light.color * light.intensity * ndotl * attenuation;
}
