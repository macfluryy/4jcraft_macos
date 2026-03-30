#pragma once

#include "../Util/Exceptions.h"

class LevelConflictException : public RuntimeException {
private:
    static const int32_t serialVersionUID = 1L;

public:
    LevelConflictException(const std::wstring& msg);
};