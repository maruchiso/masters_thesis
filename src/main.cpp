#include "core/Camera.h"
#include "core/DeferredRenderer.h"
#include "core/ForwardRenderer.h"
#include "core/Light.h"
#include "core/Scene.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

#include <algorithm>
#include <exception>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

enum class RenderMode { Forward, Deferred };

// Fixed (lights, objects, camera pose) combos for reproducible A/B comparisons -- select
// one with number keys 1-4, then Tab freely between Forward/Deferred knowing the scene and
// viewpoint are frozen. Camera Z is pulled back further for the denser presets so the grid
// (which grows in all three axes with object count, see Scene::initialize) stays in view.
struct BenchmarkPreset {
    const char* label;
    int lightCount;
    int objectCount;
    glm::vec3 cameraPosition;
    float cameraYaw;
    float cameraPitch;
};

// const, not constexpr: glm::vec3's constructor being constexpr isn't guaranteed across
// GLM configurations, and there's no need for compile-time evaluation here anyway.
const BenchmarkPreset kBenchmarkPresets[] = {
    {"Low",     2,  5,    glm::vec3(0.0f, 0.0f, 4.0f),  -90.0f, 0.0f},
    {"Medium",  16, 200,  glm::vec3(0.0f, 0.0f, 8.0f),  -90.0f, 0.0f},
    {"High",    64, 800,  glm::vec3(0.0f, 0.0f, 12.0f), -90.0f, 0.0f},
    {"Extreme", 64, 2000, glm::vec3(0.0f, 0.0f, 16.0f), -90.0f, 0.0f},
};

Camera* g_camera = nullptr;
DeferredRenderer* g_deferredRenderer = nullptr;
Scene* g_scene = nullptr;
RenderMode g_renderMode = RenderMode::Forward;
int g_lightCount = 2;
int g_objectCount = 5;
bool g_vsyncEnabled = true;
bool g_resetStats = false;
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
    if (g_deferredRenderer != nullptr && width > 0 && height > 0) {
        g_deferredRenderer->resize(width, height);
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

// Discrete key-press events (fires once per press, not once per frame while held) --
// used for the render-mode toggle so it doesn't flicker every frame while Tab is down.
void keyCallback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/) {
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        g_renderMode = (g_renderMode == RenderMode::Forward) ? RenderMode::Deferred : RenderMode::Forward;
        std::cout << "Render mode: " << (g_renderMode == RenderMode::Forward ? "Forward" : "Deferred") << std::endl;
        g_resetStats = true;
        return;
    }

    // Benchmark presets: 1-4 snap scene complexity AND camera pose to a fixed, reproducible
    // combo. Render mode is deliberately untouched here -- press a preset once, then Tab
    // between Forward/Deferred to compare the exact same frozen configuration.
    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_4 && action == GLFW_PRESS) {
        const size_t presetIndex = static_cast<size_t>(key - GLFW_KEY_1);
        if (presetIndex < std::size(kBenchmarkPresets) && g_scene != nullptr && g_camera != nullptr) {
            const BenchmarkPreset& preset = kBenchmarkPresets[presetIndex];
            g_lightCount = preset.lightCount;
            g_objectCount = preset.objectCount;
            g_scene->initialize(g_lightCount, g_objectCount);
            g_camera->setPose(preset.cameraPosition, preset.cameraYaw, preset.cameraPitch);
            g_resetStats = true;
            std::cout << "Preset: " << preset.label << " (Lights: " << g_lightCount
                      << ", Objects: " << g_objectCount << ")" << std::endl;
        }
        return;
    }

    // Vsync caps CPU-measured frame time at the monitor refresh rate, which can hide real
    // differences between renderers if both stay under that budget. GPU per-pass timing
    // (printed below) is unaffected either way, but toggling this off makes the CPU-side
    // FPS number tell the truth too.
    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        g_vsyncEnabled = !g_vsyncEnabled;
        glfwSwapInterval(g_vsyncEnabled ? 1 : 0);
        std::cout << "Vsync: " << (g_vsyncEnabled ? "on" : "off") << std::endl;
        g_resetStats = true;
        return;
    }

    // Live scene tuning: Up/Down = light count, Left/Right = object count. GLFW_REPEAT is
    // included (not just GLFW_PRESS) so holding a key ramps the count instead of requiring
    // one press per step. This rebuilds the whole Scene each change -- fine for manual
    // testing, and Scene::initialize(N, M) is the same entry point the Week 7 benchmark
    // harness will call directly, just without a human at the keyboard driving it.
    if (action != GLFW_PRESS && action != GLFW_REPEAT) {
        return;
    }

    bool sceneChanged = false;
    if (key == GLFW_KEY_UP) {
        g_lightCount = std::min(g_lightCount + 1, kMaxLights);
        sceneChanged = true;
    } else if (key == GLFW_KEY_DOWN) {
        g_lightCount = std::max(g_lightCount - 1, 0);
        sceneChanged = true;
    } else if (key == GLFW_KEY_RIGHT) {
        g_objectCount += 4;
        sceneChanged = true;
    } else if (key == GLFW_KEY_LEFT) {
        g_objectCount = std::max(g_objectCount - 4, 0);
        sceneChanged = true;
    }

    if (sceneChanged && g_scene != nullptr) {
        g_scene->initialize(g_lightCount, g_objectCount);
        g_resetStats = true;
        std::cout << "Lights: " << g_lightCount << ", Objects: " << g_objectCount << std::endl;
    }
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
    glfwSetKeyCallback(window, keyCallback);
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
        ForwardRenderer forwardRenderer;
        DeferredRenderer deferredRenderer(1280, 720);
        g_deferredRenderer = &deferredRenderer;

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Deferred G-buffer: 14 bytes/pixel (4 albedo + 6 normal + 4 depth), "
                   << deferredRenderer.gbufferMemoryMegabytes() << " MB at 1280x720. "
                   << "Forward Rendering: 0 bytes/pixel intermediate storage (no G-buffer)."
                   << std::endl;

        Camera camera(1280.0f / 720.0f);
        g_camera = &camera;

        Scene scene;
        scene.initialize(g_lightCount, g_objectCount);
        g_scene = &scene;

        float previousTime = static_cast<float>(glfwGetTime());
        float statsTimer = 0.0f;
        int statsFrameCount = 0;

        std::cout << "Press TAB to toggle Forward/Deferred rendering." << std::endl;
        std::cout << "Press UP/DOWN to change light count, LEFT/RIGHT to change object count." << std::endl;
        std::cout << "Press 1-4 for fixed benchmark presets (Low/Medium/High/Extreme), same camera pose each time." << std::endl;
        std::cout << "Press V to toggle vsync." << std::endl;
        std::cout << "Lights: " << g_lightCount << ", Objects: " << g_objectCount << std::endl;

        while (glfwWindowShouldClose(window) == GLFW_FALSE) {
            const float currentTime = static_cast<float>(glfwGetTime());
            const float deltaTime = currentTime - previousTime;
            previousTime = currentTime;

            processInput(window, camera, deltaTime);
            scene.update(currentTime);

            glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (g_renderMode == RenderMode::Forward) {
                forwardRenderer.render(scene, camera);
            } else {
                deferredRenderer.render(scene, camera);
            }

            // A preset, mode toggle, vsync toggle, or manual count change all invalidate
            // whatever's currently accumulating -- start a fresh window instead of blending
            // pre-change and post-change frames into the same average.
            if (g_resetStats) {
                statsTimer = 0.0f;
                statsFrameCount = 0;
                g_resetStats = false;
            }

            // Update the live stats readout twice a second -- often enough to feel live,
            // rare enough not to spam the console or dominate CPU time with string formatting.
            statsTimer += deltaTime;
            ++statsFrameCount;
            if (statsTimer >= 0.5f) {
                const double avgFrameMs = static_cast<double>(statsTimer) / statsFrameCount * 1000.0;
                const double fps = static_cast<double>(statsFrameCount) / statsTimer;

                std::ostringstream stats;
                stats << (g_renderMode == RenderMode::Forward ? "Forward" : "Deferred")
                      << " | CPU " << std::fixed << std::setprecision(2) << avgFrameMs << " ms ("
                      << std::setprecision(0) << fps << " FPS)";

                if (g_renderMode == RenderMode::Forward) {
                    stats << " | GPU " << std::setprecision(3) << forwardRenderer.lastGpuMilliseconds() << " ms";
                } else {
                    stats << " | GPU geometry " << std::setprecision(3) << deferredRenderer.lastGeometryMilliseconds()
                          << " ms, lighting " << deferredRenderer.lastLightingMilliseconds() << " ms";
                }

                const std::string statsLine = stats.str();
                glfwSetWindowTitle(window, ("Magister Renderer | " + statsLine).c_str());
                std::cout << statsLine << std::endl;

                statsTimer = 0.0f;
                statsFrameCount = 0;
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        g_camera = nullptr;
        g_deferredRenderer = nullptr;
        g_scene = nullptr;
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << std::endl;
        g_camera = nullptr;
        g_deferredRenderer = nullptr;
        g_scene = nullptr;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
