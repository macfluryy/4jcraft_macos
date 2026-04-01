
#include <chrono>
#include <string>

#include "console_helpers/PerformanceTimer.h"

PerformanceTimer::PerformanceTimer() { Reset(); }

void PerformanceTimer::Reset() {
    m_startTime = std::chrono::steady_clock::now();
}

void PerformanceTimer::PrintElapsedTime(const std::wstring& description) {
    const std::chrono::duration<float> elapsedTime =
        std::chrono::steady_clock::now() - m_startTime;

    fprintf(stderr, "TIMER: %ls: Elapsed time %f\n", description.c_str(),
            elapsedTime.count());
}
