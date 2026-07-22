#include "HtmlString.h"

#include <iomanip>
#include <sstream>
#include <vector>

#include "app/mac/MacGame.h"
#include "minecraft/util/FormatCodes.h"
#include "util/StringHelpers.h"

HtmlString::HtmlString(std::wstring text, eMinecraftColour hexColor,
                       bool italics, bool indent) {
    this->text = escapeXML(text);
    this->color = hexColor;
    this->italics = italics;
    this->indent = indent;
}

std::wstring HtmlString::ToString() {
    std::wstringstream ss;

    if (indent) {
        ss << L"&nbsp;&nbsp;";
    }

    if (italics) {
        ss << "<i>";
    }

    eMinecraftColour color =
        this->color == eMinecraftColour_NOT_SET ? eHTMLColor_7 : this->color;

    
    
    static uint32_t s_palette[16];
    static bool s_paletteInit = false;
    if (!s_paletteInit) {
        for (int i = 0; i < 16; ++i) {
            s_palette[i] = static_cast<uint32_t>(app.GetHTMLColor(
                static_cast<eMinecraftColour>(eHTMLColor_0 + i)));
        }
        s_paletteInit = true;
    }

    
    
    ss << formatCodesToHtml(
        text, static_cast<uint32_t>(app.GetHTMLColor(color)), s_palette);

    if (italics) {
        ss << "</i>";
    }

    return ss.str();
}

std::wstring HtmlString::Compose(std::vector<HtmlString>* strings) {
    if (strings == nullptr) return L"";

    std::wstringstream ss;

    for (int i = 0; i < strings->size(); i++) {
        ss << strings->at(i).ToString();

        
        if (i + 1 < strings->size()) {
            ss << L"<br>";
        }
    }

    return ss.str();
}