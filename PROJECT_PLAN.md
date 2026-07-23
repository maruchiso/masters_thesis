# Master Thesis Project Plan

## Thesis Title
Implementation and comparative analysis of Deferred Shading and Visibility Buffer in real-time

## Goal
Deliver a minimal but complete OpenGL-based implementation of Deferred Shading and Visibility Buffer, compare them on the same scene, and document performance and implementation trade-offs.

## Thesis Contribution
Implementing both pipelines alone is not the contribution -- Deferred Shading and Visibility Buffer are both established techniques (Burns & Hunt 2013 for Visibility Buffer), and comparisons between them already exist in the literature. The contribution of this thesis is:

A controlled, shared-codebase comparison of Deferred Shading and Visibility Buffer under identical scene, lighting math, and hardware conditions in OpenGL, characterizing the light-count and geometry-density crossover point at which each pipeline wins, and explaining that crossover in terms of the underlying GPU cost (bandwidth vs. shading redundancy vs. vertex processing).

This must be confirmed with the thesis advisor, but it is the working framing for what makes this more than a re-implementation exercise. It directly shapes what "benchmarking and analysis" (see below) needs to produce: not just frame-time numbers, but a crossover curve plus an architectural explanation of why it occurs.

## Project Scope
- Use OpenGL as the rendering API
- Implement a simple rendering engine in C++ with GLFW for window/input and GLAD for OpenGL loading
- Build a small controlled scene with a few meshes and materials
- Scene must support runtime-configurable light count and geometry density (not hand-placed objects), so both can be swept for benchmarking -- done: `Scene::initialize(lightCount, objectCount)` spawns a ring of lights and a 3D grid of cubes procedurally; live-adjustable via Up/Down/Left/Right while the app runs, and the same signature is what the Week 7 harness will call programmatically
- Light count is capped at `kMaxLights = 64` (centralized in `include/core/Light.h`, matched by `MAX_LIGHTS` in `shaders/common/lighting.glsl`) -- a plain uniform array, not an SSBO-backed unbounded list, since 64 is generous enough for the sweeps and avoids std140/std430 layout-matching complexity between C++ and GLSL. Revisit only if a benchmark genuinely needs more
- Implement:
  - Forward Rendering pipeline (baseline)
  - Deferred Shading pipeline
  - Visibility Buffer pipeline
- All three pipelines render the same `Scene` data (same objects, same lights, same camera) through separate, swappable renderer classes -- this is what makes "identical rendering conditions" true rather than just claimed
- All shading passes call the same `evaluatePointLight()` from `shaders/common/lighting.glsl`, spliced in via a lightweight `#include` preprocessor in `Shader` -- one copy of the lighting math, not one per pipeline, so it cannot silently drift
- Collect comparison data across:
  - frame time / FPS, broken down per GPU pass (geometry/visibility pass vs. lighting/shading pass), via GPU timer queries -- done: `GpuTimer` (double-buffered `GL_TIME_ELAPSED` queries, non-stalling) wraps each pass in `ForwardRenderer`/`DeferredRenderer`, live in the app's title bar and console (updated twice/sec). Crucially, this is unaffected by vsync, unlike CPU frame time -- the reliable signal even when both pipelines stay under the monitor's frame budget
  - CPU execution time (wall-clock around submission), alongside GPU time, per pass -- done, same live readout; a V key toggles vsync so the CPU-side FPS number isn't capped at the monitor refresh rate during manual testing
  - shader invocation counts via `ARB_pipeline_statistics_query` (native OpenGL, no external profiler needed) -- direct evidence for the "shade once per pixel vs. once per pixel per overdraw layer" story
  - memory usage / G-buffer size vs. visibility buffer size, computed analytically from formats + resolution (not measured via vendor-specific VRAM counters)
  - lighting scalability: frame time vs. light count sweep, all three pipelines, geometry held fixed -- this is where Forward Rendering is expected to fall off fastest
  - geometry scalability: frame time vs. triangle count sweep, all three pipelines, light count held fixed
  - overdraw scalability: frame time vs. overlapping-geometry layers, all three pipelines, reusing the same object-spawning mechanism as the geometry sweep
  - the crossover point(s) where one pipeline overtakes another, and why (bandwidth-bound vs. compute-bound reasoning)
  - visual correctness (all three pipelines must produce matching output for the same scene, using the same shared lighting function), plus a qualitative side-by-side screenshot comparison of edge aliasing
- Keep the implementation minimal: no complex PBR, no shadows, no volumetrics, no post-processing beyond simple output
- MSAA support and multi-material variety are discussed in the thesis (both are real, literature-backed advantages of Visibility Buffer) but not implemented -- see "Out of Scope" below

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
3. Forward Rendering (baseline) -- done
   - Single pass: geometry and lighting evaluated together, once per object, no intermediate buffer
   - `ForwardRenderer` owns its shader and draw loop; `Scene` stays pure data so Deferred/Visibility Buffer renderers can consume the same data
   - Purpose: the "before" picture -- demonstrates the O(objects x lights) cost that motivates Deferred Shading and Visibility Buffer existing at all
4. Deferred Shading -- done
   - Geometry pass: `gbuffer.vert`/`.frag` write albedo (RGBA8) and world-space normal (RGB16F) into a `Framebuffer` G-buffer; depth comes from the hardware depth test, no explicit output needed
   - No position attachment -- deliberately reconstructed from depth + inverse view-projection in the lighting pass instead, to keep the G-buffer as small as a real implementation would (matters directly for the memory/bandwidth comparison against Visibility Buffer)
   - Lighting pass: `deferred_lighting.vert`/`.frag`, a procedural fullscreen triangle (no vertex buffer) that samples the G-buffer and shades each pixel exactly once via the shared `evaluatePointLight()`
   - `DeferredRenderer` owns both shaders, the G-buffer, and a resize() hook wired to the window resize callback
   - Runtime toggle: press Tab to switch between Forward and Deferred at runtime for visual comparison, no recompile needed
5. Visibility Buffer
   - Visibility pass: produce visible triangle/primitive IDs and depth
   - Resolve pass: build visible list
   - Shading pass: shade using the visible data
6. Architecture Analysis (written alongside implementation, not deferred to Week 8)
   - Data flow diagrams for all three pipelines (geometry/visibility pass -> intermediate buffer, if any -> shading pass -> output)
   - Buffer organization and memory layout comparison (no intermediate buffer vs. G-buffer attachments vs. single packed visibility target)
   - Analytical memory consumption, memory access count, and bandwidth calculations per pass, derived from formats + resolution + light/object counts -- computed, not measured with an external profiler
   - Deferred G-buffer cost as built: 4 (albedo) + 6 (normal) + 4 (depth) = 14 bytes/pixel -- worth citing directly against Visibility Buffer's ~4 bytes/pixel
7. Benchmarking and analysis
   - Runtime-configurable light count/geometry density -- done (`Scene::initialize`). Per-pass GPU + CPU timing -- done (`GpuTimer`, live readout). Still needed: an actual sweep harness that drives these programmatically across a range of values and writes CSV output, rather than a human holding down arrow keys
   - Run controlled sweeps: (a) light count, (b) geometry density, (c) overdraw layers -- each with the other variables fixed, as separate 1D sweeps rather than a full cross-product grid, to stay within the timeline
   - Measure frame time, per-pass GPU time, CPU time, and shader invocation counts; multiple runs, report stability/variance
   - Log bandwidth/texture memory metrics (G-buffer size vs. visibility buffer size, computed from formats + resolution)
   - Plot frame time vs. swept variable for both pipelines, identify the crossover point(s)
   - Interpret the crossover architecturally (bandwidth vs. compute vs. vertex cost) -- this interpretation is the actual thesis contribution, not the raw numbers
   - Compare visual output for correctness (screenshot diff) and aliasing behavior, and write conclusions

## Out of Scope (Discussed in the Thesis, Not Implemented)
Considered and deliberately cut, to keep the 2-month timeline realistic for a solo implementation. Each is still worth a paragraph in the thesis (background/future work), citing the literature instead of building it:
- **MSAA compatibility** -- one of Visibility Buffer's headline advantages in the literature (cheap per-sample visibility vs. expensive per-sample G-buffers), but implementing MSAA support in both pipelines is real added engineering. Discuss, don't build, unless earlier phases finish with time to spare.
- **Multiple material configurations / textures** -- thematically relevant to Visibility Buffer's material-flexibility argument, but requires a texture/material system that doesn't exist yet and isn't essential to the core bandwidth-vs-shading-redundancy question. Discuss, don't build.
- **Cache misses, GPU occupancy** -- not accessible via plain OpenGL; require external vendor profilers (Nsight Graphics, Radeon GPU Profiler). A genuinely separate tooling investment. Optional stretch goal only, attempted after everything else works.
- **Transparency handling, deferred decal support, post-processing integration** -- each a standalone rendering feature with no direct bearing on the core architectural comparison, and each contradicts the "minimal implementation" scope decision above. Cut outright.

## Minimal Deliverables
- `CMakeLists.txt`
- `src/` code for renderer and application
- `include/` headers
- `shaders/` GLSL files
- Benchmark harness (light count / geometry density / overdraw sweeps, GPU timer queries, CSV output) and the resulting plots/data
- Architecture analysis write-up: data flow diagrams, buffer layout comparison, analytical memory/bandwidth calculations
- `PROJECT_PLAN.md`, `README.md`, and `docs/CODE_WALKTHROUGH.md`

## 8-Week Timeline
- Week 1: Setup project and OpenGL basics
- Week 2: Build a stable rendering loop and generic scene foundation (Mesh/Material/Light/SceneObject, expanded Shader uniforms, Framebuffer wrapper) -- done ahead of Deferred Shading so both pipelines share it
- Week 3: Forward Rendering baseline and Deferred Shading both implemented (`ForwardRenderer`/`DeferredRenderer`, shared lighting via `shaders/common/lighting.glsl`, runtime toggle), and `Scene` is now procedurally configurable (light count, object count) instead of hardcoded. Still need: the architecture-analysis write-up while it's fresh
- Week 4: Validate Deferred Shading and fix bugs
- Week 5: Implement Visibility Buffer, sharing the same lighting function for a fair visual-correctness comparison; draft its architecture-analysis section too
- Week 6: Validate Visibility Buffer and compare correctness
- Week 7: Build the benchmark harness, run the light-count/geometry-density/overdraw sweeps, find and interpret the crossover point(s)
- Week 8: Write thesis chapters (start Introduction/Background earlier, in parallel, rather than only in Week 8) and finalize documentation

## Next step
Forward Rendering and Deferred Shading are both implemented and toggleable at runtime (Tab key), `Scene` is procedurally configurable (Up/Down = light count, Left/Right = object count), and both pipelines report live GPU per-pass timing + CPU frame time (title bar and console, updated twice/sec; V toggles vsync). Next: implement Visibility Buffer -- visibility pass writing packed triangle/object IDs, resolve pass doing vertex-pulling via SSBOs, shading pass reusing `evaluatePointLight()`. The benchmark *harness* itself (programmatic sweeps + CSV export, vs. a human holding arrow keys) is still Week 7 work, not yet started.
