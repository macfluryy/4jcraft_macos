#pragma once

#include <format>
#include <string>
#include <vector>

#include "app/common/App_enums.h"


class HtmlString {
public:
    std::wstring text;       
    eMinecraftColour color;  
    bool italics;            
    bool indent;             

    HtmlString(std::wstring text,
               eMinecraftColour color = eMinecraftColour_NOT_SET,
               bool italics = false, bool indent = false);
    std::wstring ToString();

    static std::wstring Compose(std::vector<HtmlString>* strings);
};