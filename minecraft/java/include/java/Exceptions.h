#pragma once

#include <string>
#include <exception>

class EOFException : public std::exception {};

class IllegalArgumentException : public std::exception {
public:
    std::wstring information;

    IllegalArgumentException(const std::wstring& information);
};

class IOException : public std::exception {
public:
    std::wstring information;

    IOException(const std::wstring& information);
};

class RuntimeException : public std::exception {
public:
    RuntimeException(const std::wstring& information);
};