#include "core/Scene.h"

#include <iterator>

Scene::Scene() = default;

void Scene::initialize() {
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

    const glm::vec3 cubePositions[] = {
        {0.0f, 0.0f, 0.0f},
        {2.0f, 0.5f, -2.0f},
        {-2.0f, -0.3f, -1.5f},
        {1.0f, -0.5f, -4.0f},
        {-1.2f, 0.8f, -3.2f},
    };

    for (size_t i = 0; i < std::size(cubePositions); ++i) {
        SceneObject cube;
        cube.mesh = cubeMesh;
        cube.position = cubePositions[i];
        cube.rotationEulerDegrees = glm::vec3(0.0f, static_cast<float>(i) * 18.0f, 0.0f);
        cube.material.albedo = glm::vec3(0.75f, 0.35f, 0.25f);
        m_objects.push_back(cube);
    }

    SceneObject sphere;
    sphere.mesh = sphereMesh;
    sphere.position = glm::vec3(0.0f, 1.2f, -6.0f);
    sphere.scale = glm::vec3(1.3f);
    sphere.material.albedo = glm::vec3(0.25f, 0.55f, 0.75f);
    m_objects.push_back(sphere);

    m_lights.push_back(PointLight{glm::vec3(3.0f, 3.0f, 2.0f), glm::vec3(1.0f, 0.95f, 0.85f), 3.0f, 15.0f});
    m_lights.push_back(PointLight{glm::vec3(-3.0f, 2.0f, -3.0f), glm::vec3(0.4f, 0.55f, 1.0f), 2.5f, 15.0f});
}

void Scene::update(float /*elapsedSeconds*/) {
}
