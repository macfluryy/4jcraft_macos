#include "GuiMessage.h"
#include "minecraft/client/GuiMessage.h"

GuiMessage::GuiMessage(const std::wstring& string) {
    this->string = string;
    ticks = 0;
}