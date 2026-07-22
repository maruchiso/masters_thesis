#pragma once

#include "core/Shader.h"

class Camera;
class Scene;

// The baseline pipeline: geometry and lighting evaluated together in a single
// pass, once per object -- the classic "immediate mode" way of rendering,
// and the technique Deferred Shading and Visibility Buffer both exist to improve on.
//
// Owns its own Shader, deliberately: DeferredRenderer and VisibilityBufferRenderer
// each own their own shaders/passes too, all consuming the same Scene and Camera,
// so the three pipelines can be benchmarked under identical conditions.
class ForwardRenderer {
public:
    ForwardRenderer();

    ForwardRenderer(const ForwardRenderer&) = delete;
    ForwardRenderer& operator=(const ForwardRenderer&) = delete;

    void render(const Scene& scene, const Camera& camera) const;

private:
    Shader m_shader;
};
