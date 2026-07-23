#pragma once

#include "core/Framebuffer.h"
#include "core/GpuTimer.h"
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

    // No longer const: render() now records GPU timing state via m_geometryTimer/m_lightingTimer.
    void render(const Scene& scene, const Camera& camera);

    // Must be called whenever the window resizes, so the G-buffer stays pixel-for-pixel
    // matched with the screen. See main.cpp's framebufferSizeCallback.
    void resize(int width, int height);

    double lastGeometryMilliseconds() const { return m_geometryTimer.lastElapsedMilliseconds(); }
    double lastLightingMilliseconds() const { return m_lightingTimer.lastElapsedMilliseconds(); }

    // Analytical G-buffer footprint at the current resolution -- not a live driver VRAM
    // query (those are vendor-specific extensions, not portable across GPUs). Computed from
    // the known attachment formats: 4 B/px albedo (RGBA8) + 6 B/px normal (RGB16F) + 4 B/px depth.
    double gbufferMemoryMegabytes() const;

private:
    Framebuffer m_gbuffer;
    Shader m_geometryShader;
    Shader m_lightingShader;
    GpuTimer m_geometryTimer;
    GpuTimer m_lightingTimer;
    GLuint m_fullscreenVao;
    int m_width;
    int m_height;
};
