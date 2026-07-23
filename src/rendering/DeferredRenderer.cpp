#include "core/DeferredRenderer.h"

#include "core/Camera.h"
#include "core/Light.h"
#include "core/Scene.h"

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/mat4x4.hpp>

#include <string>

DeferredRenderer::DeferredRenderer(int width, int height)
    : m_gbuffer(width, height),
      m_geometryShader("shaders/gbuffer.vert", "shaders/gbuffer.frag"),
      m_lightingShader("shaders/deferred_lighting.vert", "shaders/deferred_lighting.frag"),
      m_fullscreenVao(0),
      m_width(width),
      m_height(height) {
    // Attachment order here fixes the "location = N" numbers in gbuffer.frag:
    // 0 = albedo (RGBA8, 4 bytes/pixel), 1 = normal (RGB16F, 6 bytes/pixel).
    // No position attachment -- reconstructed from depth instead, see deferred_lighting.frag.
    m_gbuffer.addColorAttachment(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
    m_gbuffer.addColorAttachment(GL_RGB16F, GL_RGB, GL_FLOAT);
    m_gbuffer.addDepthAttachment();
    m_gbuffer.finalize();

    // Core profile requires a bound VAO to issue any draw call, even though the
    // fullscreen triangle vertex shader fetches no per-vertex attributes at all.
    glGenVertexArrays(1, &m_fullscreenVao);
}

DeferredRenderer::~DeferredRenderer() {
    if (m_fullscreenVao != 0) {
        glDeleteVertexArrays(1, &m_fullscreenVao);
    }
}

void DeferredRenderer::resize(int width, int height) {
    m_width = width;
    m_height = height;
    m_gbuffer.resize(width, height);
}

void DeferredRenderer::render(const Scene& scene, const Camera& camera) {
    const glm::mat4 viewProjection = camera.projectionMatrix() * camera.viewMatrix();

    // ---- Geometry pass: fill the G-buffer, one draw call per object ----------------
    m_geometryTimer.begin();
    m_gbuffer.bind();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_geometryShader.bind();
    for (const SceneObject& object : scene.objects()) {
        const glm::mat4 model = object.modelMatrix();
        const glm::mat4 mvp = viewProjection * model;
        const glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(model));

        m_geometryShader.setMat4("uMVP", mvp);
        m_geometryShader.setMat3("uNormalMatrix", normalMatrix);
        m_geometryShader.setVec3("uAlbedo", object.material.albedo);

        object.mesh->draw();
    }

    Framebuffer::unbind();
    m_geometryTimer.end();

    // ---- Lighting pass: one fullscreen triangle, shading each pixel exactly once ---
    m_lightingTimer.begin();
    glViewport(0, 0, m_width, m_height);
    glDisable(GL_DEPTH_TEST); // not testing the fullscreen triangle against anything

    m_lightingShader.bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_gbuffer.colorTexture(0));
    m_lightingShader.setInt("uGAlbedo", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_gbuffer.colorTexture(1));
    m_lightingShader.setInt("uGNormal", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_gbuffer.depthTexture());
    m_lightingShader.setInt("uGDepth", 2);

    m_lightingShader.setMat4("uInverseViewProjection", glm::inverse(viewProjection));

    const auto& lights = scene.lights();
    const int lightCount = static_cast<int>(lights.size() < kMaxLights ? lights.size() : kMaxLights);
    m_lightingShader.setInt("uLightCount", lightCount);
    for (int i = 0; i < lightCount; ++i) {
        const std::string prefix = "uLights[" + std::to_string(i) + "].";
        m_lightingShader.setVec3(prefix + "position", lights[i].position);
        m_lightingShader.setVec3(prefix + "color", lights[i].color);
        m_lightingShader.setFloat(prefix + "intensity", lights[i].intensity);
        m_lightingShader.setFloat(prefix + "radius", lights[i].radius);
    }

    glBindVertexArray(m_fullscreenVao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    m_lightingTimer.end();
}

double DeferredRenderer::gbufferMemoryMegabytes() const {
    constexpr double kBytesPerPixel = 4.0 /* albedo RGBA8 */ + 6.0 /* normal RGB16F */ + 4.0 /* depth */;
    const double totalBytes = kBytesPerPixel * static_cast<double>(m_width) * static_cast<double>(m_height);
    return totalBytes / (1024.0 * 1024.0);
}
