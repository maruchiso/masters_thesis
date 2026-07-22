#pragma once

#include "core/Light.h"
#include "core/Mesh.h"
#include "core/SceneObject.h"

#include <glm/mat4x4.hpp>

#include <vector>

class Shader;

// Generic scene container, replacing the old hardcoded SimpleScene.
// Owns the mesh geometry and a list of instances (SceneObject) placed with it,
// plus a small set of point lights. Both the Deferred Shading and Visibility
// Buffer pipelines are meant to iterate the same `objects()`/`lights()` data --
// only the geometry/shading passes that consume it differ.
class Scene {
public:
    Scene();

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    void initialize();
    void update(float elapsedSeconds);

    // Simple forward-rendering path used for validating the scene while the
    // deferred/visibility pipelines are not built yet.
    void render(const Shader& shader, const glm::mat4& viewProjection) const;

    const std::vector<SceneObject>& objects() const { return m_objects; }
    const std::vector<PointLight>& lights() const { return m_lights; }

private:
    std::vector<Mesh> m_meshes;
    std::vector<SceneObject> m_objects;
    std::vector<PointLight> m_lights;
};
