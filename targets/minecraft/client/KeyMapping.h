#pragma once

#include <string>


class KeyMapping {
public:
    std::wstring name;
    int key;
    KeyMapping(const std::wstring& name, int key);
};