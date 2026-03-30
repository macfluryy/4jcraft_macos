#include "../Header Files/stdafx.h"
#include "PerformanceTimer.h"

PerformanceTimer::PerformanceTimer() { Reset(); }

void PerformanceTimer::Reset() {
    m_startTime = std::chrono::steady_clock::now();
}

void PerformanceTimer::PrintElapsedTime(const std::wstring& description) {
    const std::chrono::duration<float> elapsedTime =
        std::chrono::steady_clock::now() - m_startTime;

    app.DebugPrintf("TIMER: %ls: Elapsed time %f\n", description.c_str(),
                    elapsedTime.count());
}
