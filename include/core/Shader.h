#pragma once

#include <glad/glad.h>

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

private:
    GLuint m_program;

    static std::string readTextFile(const std::string& path);
    static GLuint compileStage(GLenum type, const std::string& source, const std::string& debugName);
    static GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);
};
