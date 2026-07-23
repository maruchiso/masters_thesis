#include "core/ForwardRenderer.h"

#include "core/Camera.h"
#include "core/Light.h"
#include "core/Scene.h"

#include <glm/gtc/matrix_inverse.hpp>

#include <string>

ForwardRenderer::ForwardRenderer()
    : m_shader("shaders/lit.vert", "shaders/lit.frag") {
}

void ForwardRenderer::render(const Scene& scene, const Camera& camera) {
    m_timer.begin();

    // Explicit, not assumed: DeferredRenderer's lighting pass disables depth testing
    // for its fullscreen triangle, so each renderer sets the GL state it needs itself
    // rather than relying on whatever a previously-run renderer left behind.
    glEnable(GL_DEPTH_TEST);

    m_shader.bind();

    const glm::mat4 viewProjection = camera.projectionMatrix() * camera.viewMatrix();

    const auto& lights = scene.lights();
    const int lightCount = static_cast<int>(lights.size() < kMaxLights ? lights.size() : kMaxLights);
    m_shader.setInt("uLightCount", lightCount);
    for (int i = 0; i < lightCount; ++i) {
        const std::string prefix = "uLights[" + std::to_string(i) + "].";
        m_shader.setVec3(prefix + "position", lights[i].position);
        m_shader.setVec3(prefix + "color", lights[i].color);
        m_shader.setFloat(prefix + "intensity", lights[i].intensity);
        m_shader.setFloat(prefix + "radius", lights[i].radius);
    }

    for (const SceneObject& object : scene.objects()) {
        const glm::mat4 model = object.modelMatrix();
        const glm::mat4 mvp = viewProjection * model;
        const glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(model));

        m_shader.setMat4("uModel", model);
        m_shader.setMat4("uMVP", mvp);
        m_shader.setMat3("uNormalMatrix", normalMatrix);
        m_shader.setVec3("uAlbedo", object.material.albedo);

        object.mesh->draw();
    }

    m_timer.end();
}
