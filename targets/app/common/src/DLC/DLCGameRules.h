#pragma once

#include "DLCFile.h"
#include "app/common/src/GameRules/LevelGeneration/LevelGenerationOptions.h"

class DLCGameRules : public DLCFile {
public:
    DLCGameRules(DLCManager::EDLCType type, const std::wstring& path)
        : DLCFile(type, path) {}
};