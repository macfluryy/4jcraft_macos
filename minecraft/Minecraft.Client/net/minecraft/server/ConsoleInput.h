#pragma once
#include <string>

#include "ConsoleInputSource.h"

class ConsoleInputSource;

class ConsoleInput {
public:
    std::wstring msg;
    ConsoleInputSource* source;

    ConsoleInput(const std::wstring& msg, ConsoleInputSource* source);
};