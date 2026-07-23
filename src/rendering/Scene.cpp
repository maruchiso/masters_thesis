#include "core/Scene.h"

#include <glm/gtc/constants.hpp>

#include <algorithm>
#include <cmath>

Scene::Scene() = default;

void Scene::initialize(int lightCount, int objectCount) {
    m_meshes.clear();
    m_objects.clear();
    m_lights.clear();

    // Build all mesh geometry up front: SceneObject stores raw pointers into
    // this vector, so nothing may be appended to m_meshes afterwards.
    m_meshes.push_back(Mesh::createPlane(12.0f));
    m_meshes.push_back(Mesh::createCube(1.0f));
    m_meshes.push_back(Mesh::createSphere());

    const Mesh* planeMesh = &m_meshes[0];
    const Mesh* cubeMesh = &m_meshes[1];
    const Mesh* sphereMesh = &m_meshes[2];

    SceneObject ground;
    ground.mesh = planeMesh;
    ground.position = glm::vec3(0.0f, -1.0f, 0.0f);
    ground.material.albedo = glm::vec3(0.35f, 0.35f, 0.38f);
    m_objects.push_back(ground);

    SceneObject sphere;
    sphere.mesh = sphereMesh;
    sphere.position = glm::vec3(0.0f, 1.2f, -6.0f);
    sphere.scale = glm::vec3(1.3f);
    sphere.material.albedo = glm::vec3(0.25f, 0.55f, 0.75f);
    m_objects.push_back(sphere);

    // Grid of cubes: the geometry-density and overdraw benchmark knob. The grid extends
    // along Z (the camera's default view axis), so a higher objectCount both raises total
    // triangle count (density sweep) and stacks more overlapping layers from the default
    // viewpoint (overdraw sweep) -- one spawning mechanism covers both, per PROJECT_PLAN.
    const int clampedObjectCount = std::max(objectCount, 0);
    const int gridSize = static_cast<int>(std::ceil(std::cbrt(static_cast<double>(std::max(clampedObjectCount, 1)))));
    const float spacing = 1.6f;

    int spawned = 0;
    for (int x = 0; x < gridSize && spawned < clampedObjectCount; ++x) {
        for (int y = 0; y < gridSize && spawned < clampedObjectCount; ++y) {
            for (int z = 0; z < gridSize && spawned < clampedObjectCount; ++z) {
                SceneObject cube;
                cube.mesh = cubeMesh;
                cube.position = glm::vec3(
                    (static_cast<float>(x) - gridSize * 0.5f) * spacing,
                    (static_cast<float>(y) - gridSize * 0.5f) * spacing,
                    -4.0f - static_cast<float>(z) * spacing
                );
                cube.rotationEulerDegrees = glm::vec3(0.0f, static_cast<float>(spawned) * 11.0f, 0.0f);
                cube.material.albedo = glm::vec3(0.75f, 0.35f, 0.25f);
                m_objects.push_back(cube);
                ++spawned;
            }
        }
    }

    // Lights arranged procedurally in a ring instead of hand-placed, so lightCount can be
    // swept for benchmarking. Color hue is spread evenly around the ring purely so a large
    // lightCount is visually distinguishable rather than every light looking identical.
    const int clampedLightCount = std::clamp(lightCount, 0, kMaxLights);
    for (int i = 0; i < clampedLightCount; ++i) {
        const float denom = static_cast<float>(std::max(clampedLightCount, 1));
        const float angle = (static_cast<float>(i) / denom) * glm::two_pi<float>();
        const float ringRadius = 6.0f;

        PointLight light;
        light.position = glm::vec3(
            std::cos(angle) * ringRadius,
            3.0f + std::sin(angle * 2.0f),
            std::sin(angle) * ringRadius - 3.0f
        );
        light.color = glm::vec3(
            0.5f + 0.5f * std::sin(angle),
            0.5f + 0.5f * std::sin(angle + glm::two_pi<float>() / 3.0f),
            0.5f + 0.5f * std::sin(angle + 2.0f * glm::two_pi<float>() / 3.0f)
        );
        light.intensity = 2.0f;
        light.radius = 10.0f;
        m_lights.push_back(light);
    }
}

void Scene::update(float /*elapsedSeconds*/) {
}
