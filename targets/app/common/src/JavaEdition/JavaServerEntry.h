#pragma once
#include <cstdint>
#include <string>

struct JavaServerEntry {
    std::wstring name;
    std::string  host;
    uint16_t     port = 25565;
};