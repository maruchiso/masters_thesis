#pragma once

#include <glad/glad.h>

// Double-buffered GL_TIME_ELAPSED query around a GPU pass. begin()/end() bracket the pass;
// lastElapsedMilliseconds() returns the PREVIOUS frame's result, not the current one -- by
// the time it's read, a full frame has already passed, so the GPU has almost certainly
// finished and glGetQueryObjectui64v does not stall the CPU waiting for it. Reading the
// *current* frame's result immediately after end() would force exactly that stall, which
// would distort the very timing this class exists to measure.
//
// Note: this measures GPU execution time only. It is unaffected by vsync (which delays
// glfwSwapBuffers, after all rendering commands are already issued) -- so it stays a valid,
// meaningful signal even with vsync enabled, unlike CPU-side frame time.
class GpuTimer {
public:
    GpuTimer();
    ~GpuTimer();

    GpuTimer(const GpuTimer&) = delete;
    GpuTimer& operator=(const GpuTimer&) = delete;

    void begin();
    void end();

    double lastElapsedMilliseconds() const { return m_lastElapsedMs; }

private:
    GLuint m_queries[2];
    int m_writeIndex;
    bool m_hasResult[2];
    double m_lastElapsedMs;
};
