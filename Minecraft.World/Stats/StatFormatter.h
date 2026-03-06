#pragma once
using namespace std;

class StatFormatter
{
public:
	virtual std::wstring format(int value) = 0;
};
