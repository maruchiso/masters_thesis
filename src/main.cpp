#include "core/Camera.h"
#include "core/Shader.h"
#include "core/SimpleScene.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <exception>
#include <iostream>
#include <stdexcept>

namespace {

Camera* g_camera = nullptr;
bool g_firstMouseEvent = true;
double g_lastMouseX = 0.0;
double g_lastMouseY = 0.0;

}

namespace {

void framebufferSizeCallback(GLFWwindow* /*window*/, int width, int height) {
    glViewport(0, 0, width, height);
    if (g_camera != nullptr && height > 0) {
        g_camera->setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    }
}

void mouseMoveCallback(GLFWwindow* /*window*/, double xpos, double ypos) {
    if (g_camera == nullptr) {
        return;
    }

    if (g_firstMouseEvent) {
        g_lastMouseX = xpos;
        g_lastMouseY = ypos;
        g_firstMouseEvent = false;
        return;
    }

    const float xOffset = static_cast<float>(xpos - g_lastMouseX);
    const float yOffset = static_cast<float>(g_lastMouseY - ypos);

    g_lastMouseX = xpos;
    g_lastMouseY = ypos;

    g_camera->processMouseDelta(xOffset, yOffset);
}

void processInput(GLFWwindow* window, Camera& camera, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    camera.processKeyboard(
        glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS,
        glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS,
        glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS,
        glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS,
        deltaTime
    );
}

} // namespace

int main() {
    if (glfwInit() == GLFW_FALSE) {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Magister Renderer", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseMoveCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(1);

    const int gladStatus = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    if (gladStatus == 0) {
        std::cerr << "Failed to initialize GLAD." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    try {
        Shader shader("shaders/fullscreen.vert", "shaders/solid_color.frag");

        Camera camera(1280.0f / 720.0f);
        g_camera = &camera;

        SimpleScene scene;
        scene.initialize();

        float previousTime = static_cast<float>(glfwGetTime());

        glEnable(GL_DEPTH_TEST);

        while (glfwWindowShouldClose(window) == GLFW_FALSE) {
            const float currentTime = static_cast<float>(glfwGetTime());
            const float deltaTime = currentTime - previousTime;
            previousTime = currentTime;

            processInput(window, camera, deltaTime);
            scene.update(currentTime);

            const auto viewProjection = camera.projectionMatrix() * camera.viewMatrix();

            glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            scene.render(shader, viewProjection);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        g_camera = nullptr;
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << std::endl;
        g_camera = nullptr;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
