#pragma once

#include "core/Vertex.h"

#include <glad/glad.h>

#include <vector>

// Owns a VAO/VBO/EBO for a single piece of static geometry.
// Move-only: copying a live GPU buffer set makes no sense.
class Mesh {
public:
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void draw() const;

    GLsizei indexCount() const;

    static Mesh createCube(float size = 1.0f);
    static Mesh createPlane(float size = 10.0f);
    static Mesh createSphere(int stacks = 16, int slices = 24, float radius = 0.5f);

private:
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ebo;
    GLsizei m_indexCount;

    void upload(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    void release();
};
