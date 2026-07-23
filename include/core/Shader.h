#pragma once

#include <glad/glad.h>

#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <string>

class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void bind() const;
    GLuint id() const;

    void setMat4(const std::string& uniformName, const float* matrixData) const;
    void setMat4(const std::string& uniformName, const glm::mat4& matrix) const;
    void setMat3(const std::string& uniformName, const glm::mat3& matrix) const;
    void setVec3(const std::string& uniformName, const glm::vec3& value) const;
    void setFloat(const std::string& uniformName, float value) const;
    void setInt(const std::string& uniformName, int value) const;

private:
    GLuint m_program;

    static std::string readTextFile(const std::string& path);

    // Textual #include "relative/path.glsl" preprocessing -- GLSL has no native #include
    // in core OpenGL. Paths are resolved relative to the directory of the file that
    // contains the #include line, the same convention as C/C++'s #include "...".
    static std::string resolveIncludes(const std::string& source, const std::string& baseDir);

    static GLuint compileStage(GLenum type, const std::string& source, const std::string& debugName);
    static GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);
};
