#version 450 core

#include "common/lighting.glsl"

in vec3 vWorldPosition;
in vec3 vWorldNormal;
in vec2 vUV;

out vec4 outColor;

uniform vec3 uAlbedo;
uniform int uLightCount;
uniform PointLight uLights[MAX_LIGHTS];

void main() {
    vec3 normal = normalize(vWorldNormal);

    vec3 result = uAlbedo * 0.05; // flat ambient term, kept minimal on purpose
    for (int i = 0; i < uLightCount; ++i) {
        result += evaluatePointLight(uLights[i], vWorldPosition, normal, uAlbedo);
    }

    outColor = vec4(result, 1.0);
}
