#include "../Platform/stdafx.h"
#include "SmallButton.h"

SmallButton::SmallButton(int id, int x, int y, const std::wstring& msg)
    : Button(id, x, y, 150, 20, msg) {
    this->option = NULL;
}

SmallButton::SmallButton(int id, int x, int y, int width, int height,
                         const std::wstring& msg)
    : Button(id, x, y, width, height, msg) {
    this->option = NULL;
}

SmallButton::SmallButton(int id, int x, int y, const Options::Option* item,
                         const std::wstring& msg)
    : Button(id, x, y, 150, 20, msg) {
    this->option = item;
}

const Options::Option* SmallButton::getOption() { return option; }