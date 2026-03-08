#pragma once
#include "ConsoleInputSource.h"


class ConsoleInput
{
public:
	std::wstring msg;
	ConsoleInputSource *source;

	ConsoleInput(const std::wstring& msg, ConsoleInputSource *source);
};