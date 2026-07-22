#pragma once

#include "core/Material.h"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

class Mesh;

// A single instance of a Mesh placed in the scene with its own transform and material.
// Non-owning pointer to the Mesh: Scene owns the mesh geometry, objects just reference it,
// so multiple objects can share one Mesh (e.g. several cubes reusing the same cube geometry).
struct SceneObject {
    const Mesh* mesh = nullptr;
    Material material;

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotationEulerDegrees = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);

    glm::mat4 modelMatrix() const {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
        model = glm::rotate(model, glm::radians(rotationEulerDegrees.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotationEulerDegrees.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotationEulerDegrees.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);
        return model;
    }
};
