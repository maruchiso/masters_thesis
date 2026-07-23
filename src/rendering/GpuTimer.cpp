#include "core/GpuTimer.h"

GpuTimer::GpuTimer()
    : m_queries{0, 0}, m_writeIndex(0), m_hasResult{false, false}, m_lastElapsedMs(0.0) {
    glGenQueries(2, m_queries);
}

GpuTimer::~GpuTimer() {
    glDeleteQueries(2, m_queries);
}

void GpuTimer::begin() {
    glBeginQuery(GL_TIME_ELAPSED, m_queries[m_writeIndex]);
}

void GpuTimer::end() {
    glEndQuery(GL_TIME_ELAPSED);

    const int readIndex = 1 - m_writeIndex;
    if (m_hasResult[readIndex]) {
        GLuint64 elapsedNanoseconds = 0;
        glGetQueryObjectui64v(m_queries[readIndex], GL_QUERY_RESULT, &elapsedNanoseconds);
        m_lastElapsedMs = static_cast<double>(elapsedNanoseconds) / 1000000.0;
    }

    m_hasResult[m_writeIndex] = true;
    m_writeIndex = readIndex;
}
