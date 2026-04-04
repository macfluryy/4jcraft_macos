#pragma once

#include <format>
#include <string>
#include <vector>

#include "app/common/App_enums.h"

// 4J: Simple std::string wrapper that includes basic formatting information
class HtmlString {
public:
    std::wstring text;       // Text content of std::string
    eMinecraftColour color;  // Hex color
    bool italics;            // Show text in italics
    bool indent;             // Indent text

    HtmlString(std::wstring text,
               eMinecraftColour color = eMinecraftColour_NOT_SET,
               bool italics = false, bool indent = false);
    std::wstring ToString();

    static std::wstring Compose(std::vector<HtmlString>* strings);
};