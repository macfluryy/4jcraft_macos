#pragma once

#include <stdarg.h>
#include <string>

#include "Language.h"

class Language;

class I18n {
private:
    static Language* lang;

public:
    static std::wstring get(std::wstring id, ...);
    static std::wstring get(const std::wstring& id, va_list args);
};
