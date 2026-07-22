#pragma once

#include <glad/glad.h>

#include <vector>

// Generic offscreen render target wrapper. Not wired into the render loop yet --
// this is the shared building block the Deferred Shading G-buffer and the
// Visibility Buffer's visibility/resolve targets will both be built on top of.
//
// Usage:
//   Framebuffer gbuffer(width, height);
//   gbuffer.addColorAttachment(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);   // albedo
//   gbuffer.addColorAttachment(GL_RGB16F, GL_RGB, GL_FLOAT);          // normal
//   gbuffer.addDepthAttachment();
//   gbuffer.finalize();
//   gbuffer.bind();
//   ... render geometry pass ...
//   Framebuffer::unbind();
class Framebuffer {
public:
    Framebuffer(int width, int height);
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    Framebuffer(Framebuffer&& other) noexcept;
    Framebuffer& operator=(Framebuffer&& other) noexcept;

    // Adds a texture color attachment at the next available color slot.
    // Call before finalize().
    void addColorAttachment(GLenum internalFormat, GLenum format, GLenum type);

    // Adds a sampleable depth texture attachment. Call before finalize().
    void addDepthAttachment();

    // Sets glDrawBuffers for all added color attachments and checks completeness.
    // Must be called once after all attachments are added, before first use.
    void finalize();

    void bind() const;
    static void unbind();

    // Recreates all attachments at the new size. Call finalize() again afterwards
    // is not required -- draw buffer setup is preserved.
    void resize(int width, int height);

    GLuint colorTexture(size_t index) const;
    GLuint depthTexture() const;

    int width() const;
    int height() const;

private:
    GLuint m_fbo;
    std::vector<GLuint> m_colorTextures;
    std::vector<GLenum> m_colorInternalFormats;
    std::vector<GLenum> m_colorFormats;
    std::vector<GLenum> m_colorTypes;
    GLuint m_depthTexture;
    bool m_hasDepth;
    int m_width;
    int m_height;

    void release();
};
