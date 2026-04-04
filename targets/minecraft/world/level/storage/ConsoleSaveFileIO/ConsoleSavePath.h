#pragma once

#include <string>

class ConsoleSavePath {
private:
    std::wstring path;

public:
    ConsoleSavePath(const std::wstring& newPath) { path = newPath; }

    std::wstring getName() const { return path; }

    std::wstring operator+(std::wstring& b) { return path + b; }
};