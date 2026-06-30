#pragma once
#include <cstdint>
#include <string>

struct ParsedAddress {
    std::string host;
    uint16_t    port = 25565;
    bool        hostEmpty = true;
};

ParsedAddress parseAddress(const std::wstring& input);