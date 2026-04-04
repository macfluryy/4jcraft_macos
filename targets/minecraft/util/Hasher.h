#pragma once

#include <string>

class Hasher {
private:
    std::wstring salt;

public:
    Hasher(std::wstring& salt);
    std::wstring getHash(std::wstring& name);
};