#pragma once

#include "DirectoryLevelStorageSource.h"
#include "java/FileFilter.h"
#include "java/FilenameFilter.h"

class ProgressListener;
class LevelStorage;

class McRegionLevelStorageSource : public DirectoryLevelStorageSource {
public:
    class ChunkFile;

    McRegionLevelStorageSource(File dir);
    virtual std::wstring getName();
    virtual std::vector<LevelSummary*>* getLevelList();
    virtual void clearAll();
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

private:
    void convertRegions(File& baseFolder, std::vector<ChunkFile*>* chunkFiles,
                        int currentCount, int totalCount,
                        ProgressListener* progress);
    void eraseFolders(std::vector<File*>* folders, int currentCount,
                      int totalCount, ProgressListener* progress);

public:
};
