#pragma once

#include <glm/vec3.hpp>

// Point light only for now. A directional/sun light can be added the same way
// once the lighting pass exists, without changing this struct's layout much.
struct PointLight {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;
    float radius = 10.0f; // effective attenuation range, used later for scalability tests
};
