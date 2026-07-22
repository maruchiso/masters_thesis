#include "core/Mesh.h"

#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>

#include <cmath>
#include <cstddef>
#include <utility>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    : m_vao(0), m_vbo(0), m_ebo(0), m_indexCount(0) {
    upload(vertices, indices);
}

Mesh::~Mesh() {
    release();
}

Mesh::Mesh(Mesh&& other) noexcept
    : m_vao(other.m_vao), m_vbo(other.m_vbo), m_ebo(other.m_ebo), m_indexCount(other.m_indexCount) {
    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_ebo = 0;
    other.m_indexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    release();

    m_vao = other.m_vao;
    m_vbo = other.m_vbo;
    m_ebo = other.m_ebo;
    m_indexCount = other.m_indexCount;

    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_ebo = 0;
    other.m_indexCount = 0;

    return *this;
}

void Mesh::draw() const {
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

GLsizei Mesh::indexCount() const {
    return m_indexCount;
}

void Mesh::upload(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    m_indexCount = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
        vertices.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
        indices.data(),
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<const void*>(offsetof(Vertex, position))
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<const void*>(offsetof(Vertex, normal))
    );

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<const void*>(offsetof(Vertex, uv))
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Mesh::release() {
    if (m_ebo != 0) {
        glDeleteBuffers(1, &m_ebo);
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
    }
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
    }

    m_vao = 0;
    m_vbo = 0;
    m_ebo = 0;
    m_indexCount = 0;
}

Mesh Mesh::createCube(float size) {
    const float h = size * 0.5f;

    // 24 vertices (4 per face) so each face gets its own flat normal and UVs.
    const std::vector<Vertex> vertices = {
        // +X face
        {{ h, -h, -h}, {1, 0, 0}, {0, 0}}, {{ h, -h,  h}, {1, 0, 0}, {1, 0}},
        {{ h,  h,  h}, {1, 0, 0}, {1, 1}}, {{ h,  h, -h}, {1, 0, 0}, {0, 1}},
        // -X face
        {{-h, -h,  h}, {-1, 0, 0}, {0, 0}}, {{-h, -h, -h}, {-1, 0, 0}, {1, 0}},
        {{-h,  h, -h}, {-1, 0, 0}, {1, 1}}, {{-h,  h,  h}, {-1, 0, 0}, {0, 1}},
        // +Y face
        {{-h,  h, -h}, {0, 1, 0}, {0, 0}}, {{ h,  h, -h}, {0, 1, 0}, {1, 0}},
        {{ h,  h,  h}, {0, 1, 0}, {1, 1}}, {{-h,  h,  h}, {0, 1, 0}, {0, 1}},
        // -Y face
        {{-h, -h,  h}, {0, -1, 0}, {0, 0}}, {{ h, -h,  h}, {0, -1, 0}, {1, 0}},
        {{ h, -h, -h}, {0, -1, 0}, {1, 1}}, {{-h, -h, -h}, {0, -1, 0}, {0, 1}},
        // +Z face
        {{-h, -h,  h}, {0, 0, 1}, {0, 0}}, {{ h, -h,  h}, {0, 0, 1}, {1, 0}},
        {{ h,  h,  h}, {0, 0, 1}, {1, 1}}, {{-h,  h,  h}, {0, 0, 1}, {0, 1}},
        // -Z face
        {{ h, -h, -h}, {0, 0, -1}, {0, 0}}, {{-h, -h, -h}, {0, 0, -1}, {1, 0}},
        {{-h,  h, -h}, {0, 0, -1}, {1, 1}}, {{ h,  h, -h}, {0, 0, -1}, {0, 1}},
    };

    std::vector<unsigned int> indices;
    indices.reserve(36);
    for (unsigned int face = 0; face < 6; ++face) {
        const unsigned int base = face * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
        indices.push_back(base + 0);
    }

    return Mesh(vertices, indices);
}

Mesh Mesh::createPlane(float size) {
    const float h = size * 0.5f;

    const std::vector<Vertex> vertices = {
        {{-h, 0.0f, -h}, {0, 1, 0}, {0, 0}},
        {{ h, 0.0f, -h}, {0, 1, 0}, {1, 0}},
        {{ h, 0.0f,  h}, {0, 1, 0}, {1, 1}},
        {{-h, 0.0f,  h}, {0, 1, 0}, {0, 1}},
    };

    const std::vector<unsigned int> indices = {
        0, 1, 2,
        2, 3, 0,
    };

    return Mesh(vertices, indices);
}

Mesh Mesh::createSphere(int stacks, int slices, float radius) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (int stack = 0; stack <= stacks; ++stack) {
        const float v = static_cast<float>(stack) / static_cast<float>(stacks);
        const float phi = v * glm::pi<float>();

        for (int slice = 0; slice <= slices; ++slice) {
            const float u = static_cast<float>(slice) / static_cast<float>(slices);
            const float theta = u * glm::two_pi<float>();

            const float x = std::sin(phi) * std::cos(theta);
            const float y = std::cos(phi);
            const float z = std::sin(phi) * std::sin(theta);

            const glm::vec3 normal(x, y, z);
            vertices.push_back({normal * radius, normal, {u, 1.0f - v}});
        }
    }

    const unsigned int ringVertexCount = static_cast<unsigned int>(slices) + 1;
    for (int stack = 0; stack < stacks; ++stack) {
        for (int slice = 0; slice < slices; ++slice) {
            const unsigned int first = static_cast<unsigned int>(stack) * ringVertexCount + static_cast<unsigned int>(slice);
            const unsigned int second = first + ringVertexCount;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(first + 1);
            indices.push_back(second);
            indices.push_back(second + 1);
        }
    }

    return Mesh(vertices, indices);
}
