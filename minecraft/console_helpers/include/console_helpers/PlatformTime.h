#pragma once

#include <chrono>
#include <cstdint>

// Portable replacements for Win32 time APIs.
// GetTickCount()              → PlatformTime::GetTickCount()
// QueryPerformanceCounter()   → PlatformTime::QueryPerformanceCounter()
// QueryPerformanceFrequency() → PlatformTime::QueryPerformanceFrequency()

namespace PlatformTime {

// Returns milliseconds since an unspecified epoch (like Win32 GetTickCount).
inline std::uint32_t GetTickCount() {
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<std::uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

// High-resolution counter value (like Win32 QueryPerformanceCounter).
// Returns a count in ticks of steady_clock.
inline std::int64_t QueryPerformanceCounter() {
    return std::chrono::steady_clock::now().time_since_epoch().count();
}

// Ticks per second for the high-resolution counter.
inline std::int64_t QueryPerformanceFrequency() {
    return static_cast<std::int64_t>(std::chrono::steady_clock::period::den)
         / static_cast<std::int64_t>(std::chrono::steady_clock::period::num);
}

// Elapsed seconds between two counter values.
inline double ElapsedSeconds(std::int64_t start, std::int64_t end) {
    return static_cast<double>(end - start)
         / static_cast<double>(QueryPerformanceFrequency());
}

}  // namespace PlatformTime
