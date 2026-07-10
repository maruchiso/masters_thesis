#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class Camera {
public:
    Camera(float aspectRatio);

    void setAspectRatio(float aspectRatio);
    void processKeyboard(bool moveForward, bool moveBackward, bool moveLeft, bool moveRight, float deltaTime);
    void processMouseDelta(float xOffset, float yOffset);

    glm::mat4 viewMatrix() const;
    glm::mat4 projectionMatrix() const;
    glm::vec3 position() const;

private:
    glm::vec3 m_position;
    glm::vec3 m_worldUp;

    float m_yaw;
    float m_pitch;
    float m_fovY;
    float m_aspectRatio;
    float m_moveSpeed;
    float m_mouseSensitivity;

    glm::vec3 front() const;
    glm::vec3 right() const;
};
