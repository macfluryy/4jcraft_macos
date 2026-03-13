#pragma once

#include "LevelStorageSource.h"

class MemoryLevelStorageSource : public LevelStorageSource {
public:
    MemoryLevelStorageSource();
    std::wstring getName();
    std::shared_ptr<LevelStorage> selectLevel(const std::wstring& levelId,
                                              bool createPlayerDir);
    std::vector<LevelSummary*>* getLevelList();
    void clearAll();
    LevelData* getDataTagFor(const std::wstring& levelId);
    bool isNewLevelIdAcceptable(const std::wstring& levelId);
    void deleteLevel(const std::wstring& levelId);
    void renameLevel(const std::wstring& levelId,
                     const std::wstring& newLevelName);
    bool isConvertible(const std::wstring& levelId);
    bool requiresConversion(const std::wstring& levelId);
    bool convertLevel(const std::wstring& levelId, ProgressListener* progress);
};