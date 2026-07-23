#version 450 core

#include "common/lighting.glsl"

in vec2 vUV;
out vec4 outColor;

uniform sampler2D uGAlbedo;
uniform sampler2D uGNormal;
uniform sampler2D uGDepth;

uniform mat4 uInverseViewProjection;

uniform int uLightCount;
uniform PointLight uLights[MAX_LIGHTS];

// Undoes the camera's Model-View-Projection transform for a single pixel: takes this
// pixel's screen UV plus its stored depth, reconstructs the clip-space position, and
// multiplies by the inverse of View*Projection to get back to world space. Setting the
// clip-space w component to 1.0 here is not a simplification -- it is mathematically
// exact for this purpose (the perspective divide below cancels out whatever the "true"
// w would have been), so this recovers the same world position the geometry pass's
// vertex shader started from, without the G-buffer ever having stored it.
vec3 reconstructWorldPosition(vec2 uv, float depth) {
    vec3 ndc = vec3(uv, depth) * 2.0 - 1.0; // [0,1] -> [-1,1] on all three axes
    vec4 clipPosition = vec4(ndc, 1.0);
    vec4 worldPosition = uInverseViewProjection * clipPosition;
    return worldPosition.xyz / worldPosition.w;
}

void main() {
    float depth = texture(uGDepth, vUV).r;

    // Depth was cleared to 1.0 (the far plane) wherever the geometry pass drew nothing --
    // skip shading the background instead of lighting garbage G-buffer data there.
    if (depth >= 1.0) {
        discard;
    }

    vec3 albedo = texture(uGAlbedo, vUV).rgb;
    vec3 normal = normalize(texture(uGNormal, vUV).rgb);
    vec3 worldPosition = reconstructWorldPosition(vUV, depth);

    vec3 result = albedo * 0.05; // same flat ambient term as the forward pass
    for (int i = 0; i < uLightCount; ++i) {
        result += evaluatePointLight(uLights[i], worldPosition, normal, albedo);
    }

    outColor = vec4(result, 1.0);
}
