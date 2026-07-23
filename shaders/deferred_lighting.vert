#version 450 core

out vec2 vUV;

// Draws one triangle that covers the whole screen, using no vertex buffer at all --
// positions are generated purely from gl_VertexID (0, 1, 2). This is the standard
// "fullscreen triangle" trick: a triangle with corners at (0,0), (2,0), (0,2) in UV
// space covers the entire [0,1] visible area (the part past 1.0 just gets clipped),
// using one triangle instead of two (a quad), which avoids a diagonal seam.
// DeferredRenderer still has to bind *some* VAO to issue the draw call (core profile
// requires it), even though this VAO has zero enabled vertex attributes.
void main() {
    vec2 uv = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    vUV = uv;
    gl_Position = vec4(uv * 2.0 - 1.0, 0.0, 1.0);
}
