#include "core/SimpleScene.h"

#include "core/Shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>

SimpleScene::SimpleScene()
    : m_vao(0),
      m_vbo(0),
      m_ebo(0),
      m_cubePositions({
          glm::vec3(0.0f, 0.0f, 0.0f),
          glm::vec3(2.0f, 0.5f, -2.0f),
          glm::vec3(-2.0f, -0.3f, -1.5f),
          glm::vec3(1.0f, -0.5f, -4.0f),
          glm::vec3(-1.2f, 0.8f, -3.2f),
          glm::vec3(0.0f, -1.0f, -6.0f)
      }) {
}

SimpleScene::~SimpleScene() {
    if (m_ebo != 0) {
        glDeleteBuffers(1, &m_ebo);
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
    }
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
    }
}

void SimpleScene::initialize() {
    const std::array<float, 24> cubeVertices = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f
    };

    const std::array<unsigned int, 36> cubeIndices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        4, 5, 1, 1, 0, 4,
        7, 6, 2, 2, 3, 7,
        4, 7, 3, 3, 0, 4,
        5, 6, 2, 2, 1, 5
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(cubeVertices.size() * sizeof(float)),
        cubeVertices.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(cubeIndices.size() * sizeof(unsigned int)),
        cubeIndices.data(),
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SimpleScene::update(float /*elapsedSeconds*/) {
}

void SimpleScene::render(const Shader& shader, const glm::mat4& viewProjection) const {
    shader.bind();

    glBindVertexArray(m_vao);

    for (size_t i = 0; i < m_cubePositions.size(); ++i) {
        const glm::vec3& position = m_cubePositions[i];

        glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
        model = glm::rotate(model, static_cast<float>(i) * 0.22f, glm::vec3(0.2f, 1.0f, 0.4f));

        const glm::mat4 mvp = viewProjection * model;
        shader.setMat4("uMVP", glm::value_ptr(mvp));

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    }

    glBindVertexArray(0);
}
