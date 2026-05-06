#include "DirectoryLevelStorageSource.h"

#include <memory>
#include <vector>

#include "DirectoryLevelStorage.h"
#include "LevelData.h"
#include "java/File.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFile.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFileInputStream.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFileOriginal.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFileOutputStream.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSavePath.h"
#include "nbt/CompoundTag.h"
#include "nbt/NbtIo.h"

DirectoryLevelStorageSource::DirectoryLevelStorageSource(const File dir)
    : baseDir(dir) {}

std::wstring DirectoryLevelStorageSource::getName() {
    return L"Old Format";
}

std::vector<LevelSummary*>* DirectoryLevelStorageSource::getLevelList() {
    return new std::vector<LevelSummary*>();
}

void DirectoryLevelStorageSource::clearAll() {}

LevelData* DirectoryLevelStorageSource::getDataTagFor(
    ConsoleSaveFile* saveFile, const std::wstring& levelId) {
    if (saveFile == nullptr) {
        return nullptr;
    }

    ConsoleSavePath dataFile(std::wstring(L"level.dat"));

    if (!saveFile->doesFileExist(dataFile)) {
        return nullptr;
    }

    try {
        ConsoleSaveFileInputStream fis(saveFile, dataFile);
        CompoundTag* root = NbtIo::readCompressed(&fis);
        if (root == nullptr) {
            return nullptr;
        }

        CompoundTag* tag = root->getCompound(L"Data");
        if (tag == nullptr) {
            delete root;
            return nullptr;
        }

        LevelData* ret = new LevelData(tag);
        delete root;
        return ret;
    } catch (...) {
        return nullptr;
    }
}

void DirectoryLevelStorageSource::renameLevel(
    const std::wstring& levelId, const std::wstring& newLevelName) {
    if (levelId.empty() || newLevelName.empty()) {
        return;
    }

    ConsoleSaveFileOriginal tempSave(levelId);
    ConsoleSavePath dataFile(std::wstring(L"level.dat"));

    if (!tempSave.doesFileExist(dataFile)) {
        return;
    }

    try {
        ConsoleSaveFileInputStream fis(&tempSave, dataFile);
        CompoundTag* root = NbtIo::readCompressed(&fis);
        if (root == nullptr) {
            return;
        }

        CompoundTag* tag = root->getCompound(L"Data");
        if (tag == nullptr) {
            delete root;
            return;
        }

        tag->putString(L"LevelName", newLevelName);

        ConsoleSaveFileOutputStream fos(&tempSave, dataFile);
        NbtIo::writeCompressed(root, &fos);

        delete root;
    } catch (...) {
    }
}

bool DirectoryLevelStorageSource::isNewLevelIdAcceptable(
    const std::wstring& levelId) {
    if (levelId.empty()) {
        return false;
    }

    File levelFolder(baseDir, levelId);

    if (levelFolder.exists()) {
        return false;
    }

    levelFolder.mkdir();
    return true;
}

void DirectoryLevelStorageSource::deleteLevel(const std::wstring& levelId) {
    if (levelId.empty()) {
        return;
    }

    File dir(baseDir, levelId);

    if (!dir.exists()) {
        return;
    }

    std::vector<File*>* files = dir.listFiles();
    if (files != nullptr) {
        deleteRecursive(files);
        delete files;
    }

    dir._delete();
}

void DirectoryLevelStorageSource::deleteRecursive(std::vector<File*>* files) {
    if (files == nullptr) {
        return;
    }

    for (auto it = files->begin(); it != files->end(); ++it) {
        File* file = *it;
        if (file == nullptr) {
            continue;
        }

        if (file->isDirectory()) {
            std::vector<File*>* subFiles = file->listFiles();
            if (subFiles != nullptr) {
                deleteRecursive(subFiles);
                delete subFiles;
            }
        }

        file->_delete();
    }
}

std::shared_ptr<LevelStorage> DirectoryLevelStorageSource::selectLevel(
    ConsoleSaveFile* saveFile, const std::wstring& levelId,
    bool createPlayerDir) {
    if (saveFile == nullptr || levelId.empty()) {
        return nullptr;
    }

    return std::shared_ptr<LevelStorage>(
        new DirectoryLevelStorage(saveFile, baseDir, levelId, createPlayerDir));
}

bool DirectoryLevelStorageSource::isConvertible(ConsoleSaveFile* saveFile,
                                                const std::wstring& levelId) {
    return false;
}

bool DirectoryLevelStorageSource::requiresConversion(
    ConsoleSaveFile* saveFile, const std::wstring& levelId) {
    return false;
}

bool DirectoryLevelStorageSource::convertLevel(ConsoleSaveFile* saveFile,
                                               const std::wstring& levelId,
                                               ProgressListener* progress) {
    return false;
}