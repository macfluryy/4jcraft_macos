#pragma once

#include "LevelStorageSource.h"
#include "../../../../../ConsoleJavaLibs/File.h"

class ProgressListener;
class LevelData;
class ConsoleSaveFile;

class DirectoryLevelStorageSource : public LevelStorageSource {
protected:
    const File baseDir;

public:
    DirectoryLevelStorageSource(const File dir);
    virtual std::wstring getName();
    virtual std::vector<LevelSummary*>* getLevelList();
    virtual void clearAll();
    virtual LevelData* getDataTagFor(ConsoleSaveFile* saveFile,
                                     const std::wstring& levelId);
    virtual void renameLevel(const std::wstring& levelId,
                             const std::wstring& newLevelName);
    virtual bool isNewLevelIdAcceptable(const std::wstring& levelId);
    virtual void deleteLevel(const std::wstring& levelId);

protected:
    static void deleteRecursive(std::vector<File*>* files);

public:
    virtual std::shared_ptr<LevelStorage> selectLevel(
        ConsoleSaveFile* saveFile, const std::wstring& levelId,
        bool createPlayerDir);
    virtual bool isConvertible(ConsoleSaveFile* saveFile,
                               const std::wstring& levelId);
    virtual bool requiresConversion(ConsoleSaveFile* saveFile,
                                    const std::wstring& levelId);
    virtual bool convertLevel(ConsoleSaveFile* saveFile,
                              const std::wstring& levelId,
                              ProgressListener* progress);
};