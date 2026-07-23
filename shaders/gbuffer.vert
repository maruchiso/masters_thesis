#version 450 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

uniform mat4 uMVP;
uniform mat3 uNormalMatrix;

out vec3 vWorldNormal;

// No world position output here on purpose -- the lighting pass reconstructs it from
// the depth buffer instead of paying for a third G-buffer target to store it directly.
void main() {
    vWorldNormal = normalize(uNormalMatrix * inNormal);
    gl_Position = uMVP * vec4(inPosition, 1.0);
}
