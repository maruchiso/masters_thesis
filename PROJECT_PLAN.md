# Master Thesis Project Plan

## Thesis Title
Implementation and comparative analysis of Deferred Shading and Visibility Buffer in real-time

## Goal
Deliver a minimal but complete OpenGL-based implementation of Deferred Shading and Visibility Buffer, compare them on the same scene, and document performance and implementation trade-offs.

## Project Scope
- Use OpenGL as the rendering API
- Implement a simple rendering engine in C++ with GLFW for window/input and GLAD for OpenGL loading
- Build a small controlled scene with a few meshes and materials
- Implement:
  - Deferred Shading pipeline
  - Visibility Buffer pipeline
- Collect comparison data across:
  - frame time / FPS
  - memory usage / G-buffer size
  - lighting scalability
  - visual correctness
- Keep the implementation minimal: no complex PBR, no shadows, no volumetrics, no post-processing beyond simple output

## Key Components
1. Project setup
   - CMake project
   - GLFW window and input
   - GLAD OpenGL context
   - Shader loader
   - Basic application loop
2. Scene setup
   - Simple geometry: cubes, spheres, plane
   - Camera controls (orbit + WASD)
   - One texture or solid-color material
3. Deferred Shading
   - Geometry pass: store albedo, normal, depth, material properties
   - Lighting pass: evaluate lights using G-buffer
   - Composite pass to display final image
4. Visibility Buffer
   - Visibility pass: produce visible triangle/primitive IDs and depth
   - Resolve pass: build visible list
   - Shading pass: shade using the visible data
5. Benchmarking and analysis
   - Run the same scene on both pipelines
   - Measure frame time and stability
   - Log bandwidth/texture memory metrics
   - Compare results and write conclusions

## Minimal Deliverables
- `CMakeLists.txt`
- `src/` code for renderer and application
- `include/` headers
- `shaders/` GLSL files
- `PROJECT_PLAN.md` and later a `README.md`

## 8-Week Timeline
- Week 1: Setup project and OpenGL basics
- Week 2: Build a stable rendering loop and simple scene
- Week 3: Implement Deferred Shading
- Week 4: Validate Deferred Shading and fix bugs
- Week 5: Implement Visibility Buffer
- Week 6: Validate Visibility Buffer and compare correctness
- Week 7: Benchmark and collect data
- Week 8: Write thesis chapters and finalize documentation

## Next step
Create the OpenGL/CMake scaffold with a working window, shader loading, and a minimal render loop.
