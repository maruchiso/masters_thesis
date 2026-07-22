#include "core/Shader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
    : m_program(0) {
    const std::string vertexSource = readTextFile(vertexPath);
    const std::string fragmentSource = readTextFile(fragmentPath);

    GLuint vertexShader = 0;
    GLuint fragmentShader = 0;

    try {
        vertexShader = compileStage(GL_VERTEX_SHADER, vertexSource, vertexPath);
        fragmentShader = compileStage(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);
        m_program = linkProgram(vertexShader, fragmentShader);
    } catch (...) {
        if (vertexShader != 0) {
            glDeleteShader(vertexShader);
        }
        if (fragmentShader != 0) {
            glDeleteShader(fragmentShader);
        }
        throw;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader() {
    if (m_program != 0) {
        glDeleteProgram(m_program);
    }
}

Shader::Shader(Shader&& other) noexcept
    : m_program(other.m_program) {
    other.m_program = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    if (m_program != 0) {
        glDeleteProgram(m_program);
    }

    m_program = other.m_program;
    other.m_program = 0;

    return *this;
}

void Shader::bind() const {
    glUseProgram(m_program);
}

GLuint Shader::id() const {
    return m_program;
}

void Shader::setMat4(const std::string& uniformName, const float* matrixData) const {
    const GLint location = glGetUniformLocation(m_program, uniformName.c_str());
    if (location >= 0) {
        glUniformMatrix4fv(location, 1, GL_FALSE, matrixData);
    }
}

void Shader::setMat4(const std::string& uniformName, const glm::mat4& matrix) const {
    setMat4(uniformName, &matrix[0][0]);
}

void Shader::setMat3(const std::string& uniformName, const glm::mat3& matrix) const {
    const GLint location = glGetUniformLocation(m_program, uniformName.c_str());
    if (location >= 0) {
        glUniformMatrix3fv(location, 1, GL_FALSE, &matrix[0][0]);
    }
}

void Shader::setVec3(const std::string& uniformName, const glm::vec3& value) const {
    const GLint location = glGetUniformLocation(m_program, uniformName.c_str());
    if (location >= 0) {
        glUniform3fv(location, 1, &value[0]);
    }
}

void Shader::setFloat(const std::string& uniformName, float value) const {
    const GLint location = glGetUniformLocation(m_program, uniformName.c_str());
    if (location >= 0) {
        glUniform1f(location, value);
    }
}

void Shader::setInt(const std::string& uniformName, int value) const {
    const GLint location = glGetUniformLocation(m_program, uniformName.c_str());
    if (location >= 0) {
        glUniform1i(location, value);
    }
}

std::string Shader::readTextFile(const std::string& path) {
    std::ifstream file(path, std::ios::in);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint Shader::compileStage(GLenum type, const std::string& source, const std::string& debugName) {
    const GLuint shader = glCreateShader(type);
    const char* sourcePtr = source.c_str();
    glShaderSource(shader, 1, &sourcePtr, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<char> infoLog(static_cast<size_t>(logLength));
        glGetShaderInfoLog(shader, logLength, nullptr, infoLog.data());

        glDeleteShader(shader);
        throw std::runtime_error(
            "Shader compilation failed (" + debugName + "): " + std::string(infoLog.data())
        );
    }

    return shader;
}

GLuint Shader::linkProgram(GLuint vertexShader, GLuint fragmentShader) {
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        GLint logLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<char> infoLog(static_cast<size_t>(logLength));
        glGetProgramInfoLog(program, logLength, nullptr, infoLog.data());

        glDeleteProgram(program);
        throw std::runtime_error("Program linking failed: " + std::string(infoLog.data()));
    }

    return program;
}
