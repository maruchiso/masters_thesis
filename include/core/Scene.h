#pragma once

#include "core/Light.h"
#include "core/Mesh.h"
#include "core/SceneObject.h"

#include <vector>

// Generic scene container, replacing the old hardcoded SimpleScene.
// Owns the mesh geometry and a list of instances (SceneObject) placed with it,
// plus a small set of point lights. Pure data -- it does not know how to draw
// itself. ForwardRenderer, DeferredRenderer, and later VisibilityBufferRenderer
// each consume the same objects()/lights() data and decide how to render it,
// so all pipelines run under identical scene conditions.
class Scene {
public:
    Scene();

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    // objectCount spawns that many cubes in a 3D grid (beyond the fixed ground plane and
    // one decorative sphere) -- the grid extends along the camera's view axis, so it
    // doubles as both the geometry-density and the overdraw benchmark knob. lightCount is
    // clamped to kMaxLights and arranged procedurally in a ring. Safe to call again at
    // runtime to rebuild the scene with different counts (see main.cpp's arrow-key controls).
    void initialize(int lightCount = 2, int objectCount = 5);
    void update(float elapsedSeconds);

    const std::vector<SceneObject>& objects() const { return m_objects; }
    const std::vector<PointLight>& lights() const { return m_lights; }

private:
    std::vector<Mesh> m_meshes;
    std::vector<SceneObject> m_objects;
    std::vector<PointLight> m_lights;
};
