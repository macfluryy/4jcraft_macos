#pragma once
using namespace std;

class GuiMessage
{
public:
	std::wstring string;
	int ticks;
	GuiMessage(const std::wstring& string);
};