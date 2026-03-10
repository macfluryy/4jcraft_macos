#pragma once

class  ConsoleInputSource
{
public:
	virtual ~ConsoleInputSource(){}
	virtual void info(const std::wstring& string) = 0;
    virtual void warn(const std::wstring& string) = 0;
    virtual std::wstring getConsoleName() = 0;
};
