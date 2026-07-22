#include "core/Framebuffer.h"

#include <stdexcept>
#include <string>
#include <utility>

Framebuffer::Framebuffer(int width, int height)
    : m_fbo(0), m_depthTexture(0), m_hasDepth(false), m_width(width), m_height(height) {
    glGenFramebuffers(1, &m_fbo);
}

Framebuffer::~Framebuffer() {
    release();
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : m_fbo(other.m_fbo),
      m_colorTextures(std::move(other.m_colorTextures)),
      m_colorInternalFormats(std::move(other.m_colorInternalFormats)),
      m_colorFormats(std::move(other.m_colorFormats)),
      m_colorTypes(std::move(other.m_colorTypes)),
      m_depthTexture(other.m_depthTexture),
      m_hasDepth(other.m_hasDepth),
      m_width(other.m_width),
      m_height(other.m_height) {
    other.m_fbo = 0;
    other.m_depthTexture = 0;
    other.m_hasDepth = false;
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    release();

    m_fbo = other.m_fbo;
    m_colorTextures = std::move(other.m_colorTextures);
    m_colorInternalFormats = std::move(other.m_colorInternalFormats);
    m_colorFormats = std::move(other.m_colorFormats);
    m_colorTypes = std::move(other.m_colorTypes);
    m_depthTexture = other.m_depthTexture;
    m_hasDepth = other.m_hasDepth;
    m_width = other.m_width;
    m_height = other.m_height;

    other.m_fbo = 0;
    other.m_depthTexture = 0;
    other.m_hasDepth = false;

    return *this;
}

void Framebuffer::addColorAttachment(GLenum internalFormat, GLenum format, GLenum type) {
    const size_t index = m_colorTextures.size();

    m_colorInternalFormats.push_back(internalFormat);
    m_colorFormats.push_back(format);
    m_colorTypes.push_back(type);

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(internalFormat), m_width, m_height, 0, format, type, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(index),
        GL_TEXTURE_2D,
        texture,
        0
    );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_colorTextures.push_back(texture);
}

void Framebuffer::addDepthAttachment() {
    if (m_hasDepth) {
        throw std::runtime_error("Framebuffer already has a depth attachment.");
    }

    glGenTextures(1, &m_depthTexture);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_width, m_height, 0,
        GL_DEPTH_COMPONENT, GL_FLOAT, nullptr
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_hasDepth = true;
}

void Framebuffer::finalize() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    if (!m_colorTextures.empty()) {
        std::vector<GLenum> drawBuffers(m_colorTextures.size());
        for (size_t i = 0; i < drawBuffers.size(); ++i) {
            drawBuffers[i] = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
        }
        glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
    } else {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Framebuffer is incomplete, status: " + std::to_string(status));
    }
}

void Framebuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
}

void Framebuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::resize(int width, int height) {
    if (width == m_width && height == m_height) {
        return;
    }

    m_width = width;
    m_height = height;

    for (size_t i = 0; i < m_colorTextures.size(); ++i) {
        glBindTexture(GL_TEXTURE_2D, m_colorTextures[i]);
        glTexImage2D(
            GL_TEXTURE_2D, 0, static_cast<GLint>(m_colorInternalFormats[i]),
            m_width, m_height, 0, m_colorFormats[i], m_colorTypes[i], nullptr
        );
    }

    if (m_hasDepth) {
        glBindTexture(GL_TEXTURE_2D, m_depthTexture);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_width, m_height, 0,
            GL_DEPTH_COMPONENT, GL_FLOAT, nullptr
        );
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint Framebuffer::colorTexture(size_t index) const {
    return m_colorTextures.at(index);
}

GLuint Framebuffer::depthTexture() const {
    return m_depthTexture;
}

int Framebuffer::width() const {
    return m_width;
}

int Framebuffer::height() const {
    return m_height;
}

void Framebuffer::release() {
    if (!m_colorTextures.empty()) {
        glDeleteTextures(static_cast<GLsizei>(m_colorTextures.size()), m_colorTextures.data());
        m_colorTextures.clear();
    }
    if (m_hasDepth) {
        glDeleteTextures(1, &m_depthTexture);
        m_depthTexture = 0;
        m_hasDepth = false;
    }
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
}
