#include "ConsoleInput.h"

class ConsoleInputSource;

ConsoleInput::ConsoleInput(const std::wstring& msg,
                           ConsoleInputSource* source) {
    this->msg = msg;
    this->source = source;
}