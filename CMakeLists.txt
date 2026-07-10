cmake_minimum_required(VERSION 3.20)
project(MagisterRenderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v0.1.36
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)

FetchContent_MakeAvailable(glfw glad glm)

add_executable(MagisterRenderer
    src/main.cpp
    src/rendering/Camera.cpp
    src/rendering/Shader.cpp
    src/rendering/SimpleScene.cpp
)

target_include_directories(MagisterRenderer PRIVATE
    include
)

target_link_libraries(MagisterRenderer PRIVATE
    glfw
    glad
    glm
)

if (WIN32)
    target_link_libraries(MagisterRenderer PRIVATE opengl32)
endif()
