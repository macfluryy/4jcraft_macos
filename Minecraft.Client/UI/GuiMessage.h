#pragma once

class GuiMessage {
public:
    std::wstring string;
    int ticks;
    GuiMessage(const std::wstring& string);
};