#include "I18n.h"

#include "Language.h"

Language* I18n::lang = Language::getInstance();
// 4jcraft const & into va_start is ub
std::wstring I18n::get(std::wstring id, ...) {
    va_list va;
    va_start(va, id);
    return I18n::get(id, va);
}

std::wstring I18n::get(const std::wstring& id, va_list args) {
    return lang->getElement(id, args);
}
