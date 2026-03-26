#include "../Platform/stdafx.h"
#include "Language.h"
#include "I18n.h"

Language* I18n::lang = Language::getInstance();
// 4jcraft const & into va_start is ub
std::wstring I18n::get(std::wstring id, ...) {
#if 0  // 4J - vita doesn't like having a reference type as the last
                   // parameter passed to va_start - we shouldn't need this
                   // method anyway
    return L"";
#else
    va_list va;
    va_start(va, id);
    return I18n::get(id, va);
#endif
}

std::wstring I18n::get(const std::wstring& id, va_list args) {
    return lang->getElement(id, args);
}
