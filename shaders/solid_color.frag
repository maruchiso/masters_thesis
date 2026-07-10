#version 450 core

out vec4 outColor;

void main() {
    const float depthTint = clamp(gl_FragCoord.z, 0.0, 1.0);
    const vec3 nearColor = vec3(0.93, 0.64, 0.25);
    const vec3 farColor = vec3(0.25, 0.62, 0.94);
    const vec3 color = mix(nearColor, farColor, depthTint);
    outColor = vec4(color, 1.0);
}
