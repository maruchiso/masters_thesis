#include "core/Camera.h"

#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include <algorithm>

Camera::Camera(float aspectRatio)
    : m_position(0.0f, 0.0f, 2.2f),
      m_worldUp(0.0f, 1.0f, 0.0f),
      m_yaw(-90.0f),
      m_pitch(0.0f),
      m_fovY(60.0f),
      m_aspectRatio(aspectRatio),
      m_moveSpeed(2.0f),
      m_mouseSensitivity(0.1f) {
}

void Camera::setAspectRatio(float aspectRatio) {
    m_aspectRatio = aspectRatio;
}

void Camera::processKeyboard(
    bool moveForward,
    bool moveBackward,
    bool moveLeft,
    bool moveRight,
    float deltaTime
) {
    const float velocity = m_moveSpeed * deltaTime;
    const glm::vec3 forward = front();
    const glm::vec3 side = right();

    if (moveForward) {
        m_position += forward * velocity;
    }
    if (moveBackward) {
        m_position -= forward * velocity;
    }
    if (moveLeft) {
        m_position -= side * velocity;
    }
    if (moveRight) {
        m_position += side * velocity;
    }
}

void Camera::processMouseDelta(float xOffset, float yOffset) {
    m_yaw += xOffset * m_mouseSensitivity;
    m_pitch += yOffset * m_mouseSensitivity;
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
}

void Camera::setPose(const glm::vec3& position, float yaw, float pitch) {
    m_position = position;
    m_yaw = yaw;
    m_pitch = std::clamp(pitch, -89.0f, 89.0f);
}

glm::mat4 Camera::viewMatrix() const {
    return glm::lookAt(m_position, m_position + front(), m_worldUp);
}

glm::mat4 Camera::projectionMatrix() const {
    return glm::perspective(glm::radians(m_fovY), m_aspectRatio, 0.1f, 100.0f);
}

glm::vec3 Camera::position() const {
    return m_position;
}

glm::vec3 Camera::front() const {
    const float yawRad = glm::radians(m_yaw);
    const float pitchRad = glm::radians(m_pitch);

    const glm::vec3 direction(
        glm::cos(yawRad) * glm::cos(pitchRad),
        glm::sin(pitchRad),
        glm::sin(yawRad) * glm::cos(pitchRad)
    );

    return glm::normalize(direction);
}

glm::vec3 Camera::right() const {
    return glm::normalize(glm::cross(front(), m_worldUp));
}
