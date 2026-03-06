#pragma once
#include "ConsoleInputSource.h"
using namespace std;

class ConsoleInput
{
public:
	std::wstring msg;
	ConsoleInputSource *source;

	ConsoleInput(const std::wstring& msg, ConsoleInputSource *source);
};