#include "../Platform/stdafx.h"
#include "ConsoleInput.h"

ConsoleInput::ConsoleInput(const std::wstring& msg,
                           ConsoleInputSource* source) {
    this->msg = msg;
    this->source = source;
}