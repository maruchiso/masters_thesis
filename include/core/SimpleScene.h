#pragma once

#include <glad/glad.h>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <vector>

class Shader;

class SimpleScene {
public:
    SimpleScene();
    ~SimpleScene();

    SimpleScene(const SimpleScene&) = delete;
    SimpleScene& operator=(const SimpleScene&) = delete;

    void initialize();
    void update(float elapsedSeconds);
    void render(const Shader& shader, const glm::mat4& viewProjection) const;

private:
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ebo;
    std::vector<glm::vec3> m_cubePositions;
};
