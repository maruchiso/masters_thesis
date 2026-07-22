#pragma once

#include <glm/vec3.hpp>

// Deliberately minimal per PROJECT_PLAN.md: solid-color material only.
// No textures/PBR params yet -- extend here if a textured comparison scene is needed later.
struct Material {
    glm::vec3 albedo = glm::vec3(0.8f, 0.8f, 0.8f);
};
