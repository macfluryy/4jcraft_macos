#include "LevelConflictException.h"

LevelConflictException::LevelConflictException(const std::wstring& msg)
    : RuntimeException(msg) {}