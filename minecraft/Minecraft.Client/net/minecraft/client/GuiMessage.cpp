#include "GuiMessage.h"
#include "Minecraft.Client/net/minecraft/client/GuiMessage.h"

GuiMessage::GuiMessage(const std::wstring& string) {
    this->string = string;
    ticks = 0;
}