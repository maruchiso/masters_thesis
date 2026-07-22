# Code Walkthrough (for someone new to computer graphics)

This explains every file in the project, assuming you know how to program but have never touched a GPU. Read Part 0 first — everything after it leans on those five ideas.

## Part 0 — five ideas you need before any of this makes sense

**1. CPU vs GPU.** Your C++ code (`main.cpp`, `Scene.cpp`, etc.) runs on the CPU. It doesn't draw anything itself — it uploads data to the graphics card's memory and issues short commands like "draw these triangles." The actual drawing happens on the GPU, running a totally different kind of program called a *shader*.

**2. Everything on screen is triangles.** A cube is 12 triangles. A sphere is a few hundred. "Rendering a 3D scene" really means: hand the GPU a big list of triangle corners (*vertices*) and let it figure out which pixels each triangle covers.

**3. Two shader stages run for every triangle.**
   - The **vertex shader** runs once *per vertex* (per corner). Its job: take a 3D point in object space and figure out where it lands on your 2D screen.
   - The **fragment shader** runs once *per pixel* that a triangle covers. Its job: decide that pixel's final color (this is where lighting happens).
   Both are tiny programs written in GLSL (a C-like shading language) that live in `.vert` and `.frag` files and get compiled at runtime, not at build time.

**4. Data reaches a shader three ways:**
   - **Attributes** (`in` in the vertex shader) — per-vertex data, like position and normal. Different for every vertex.
   - **Uniforms** — a single value set from C++ before a draw call, same for every vertex/pixel in that draw (e.g. "the camera matrix," "the object's color").
   - **Varyings** (`out` in vertex shader, matching `in` in fragment shader) — values the vertex shader computes and hands off; the GPU automatically blends ("interpolates") them across the triangle's surface for the fragment shader to use.

**5. Coordinate spaces and the MVP matrix.** A vertex position starts out in *object space* (coordinates relative to the object's own center, e.g. a cube from -0.5 to 0.5). To end up on screen it gets multiplied by three matrices in sequence:
   - **Model matrix** — moves/rotates/scales the object into the shared *world space*.
   - **View matrix** — re-expresses world space relative to the camera (as if the camera were at the origin looking down -Z).
   - **Projection matrix** — applies perspective (things farther away look smaller) and maps to *clip space*, which the GPU turns into 2D screen coordinates.
   `MVP = Projection * View * Model`. You multiply a vertex position by this single combined matrix once, in the vertex shader.

Keep these five things in mind and the rest of this doc is just "where does that idea live in the code."

---

## Part 1 — `src/main.cpp`, the entry point

```cpp
if (glfwInit() == GLFW_FALSE) { ... }
```
GLFW is the library that opens an OS window and gives you a place for OpenGL to draw into. `glfwInit()` starts it up.

```cpp
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
```
Before creating the window, we tell GLFW which OpenGL version to ask the driver for: 4.5, "core profile" (the modern API, no legacy fixed-function cruft). This version was picked deliberately — it's the minimum that reliably supports compute shaders and SSBOs, which the Visibility Buffer pipeline will need later.

```cpp
GLFWwindow* window = glfwCreateWindow(1280, 720, "Magister Renderer", nullptr, nullptr);
glfwMakeContextCurrent(window);
```
Creates a 1280x720 window and makes its OpenGL context "current" — meaning every OpenGL call after this point targets this window.

```cpp
glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
glfwSetCursorPosCallback(window, mouseMoveCallback);
glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
```
Registers two callback functions GLFW will call automatically: one when the window is resized, one whenever the mouse moves. `GLFW_CURSOR_DISABLED` hides the OS cursor and lets you read raw, unbounded mouse movement — the standard trick for first-person camera look.

```cpp
const int gladStatus = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
```
This is the one genuinely weird line. OpenGL isn't a normal linked library — the actual function implementations live inside your graphics driver, and their addresses have to be looked up at runtime. GLAD is the tool that does that lookup for every OpenGL function (`glDrawElements`, `glGenBuffers`, hundreds of them) and makes them callable as if they were normal functions. Nothing OpenGL-related works before this line runs.

```cpp
Shader shader("shaders/lit.vert", "shaders/lit.frag");
Camera camera(1280.0f / 720.0f);
Scene scene;
scene.initialize();
```
Loads/compiles the shader pair (Part 9), creates a camera with the window's aspect ratio, and builds the scene (Part 7) — geometry, materials, lights all get created here, once, before the loop starts.

```cpp
glEnable(GL_DEPTH_TEST);
```
Without this, triangles would be drawn in whatever order you issue them, so a far-away triangle could paint over a near one. Depth testing makes the GPU track, per pixel, the closest thing drawn there so far, and reject anything farther away.

```cpp
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
```
This is the **render loop** — it runs once per frame, roughly 60+ times a second:
- `deltaTime` = seconds since the last frame, used so movement speed doesn't depend on framerate.
- `processInput` reads WASD/mouse and moves the camera.
- `viewProjection` combines the camera's View and Projection matrices once per frame (the same for every object this frame — only the Model matrix changes per object, which is why V*P is precomputed here and M is applied later per-object).
- `glClearColor`/`glClear` wipes last frame's color and depth buffer so we start fresh.
- `scene.render(...)` is where all the actual drawing happens (Part 7).
- `glfwSwapBuffers` — GPUs draw to an invisible "back buffer" while the "front buffer" is what's on screen; swapping avoids ever showing a half-drawn frame.
- `glfwPollEvents` lets GLFW process OS events (keyboard, mouse, window close) so callbacks fire.

The `try/catch` around all of it exists because shader compilation and file loading throw `std::runtime_error` on failure (Part 8) — if a shader has a typo, you get a readable error printed to the console instead of a crash.

---

## Part 2 — `Camera.h` / `Camera.cpp` (unchanged from before, quick recap)

Stores position plus `yaw`/`pitch` (rotation around the vertical and horizontal axis — this is the standard first-person camera representation, avoids needing full quaternions for a simple free-fly cam).

`front()` converts yaw/pitch into a direction vector using basic trigonometry — this is literally "spherical coordinates," the same math as converting latitude/longitude to a 3D point on a globe. `viewMatrix()` uses `glm::lookAt(position, position + front(), up)` — GLM builds the View matrix for you from an eye position, a target point, and an up vector. `projectionMatrix()` uses `glm::perspective(fov, aspect, near, far)` to build the Projection matrix — `near`/`far` (0.1 and 100.0 here) define the depth range the camera can "see"; anything closer or farther gets clipped away.

---

## Part 3 — `include/core/Vertex.h`

```cpp
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};
```
One struct per corner of geometry. `position` is where the point is in object space. `normal` is a unit-length vector pointing "outward" from the surface at that point — this is what lighting math needs to know which way a surface faces relative to a light. `uv` are 2D texture coordinates (0..1 across a surface) — not used by any shader yet, but every mesh already stores them so adding textured materials later doesn't require touching the vertex format again.

---

## Part 4 — `include/core/Mesh.h` / `src/rendering/Mesh.cpp`

### The three GPU objects

```cpp
GLuint m_vao;
GLuint m_vbo;
GLuint m_ebo;
```
- **VBO** (Vertex Buffer Object) — a block of GPU memory holding the raw vertex data (position+normal+uv for every corner), copied there once.
- **EBO** (Element/Index Buffer Object) — a list of integers saying which vertices form each triangle. A cube has 24 vertices but 36 indices (6 faces × 2 triangles × 3 corners) — indices let vertices be reused instead of duplicating position/normal/uv data for every triangle a vertex touches.
- **VAO** (Vertex Array Object) — doesn't hold data itself; it's a saved "recipe" that remembers *how to read* the VBO (which floats are position, which are normal, which are uv) so you don't have to re-describe the layout every frame — you just bind the VAO and draw.

### `upload()` — this is where those three get created and linked together

```cpp
glGenVertexArrays(1, &m_vao);
glGenBuffers(1, &m_vbo);
glGenBuffers(1, &m_ebo);
glBindVertexArray(m_vao);
```
Creates the three objects and "binds" the VAO — from here until it's unbound, any buffer setup calls get recorded into this VAO's recipe.

```cpp
glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
```
Binds the VBO as the "array buffer" target, then copies the actual vertex data from CPU RAM (`vertices.data()`) to GPU memory. `GL_STATIC_DRAW` is a hint to the driver meaning "this data won't change often, optimize accordingly."

```cpp
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
```
Same idea for the index list.

```cpp
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

glEnableVertexAttribArray(2);
glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
```
This is the "recipe" the VAO remembers. Each call says: attribute slot `N` (0, 1, 2 — these numbers are exactly the `layout(location = N)` numbers you'll see in `lit.vert`) reads `3` or `2` floats, starting `offsetof(Vertex, field)` bytes into each `Vertex`-sized chunk, stepping `sizeof(Vertex)` bytes to get to the next vertex. It's telling the GPU how to slice the flat byte buffer back into position/normal/uv per vertex.

```cpp
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindVertexArray(0);
```
Unbinds everything so later, unrelated OpenGL calls don't accidentally modify this VAO.

### `draw()`

```cpp
glBindVertexArray(m_vao);
glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
glBindVertexArray(0);
```
Bind the recipe, tell the GPU "draw triangles using the currently bound index buffer, there are `m_indexCount` indices, they're unsigned ints." This single call is what actually triggers the vertex shader to run for every vertex and, eventually, the fragment shader for every pixel.

### The move constructor/destructor boilerplate

```cpp
Mesh(Mesh&& other) noexcept : ... { other.m_vao = 0; ... }
~Mesh() { release(); }
```
VAOs/VBOs/EBOs are GPU handles, not memory `Mesh` owns directly — if you copied a `Mesh` naively, both copies would think they own the same GPU buffers and both would try to delete them, causing a crash or corrupted rendering. So copying is disabled (`Mesh(const Mesh&) = delete`) and only *moving* is allowed — moving transfers the handles and zeroes out the source, so only one `Mesh` ever "owns" a given GPU buffer at a time. This is the standard C++ pattern for wrapping any non-copyable resource (same idea as `std::unique_ptr` or `std::fstream`).

### `createCube` / `createPlane` / `createSphere`

These just build a `std::vector<Vertex>` and a `std::vector<unsigned int>` in CPU memory (plain geometry math — a cube's 8 corners duplicated per face so each face can have its own flat normal; a sphere built from rings of points using `sin`/`cos`, the standard "UV sphere" technique), then hand them to the `Mesh` constructor, which runs `upload()`. Worth skimming but there's no graphics concept in the literal coordinate numbers — the interesting part is that they all end up going through the same `upload()` path.

---

## Part 5 — `Material.h` / `Light.h`

```cpp
struct Material {
    glm::vec3 albedo = glm::vec3(0.8f, 0.8f, 0.8f);
};
```
`albedo` is the base color of a surface before any lighting is applied (graphics term, not just "color" — it specifically means "how much light this surface reflects, per color channel, from all directions"). Right now it's just an RGB value; if textures get added later, this would hold a texture handle instead/as well.

```cpp
struct PointLight {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;
    float radius = 10.0f;
};
```
A point light radiates in all directions from a single point (like a bare bulb), unlike a directional light (like the sun, parallel rays, no position). `radius` isn't a visual size — it's the distance at which the light's contribution is defined to reach zero, used for the falloff math in Part 9.

---

## Part 6 — `include/core/SceneObject.h`

```cpp
glm::mat4 modelMatrix() const {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::rotate(model, glm::radians(rotationEulerDegrees.x), glm::vec3(1,0,0));
    model = glm::rotate(model, glm::radians(rotationEulerDegrees.y), glm::vec3(0,1,0));
    model = glm::rotate(model, glm::radians(rotationEulerDegrees.z), glm::vec3(0,0,1));
    model = glm::scale(model, scale);
    return model;
}
```
This builds the "Model" matrix from Part 0, point 5, from three human-friendly numbers: position, rotation, scale (this trio is often called a **TRS transform**). `glm::mat4(1.0f)` is a 4x4 identity matrix (does nothing) — the starting point. Then translate, rotate (once per axis), and scale are multiplied in, in that specific order, because matrix multiplication order changes the result (rotating-then-moving an object is different from moving-then-rotating it). This exact order — translate, then rotate, then scale — is what makes "position" mean "where the object's own center ends up in the world" regardless of its rotation/scale.

---

## Part 7 — `include/core/Scene.h` / `src/rendering/Scene.cpp`

### `initialize()`

Builds three `Mesh` objects (plane, cube, sphere) into `m_meshes`, then creates several `SceneObject`s pointing at them with different positions/colors, plus two `PointLight`s. The comment at the top matters:

```cpp
// SceneObject stores raw pointers into this vector, so nothing may be
// appended to m_meshes afterwards.
```
`SceneObject::mesh` is a raw `const Mesh*`. A `std::vector` can silently reallocate its internal storage (moving everything to a new memory block) whenever it grows past its current capacity — if that happened *after* we'd already taken `&m_meshes[0]`, that pointer would now point at garbage. The fix here is simply "create every mesh first, take addresses only after the vector is done growing," which is safe but fragile — worth remembering if this file gets extended later.

### `render()`

```cpp
shader.bind();

shader.setInt("uLightCount", lightCount);
for (int i = 0; i < lightCount; ++i) {
    shader.setVec3(prefix + "position", m_lights[i].position);
    ...
}
```
`shader.bind()` tells the GPU "use this shader program for subsequent draw calls." Then the light data gets pushed into the shader's `uLights[]` uniform array (Part 0, point 4 — uniforms). This happens once per frame, not once per object, because the lights don't change between objects.

```cpp
for (const SceneObject& object : m_objects) {
    const glm::mat4 model = object.modelMatrix();
    const glm::mat4 mvp = viewProjection * model;
    const glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(model));

    shader.setMat4("uModel", model);
    shader.setMat4("uMVP", mvp);
    shader.setMat3("uNormalMatrix", normalMatrix);
    shader.setVec3("uAlbedo", object.material.albedo);

    object.mesh->draw();
}
```
For every object: compute its Model matrix, combine it with the camera's View*Projection to get the final MVP, push both plus the object's color as uniforms, then call `draw()` (Part 4) which actually issues the GPU draw command. This loop is why lights are set once outside the loop but `uModel`/`uMVP`/`uAlbedo` are set inside it — those three are different for every object.

**The normal matrix** deserves a callout since it's the one non-obvious line:
```cpp
const glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(model));
```
You might expect normals to transform with the same Model matrix as positions, but that breaks under non-uniform scale (e.g. stretching an object only along X) — the normal would stop being perpendicular to the surface. The mathematically correct fix is to transform normals by the *inverse transpose* of the model matrix instead. `glm::mat3(model)` first drops the translation part (normals are directions, not points — translating them makes no sense), then `inverseTranspose` applies the fix. You don't need to derive this yourself; it's a standard, memorizable rule: "positions use the model matrix, normals use the inverse-transpose of the model matrix."

---

## Part 8 — `include/core/Shader.h` / `src/rendering/Shader.cpp`

### Loading and compiling

```cpp
Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    const std::string vertexSource = readTextFile(vertexPath);
    const std::string fragmentSource = readTextFile(fragmentPath);
    vertexShader = compileStage(GL_VERTEX_SHADER, vertexSource, vertexPath);
    fragmentShader = compileStage(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);
    m_program = linkProgram(vertexShader, fragmentShader);
    ...
}
```
Unlike your C++ code, GLSL shader source is plain text read from disk and compiled *at runtime*, by the graphics driver, every time the program starts. `readTextFile` just slurps the `.vert`/`.frag` file into a string.

```cpp
GLuint Shader::compileStage(GLenum type, const std::string& source, const std::string& debugName) {
    const GLuint shader = glCreateShader(type);
    const char* sourcePtr = source.c_str();
    glShaderSource(shader, 1, &sourcePtr, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        // fetch and throw the compiler's error log
    }
    return shader;
}
```
Creates an empty shader object of the right type (vertex or fragment), feeds it the source text, compiles it, then checks a status flag — GLSL compile errors (typos, type mismatches) don't crash anything, they just set `success = GL_FALSE` and populate an error log you have to explicitly ask for and print yourself, which is exactly what the `throw` does.

```cpp
GLuint Shader::linkProgram(GLuint vertexShader, GLuint fragmentShader) {
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    ...
}
```
A compiled vertex shader and fragment shader aren't usable on their own — they get "linked" together into one `program` object, similar in spirit to how object files get linked into an executable. `bind()` (`glUseProgram`) later selects this linked program as the one to run for draw calls.

### The uniform setters

```cpp
void Shader::setVec3(const std::string& uniformName, const glm::vec3& value) const {
    const GLint location = glGetUniformLocation(m_program, uniformName.c_str());
    if (location >= 0) {
        glUniform3fv(location, 1, &value[0]);
    }
}
```
Every uniform in a shader has a numeric "location" the driver assigns; `glGetUniformLocation` looks it up by name (a modest CPU cost — this is why the `if (location >= 0)` guard exists too, so setting a uniform that a shader doesn't actually declare/use just silently no-ops instead of crashing). `glUniform3fv` then uploads 3 floats to that location. `setMat4`/`setMat3`/`setFloat`/`setInt` all follow the identical pattern for their respective GLSL types.

---

## Part 9 — `shaders/lit.vert` and `shaders/lit.frag` — the actual graphics, line by line

### `lit.vert`

```glsl
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
```
These three lines are the receiving end of `Mesh::upload()`'s `glVertexAttribPointer(0, ...)`, `(1, ...)`, `(2, ...)` calls from Part 4 — the `location = N` numbers must match the attribute slot numbers there. This is the per-vertex data.

```glsl
uniform mat4 uModel;
uniform mat4 uMVP;
uniform mat3 uNormalMatrix;
```
The three values `Scene::render()` uploads per object (Part 7).

```glsl
out vec3 vWorldPosition;
out vec3 vWorldNormal;
out vec2 vUV;
```
Varyings — computed once per vertex here, then automatically interpolated across the triangle for every pixel the fragment shader processes.

```glsl
void main() {
    vWorldPosition = vec3(uModel * vec4(inPosition, 1.0));
```
Converts this vertex's object-space position into world-space position, for lighting math. Note the `vec4(inPosition, 1.0)` — matrix math requires 4D vectors (homogeneous coordinates) to represent both rotation/scale *and* translation in a single matrix multiply; the `1.0` in the 4th slot is what makes translation apply (if it were `0.0`, this would transform it as a *direction* instead of a *point* — that trick is exactly why normals get multiplied by a 3x3 matrix instead, no 4th component needed).

```glsl
    vWorldNormal = normalize(uNormalMatrix * inNormal);
```
Applies the normal matrix from Part 7, then `normalize` forces the result back to unit length (scaling can otherwise stretch it).

```glsl
    vUV = inUV;
    gl_Position = uMVP * vec4(inPosition, 1.0);
}
```
Passes UVs through untouched (unused for now). `gl_Position` is a special built-in output — it's the one thing every vertex shader *must* write, and it's what the GPU actually uses to figure out screen position and depth. This is the Model-View-Projection multiply from Part 0.

### `lit.frag`

```glsl
struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float radius;
};

uniform vec3 uAlbedo;
uniform int uLightCount;
uniform PointLight uLights[MAX_LIGHTS];
```
GLSL supports structs and fixed-size arrays of them. `MAX_LIGHTS` (8) is the array's compile-time capacity; `uLightCount` is how many of those 8 slots are actually populated this frame — the loop below only touches that many.

```glsl
vec3 evaluatePointLight(PointLight light, vec3 worldPosition, vec3 normal, vec3 albedo) {
    vec3 toLight = light.position - worldPosition;
    float distance = length(toLight);
    vec3 lightDir = toLight / max(distance, 0.0001);
```
Vector from this pixel's surface point to the light, its length, and the normalized direction. `max(distance, 0.0001)` guards against dividing by zero if a light sits exactly on a surface.

```glsl
    float attenuation = clamp(1.0 - (distance / light.radius), 0.0, 1.0);
    attenuation *= attenuation;
```
A simple, cheap falloff: full brightness at distance 0, fading linearly to 0 at `light.radius`, then squared to make the falloff feel less abrupt near the edge. (This is a placeholder, not physically-based inverse-square falloff — fine for this stage, worth a footnote in the thesis if you keep it.)

```glsl
    float ndotl = max(dot(normal, lightDir), 0.0);
    return albedo * light.color * light.intensity * ndotl * attenuation;
}
```
`dot(normal, lightDir)` is the core of all diffuse lighting — the dot product of two unit vectors gives the cosine of the angle between them: 1.0 when the light hits the surface head-on, 0.0 when it's at a glancing angle, negative when the light is behind the surface (which `max(..., 0.0)` clamps away). This is called **Lambertian shading**, the simplest physically-motivated lighting model. Multiplying it all together (surface color × light color × light strength × angle factor × distance falloff) gives the light's contribution; `main()` below sums this over every active light.

```glsl
void main() {
    vec3 normal = normalize(vWorldNormal);
    vec3 result = uAlbedo * 0.05;
    for (int i = 0; i < uLightCount; ++i) {
        result += evaluatePointLight(uLights[i], vWorldPosition, normal, uAlbedo);
    }
    outColor = vec4(result, 1.0);
}
```
Re-normalizes the interpolated normal (interpolating two unit vectors across a triangle can produce a slightly-non-unit result). `uAlbedo * 0.05` is a flat, fake "ambient" term so surfaces facing away from every light aren't pure black — real engines compute ambient more carefully (or skip it in favor of proper indirect lighting), this is a minimal stand-in. Then it sums every light's contribution and writes the final RGBA color to `outColor`, which is the fragment shader's required output — this is the color that pixel actually becomes on screen.

---

## Part 10 — `include/core/Framebuffer.h` / `Framebuffer.cpp` (concept only — not used yet)

So far, everything renders straight to the screen. Both comparison pipelines need an intermediate step: render into an off-screen texture first, then read that texture in a later pass. A **Framebuffer Object (FBO)** is OpenGL's mechanism for "draw into this texture instead of the screen." This class wraps that: `addColorAttachment` creates a texture and attaches it as a render target, `addDepthAttachment` does the same for depth, `bind()`/`unbind()` switch rendering between "into this FBO" and "onto the screen," and `finalize()` tells OpenGL which of the attached textures to actually write to (`glDrawBuffers`) and checks everything is configured correctly.

Deferred Shading will use this to create a **G-buffer**: several textures (albedo, normal, depth) filled in one pass, then read back in a second pass that does the actual lighting math per-pixel instead of per-object. Visibility Buffer will use it similarly, but store triangle IDs instead of material data. Nothing calls this class yet — it exists now so both pipelines can share it instead of duplicating FBO setup code.

---

## Part 11 — one frame, start to finish

1. `main.cpp`'s loop computes `deltaTime`, reads input, updates the camera.
2. `viewProjection = camera.projectionMatrix() * camera.viewMatrix()` — same for the whole frame.
3. Screen is cleared to a background color, depth buffer reset.
4. `scene.render(shader, viewProjection)` runs:
   - Shader bound, all 2 lights uploaded as uniforms once.
   - For each of the 7 objects (plane, 5 cubes, sphere): compute Model/MVP/normal matrix, upload them + the object's color, call `mesh->draw()`.
   - `draw()` issues `glDrawElements`, which triggers:
     - `lit.vert` runs once per vertex of that mesh, computing world position/normal and clip-space `gl_Position`.
     - The GPU rasterizes: figures out which pixels each triangle covers, interpolating `vWorldPosition`/`vWorldNormal`/`vUV` across each pixel.
     - `lit.frag` runs once per covered pixel, summing Lambertian lighting from both point lights into `outColor`.
5. `glfwSwapBuffers` presents the finished frame.
6. Repeat, ~60+ times a second.

That's the whole pipeline as it exists today — one geometry pass, one shading pass, done together in a single shader. The next step (Deferred Shading) splits step 4 into two separate passes using the `Framebuffer` class from Part 10: a geometry pass that only writes position/normal/color into G-buffer textures, and a second, separate lighting pass that reads those textures back and does the `evaluatePointLight` math you just read — but only once per *pixel*, regardless of how many overlapping objects were drawn there, instead of once per pixel *per object* like the current forward approach does.
