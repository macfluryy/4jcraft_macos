#pragma once
#include <string>
#if defined(__linux__)
#include <limits.h>
#include <unistd.h>
#endif

namespace PathHelper {
inline std::wstring GetExecutableDirW() {
#if defined(__linux__)
    char buffer[4096];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        std::string path(buffer);
        size_t lastSlash = path.find_last_of('/');
        if (lastSlash != std::string::npos)
            return std::wstring(path.begin(), path.begin() + lastSlash);
    }
#endif
    return L".";
}

inline std::string GetExecutableDirA() {
#if defined(__linux__)
    char buffer[4096];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        std::string path(buffer);
        size_t lastSlash = path.find_last_of('/');
        if (lastSlash != std::string::npos) return path.substr(0, lastSlash);
    }
#endif
    return ".";
}
}  // namespace PathHelper