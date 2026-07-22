#pragma once

#include "core/Light.h"
#include "core/Mesh.h"
#include "core/SceneObject.h"

#include <vector>

// Generic scene container, replacing the old hardcoded SimpleScene.
// Owns the mesh geometry and a list of instances (SceneObject) placed with it,
// plus a small set of point lights. Pure data -- it does not know how to draw
// itself. ForwardRenderer, and later DeferredRenderer/VisibilityBufferRenderer,
// each consume the same objects()/lights() data and decide how to render it,
// so all pipelines run under identical scene conditions.
class Scene {
public:
    Scene();

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    void initialize();
    void update(float elapsedSeconds);

    const std::vector<SceneObject>& objects() const { return m_objects; }
    const std::vector<PointLight>& lights() const { return m_lights; }

private:
    std::vector<Mesh> m_meshes;
    std::vector<SceneObject> m_objects;
    std::vector<PointLight> m_lights;
};
