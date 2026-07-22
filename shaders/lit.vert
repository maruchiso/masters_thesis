#version 450 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

uniform mat4 uModel;
uniform mat4 uMVP;
uniform mat3 uNormalMatrix;

out vec3 vWorldPosition;
out vec3 vWorldNormal;
out vec2 vUV;

void main() {
    vWorldPosition = vec3(uModel * vec4(inPosition, 1.0));
    vWorldNormal = normalize(uNormalMatrix * inNormal);
    vUV = inUV;

    gl_Position = uMVP * vec4(inPosition, 1.0);
}
