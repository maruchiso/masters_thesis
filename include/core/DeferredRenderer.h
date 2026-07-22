#pragma once

#include "core/Framebuffer.h"
#include "core/Shader.h"

#include <glad/glad.h>

class Camera;
class Scene;

// Splits rendering into two passes: a geometry pass that writes albedo and world-space
// normal into an offscreen G-buffer (depth comes along for free via the hardware depth
// test), and a lighting pass that reads the G-buffer back and shades once per pixel,
// regardless of how many objects overlapped that pixel during the geometry pass.
// Reuses the exact same lighting function as ForwardRenderer (shaders/common/lighting.glsl)
// so the two pipelines only differ in *how* shading work is organized, never in *what*
// it computes -- required for the visual-correctness comparison to mean anything.
class DeferredRenderer {
public:
    DeferredRenderer(int width, int height);
    ~DeferredRenderer();

    DeferredRenderer(const DeferredRenderer&) = delete;
    DeferredRenderer& operator=(const DeferredRenderer&) = delete;

    void render(const Scene& scene, const Camera& camera) const;

    // Must be called whenever the window resizes, so the G-buffer stays pixel-for-pixel
    // matched with the screen. See main.cpp's framebufferSizeCallback.
    void resize(int width, int height);

private:
    Framebuffer m_gbuffer;
    Shader m_geometryShader;
    Shader m_lightingShader;
    GLuint m_fullscreenVao;
    int m_width;
    int m_height;
};
