# Magister Renderer

Minimal OpenGL project scaffold for the thesis:

Implementation and comparative analysis of Deferred Shading and Visibility Buffer in real-time.

## Current status
- CMake project created
- GLFW + GLAD + GLM integrated via FetchContent
- Basic OpenGL window and render loop implemented
- Free-fly camera (WASD + mouse look)
- Shader loading abstraction, with `setMat4`/`setMat3`/`setVec3`/`setFloat`/`setInt` uniform helpers
- Generic `Mesh` (position/normal/UV vertices) with `createCube`/`createPlane`/`createSphere` factories
- Generic `Scene`/`SceneObject`/`Material`/`PointLight` replacing the old hardcoded cube-only scene
- `Framebuffer` wrapper for offscreen render targets (not wired into the render loop yet -- this is the base the Deferred Shading G-buffer and Visibility Buffer targets will both build on)
- Minimal forward-lit shader (`shaders/lit.vert`/`lit.frag`, Lambertian + point lights) used to validate the scene before either comparison pipeline exists

## Project structure
- `CMakeLists.txt`
- `PROJECT_PLAN.md`
- `README.md`
- `include/core/Vertex.h`, `Mesh.h`, `Material.h`, `Light.h`, `SceneObject.h`, `Scene.h`
- `include/core/Shader.h`, `Camera.h`, `Framebuffer.h`
- `src/main.cpp`
- `src/rendering/Shader.cpp`, `Camera.cpp`, `Mesh.cpp`, `Scene.cpp`, `Framebuffer.cpp`
- `shaders/lit.vert`, `shaders/lit.frag` (current default)
- `shaders/fullscreen.vert`, `shaders/solid_color.frag` (superseded depth-tint debug shader, kept for reference)

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
1. ~~Add camera and basic scene abstraction~~ (done)
2. Add mesh loading (minimal path), if an external test scene like Sponza is used later
3. Implement Deferred Shading pass pipeline (geometry pass into `Framebuffer` G-buffer, then a lighting pass)
4. Implement Visibility Buffer pipeline (vertex-pulling via SSBOs, `gl_PrimitiveID` visibility target, resolve/shading pass sharing the lighting function in `lit.frag`)
5. Add benchmark logging for comparison (GPU timer queries per pass, CSV output, light-count sweeps)

## Test controls
- Move camera: `W`, `A`, `S`, `D`
- Look around: mouse movement
- Exit: `ESC`

If mouse input feels locked, click inside the window first.

## Recommended test environment strategy
- Start with the built-in minimal scene (current default) for camera/input and pipeline correctness.
- Add a medium-complexity model later (for example Sponza) only after both rendering paths are implemented.

This keeps development risk low in a 2-month timeline and still allows strong final comparison on a known benchmark scene.


## Articles
Deferred shading is detailed discribed in: https://learnopengl.com/Advanced-Lighting/Deferred-Shading
Visibility buffer article is in folder ./docs/Burns2013Visibility.pdf