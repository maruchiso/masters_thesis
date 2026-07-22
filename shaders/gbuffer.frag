#version 450 core

in vec3 vWorldNormal;

// Location numbers here must match the order Framebuffer::addColorAttachment() was
// called in DeferredRenderer's constructor: 0 = albedo, 1 = normal. Depth is written
// automatically by the hardware depth test, no explicit output needed for it here.
layout (location = 0) out vec4 outAlbedo;
layout (location = 1) out vec3 outNormal;

uniform vec3 uAlbedo;

void main() {
    outAlbedo = vec4(uAlbedo, 1.0);
    outNormal = normalize(vWorldNormal);
}
