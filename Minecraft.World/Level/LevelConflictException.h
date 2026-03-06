#pragma once


#include "../Util/Exceptions.h"

class LevelConflictException : public RuntimeException
{
private:
	static const __int32 serialVersionUID = 1L;
    
public:
	LevelConflictException(const std::wstring& msg);
};