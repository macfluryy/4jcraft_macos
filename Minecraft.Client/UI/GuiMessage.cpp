#include "../Build/stdafx.h"
#include "GuiMessage.h"

GuiMessage::GuiMessage(const std::wstring& string)
{
	this->string = string;
	ticks = 0;
}