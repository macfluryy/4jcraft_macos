#include "McRegionLevelStorageSource.h"

#include <assert.h>

#include <algorithm>
#include <memory>

#include "LevelData.h"
#include "LevelSummary.h"
#include "McRegionLevelStorage.h"
#include "java/File.h"
#include "java/InputOutputStream/FileInputStream.h"
#include "java/InputOutputStream/FileOutputStream.h"
#include "java/JavaMath.h"
#include "app/mac/MacGame.h"
#include "nbt/StringTag.h"
#include "minecraft/SharedConstants.h"
#include "minecraft/util/ProgressListener.h"
#include "minecraft/world/level/GameRules.h"
#include "minecraft/world/level/LevelSettings.h"
#include "minecraft/world/level/chunk/storage/RegionFileCache.h"
#include "minecraft/world/level/storage/DirectoryLevelStorageSource.h"
#include "nbt/CompoundTag.h"
#include "nbt/NbtIo.h"

McRegionLevelStorageSource::McRegionLevelStorageSource(File dir)
    : DirectoryLevelStorageSource(dir) {}

std::wstring McRegionLevelStorageSource::getName() {
    return L"Scaevolus' McRegion";
}
std::vector<LevelSummary*>* McRegionLevelStorageSource::getLevelList() {
    std::vector<LevelSummary*>* levels = new std::vector<LevelSummary*>();

    app.DebugPrintf("[getLevelList] baseDir=%ls exists=%d\n",
                    baseDir.getPath().c_str(), (int)baseDir.exists());

    if (!baseDir.exists()) {
        baseDir.mkdirs();
        return levels;
    }

    std::vector<File*>* entries = baseDir.listFiles();
    if (entries == nullptr) {
        app.DebugPrintf("[getLevelList] listFiles returned null\n");
        return levels;
    }

    app.DebugPrintf("[getLevelList] %zu entries in saves dir\n",
                    entries->size());

    for (auto it = entries->begin(); it != entries->end(); ++it) {
        File* entry = *it;
        if (entry == nullptr) continue;

        if (!entry->isDirectory()) {
            app.DebugPrintf("[getLevelList] skip non-dir: %ls\n",
                            entry->getName().c_str());
            delete entry;
            continue;
        }

        File levelDat(*entry, L"level.dat");
        if (!levelDat.exists()) {
            app.DebugPrintf("[getLevelList] skip dir without level.dat: %ls\n",
                            entry->getName().c_str());
            delete entry;
            continue;
        }

        app.DebugPrintf("[getLevelList] FOUND world: %ls\n",
                        entry->getName().c_str());

        std::wstring levelId = entry->getName();
        std::wstring levelName = levelId;
        int64_t lastPlayed = entry->lastModified();
        int64_t sizeOnDisk = 0;
        GameType* gameMode = GameType::SURVIVAL;
        bool hardcore = false;
        bool hasCheats = false;
        CompoundTag* root = nullptr;
        try {
            FileInputStream fis(levelDat);
            root = NbtIo::readCompressed(&fis);
            fis.close();
        } catch (...) {
            root = nullptr;
        }

        if (root != nullptr) {
            CompoundTag* data = root->getCompound(L"Data");
            if (data != nullptr) {
                std::wstring storedName = data->getString(L"LevelName");
                if (!storedName.empty()) levelName = storedName;

                int64_t storedLastPlayed = data->getLong(L"LastPlayed");
                if (storedLastPlayed > 0) lastPlayed = storedLastPlayed;

                sizeOnDisk = data->getLong(L"SizeOnDisk");

                GameType* gt = GameType::byId(data->getInt(L"GameType"));
                if (gt != nullptr) gameMode = gt;

                hardcore = data->getBoolean(L"hardcore");
                if (data->contains(L"allowCommands")) {
                    hasCheats = data->getBoolean(L"allowCommands");
                } else {
                    hasCheats = (gameMode == GameType::CREATIVE);
                }
            }
            delete root;
        }

        if (sizeOnDisk <= 0) sizeOnDisk = levelDat.length();

        levels->push_back(new LevelSummary(levelId, levelName, lastPlayed,
                                           sizeOnDisk, gameMode,
                                            false,
                                           hardcore, hasCheats));

        delete entry;
    }

    delete entries;

    std::sort(levels->begin(), levels->end(),
              [](LevelSummary* a, LevelSummary* b) {
                  return a->compareTo(b) < 0;
              });

    return levels;
}

void McRegionLevelStorageSource::clearAll() {
    RegionFileCache::clear();
}
void McRegionLevelStorageSource::renameLevel(const std::wstring& levelId,
                                             const std::wstring& newLevelName) {
    File worldDir(baseDir, levelId);
    if (!worldDir.exists() || !worldDir.isDirectory()) {
        app.DebugPrintf("[renameLevel] world folder not found: %ls\n",
                        levelId.c_str());
        return;
    }
    File levelDat(worldDir, L"level.dat");
    if (levelDat.exists()) {
        CompoundTag* root = nullptr;
        try {
            FileInputStream fis(levelDat);
            root = NbtIo::readCompressed(&fis);
            fis.close();
        } catch (...) {
            root = nullptr;
        }

        if (root != nullptr) {
            CompoundTag* data = root->getCompound(L"Data");
            if (data != nullptr) {
                data->putString(L"LevelName", newLevelName);
                try {
                    FileOutputStream fos(levelDat);
                    NbtIo::writeCompressed(root, &fos);
                    fos.close();
                } catch (...) {
                    app.DebugPrintf(
                        "[renameLevel] failed to write level.dat\n");
                }
            }
            delete root;
        }
    }
    std::wstring sanitised = newLevelName;
    for (int i = 0; i < SharedConstants::ILLEGAL_FILE_CHARACTERS_LENGTH; ++i) {
        wchar_t bad = SharedConstants::ILLEGAL_FILE_CHARACTERS[i];
        std::replace(sanitised.begin(), sanitised.end(), bad, L'_');
    }
    while (!sanitised.empty() && (sanitised.front() == L' ')) sanitised.erase(0, 1);
    while (!sanitised.empty() && (sanitised.back() == L' ')) sanitised.pop_back();
    if (sanitised.empty()) sanitised = levelId;

    if (sanitised != levelId) {
        File target(baseDir, sanitised);
        if (!target.exists()) {
            if (!worldDir.renameTo(target)) {
                app.DebugPrintf(
                    "[renameLevel] rename folder failed: %ls -> %ls\n",
                    levelId.c_str(), sanitised.c_str());
            }
        }
    }
}
void McRegionLevelStorageSource::deleteLevel(const std::wstring& levelId) {
    RegionFileCache::clear();

    File worldDir(baseDir, levelId);
    if (!worldDir.exists()) return;

    std::vector<File*>* children = worldDir.listFiles();
    if (children != nullptr) {
        deleteRecursive(children);
        delete children;
    }
    worldDir._delete();
}

std::shared_ptr<LevelStorage> McRegionLevelStorageSource::selectLevel(
    ConsoleSaveFile* saveFile, const std::wstring& levelId,
    bool createPlayerDir) {
    
    
    return std::shared_ptr<LevelStorage>(
        new McRegionLevelStorage(saveFile, baseDir, levelId, createPlayerDir));
}

bool McRegionLevelStorageSource::isConvertible(ConsoleSaveFile* saveFile,
                                               const std::wstring& levelId) {
    LevelData* levelData = getDataTagFor(saveFile, levelId);
    if (levelData == nullptr || levelData->getVersion() != 0) {
        delete levelData;
        return false;
    }
    delete levelData;

    return true;
}

bool McRegionLevelStorageSource::requiresConversion(
    ConsoleSaveFile* saveFile, const std::wstring& levelId) {
    LevelData* levelData = getDataTagFor(saveFile, levelId);
    if (levelData == nullptr || levelData->getVersion() != 0) {
        delete levelData;
        return false;
    }
    delete levelData;

    return true;
}

bool McRegionLevelStorageSource::convertLevel(ConsoleSaveFile* saveFile,
                                              const std::wstring& levelId,
                                              ProgressListener* progress) {
    assert(false);
    return true;
}

void McRegionLevelStorageSource::convertRegions(
    File& baseFolder, std::vector<ChunkFile*>* chunkFiles, int currentCount,
    int totalCount, ProgressListener* progress) {
    assert(false);
}

void McRegionLevelStorageSource::eraseFolders(std::vector<File*>* folders,
                                              int currentCount, int totalCount,
                                              ProgressListener* progress) {
    File* folder;
    auto itEnd = folders->end();
    for (auto it = folders->begin(); it != itEnd; it++) {
        folder = *it;  

        std::vector<File*>* files = folder->listFiles();
        deleteRecursive(files);
        folder->_delete();

        currentCount++;
        int percent =
            (int)Math::round(100.0 * (double)currentCount / (double)totalCount);
        progress->progressStagePercentage(percent);
    }
}
