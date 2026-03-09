#pragma once

#include "DLCFile.h"
#include "../GameRules/LevelGenerationOptions.h"

class DLCGameRules : public DLCFile
{
public:
	DLCGameRules(DLCManager::EDLCType type, const std::wstring &path) : DLCFile(type,path) {}
};