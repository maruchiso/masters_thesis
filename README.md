# Magister Renderer

Minimal OpenGL project scaffold for the thesis:

Implementation and comparative analysis of Deferred Shading and Visibility Buffer in real-time.

## Current status
- CMake project created
- GLFW + GLAD integrated via FetchContent
- Basic OpenGL window and render loop implemented
- Shader loading abstraction implemented
- Starter shaders added

## Project structure
- `CMakeLists.txt`
- `PROJECT_PLAN.md`
- `README.md`
- `include/core/Shader.h`
- `src/main.cpp`
- `src/rendering/Shader.cpp`
- `shaders/fullscreen.vert`
- `shaders/solid_color.frag`

## Prerequisites (Windows)
Install one of these toolchains:

1. Visual Studio 2022 with C++ workload (recommended)
   - Workload: Desktop development with C++
   - CMake tools included
2. Or MinGW-w64 + Ninja

Also install:
- CMake 3.20+
- Git

## Build (Visual Studio generator)
Run in Developer Command Prompt for VS 2022:

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Run executable:

```bat
build\Release\MagisterRenderer.exe
```

## Build (Ninja + MSVC)
Run in Developer Command Prompt for VS 2022:

```bat
cmake -S . -B build -G Ninja
cmake --build build
```

Run executable:

```bat
build\MagisterRenderer.exe
```

## Notes
If configuration fails with errors about `nmake` or compiler not found, your C++ toolchain is not active in the current terminal. Use Developer Command Prompt or open VS Code from that prompt.

## Next development steps
1. Add camera and basic scene abstraction
2. Add mesh loading (minimal path)
3. Implement Deferred Shading pass pipeline
4. Implement Visibility Buffer pipeline
5. Add benchmark logging for comparison

## Test controls
- Move camera: `W`, `A`, `S`, `D`
- Look around: mouse movement
- Exit: `ESC`

If mouse input feels locked, click inside the window first.

## Recommended test environment strategy
- Start with the built-in minimal scene (current default) for camera/input and pipeline correctness.
- Add a medium-complexity model later (for example Sponza) only after both rendering paths are implemented.

This keeps development risk low in a 2-month timeline and still allows strong final comparison on a known benchmark scene.
