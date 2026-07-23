#pragma once

#include <glm/vec3.hpp>

// Must match MAX_LIGHTS in shaders/common/lighting.glsl. Deliberately a fixed cap on a
// plain uniform array rather than an unbounded SSBO-backed light list: 64 is generous
// enough to demonstrate Forward Rendering's O(objects x lights) blowup in the benchmark
// sweeps, and staying with per-field uniform setters avoids std140/std430 layout-matching
// complexity between C++ and GLSL. Revisit if a benchmark ever genuinely needs more.
constexpr int kMaxLights = 64;

// Point light only for now. A directional/sun light can be added the same way
// once the lighting pass exists, without changing this struct's layout much.
struct PointLight {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;
    float radius = 10.0f; // effective attenuation range, used later for scalability tests
};
