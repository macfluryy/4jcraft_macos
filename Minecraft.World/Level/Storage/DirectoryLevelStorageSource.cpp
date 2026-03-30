#include "../../Platform/stdafx.h"
#include "../../IO/Files/File.h"
#include "../LevelData.h"
#include "LevelSummary.h"
#include "../../Headers/com.mojang.nbt.h"
#include "DirectoryLevelStorage.h"
#include "DirectoryLevelStorageSource.h"

#include "../../IO/Files/ConsoleSaveFileIO.h"
#include "../../IO/Files/ConsoleSaveFileOriginal.h"

class LevelStorage;

DirectoryLevelStorageSource::DirectoryLevelStorageSource(const File dir)
    : baseDir(dir) {
    // if (!dir.exists()) dir.mkdirs(); // 4J Removed
    // this->baseDir = dir;
}

std::wstring DirectoryLevelStorageSource::getName() { return L"Old Format"; }

std::vector<LevelSummary*>* DirectoryLevelStorageSource::getLevelList() {
    // 4J Stu - We don't use directory list with the Xbox save locations
    std::vector<LevelSummary*>* levels = new std::vector<LevelSummary*>;
    return levels;
}

void DirectoryLevelStorageSource::clearAll() {}

LevelData* DirectoryLevelStorageSource::getDataTagFor(
    ConsoleSaveFile* saveFile, const std::wstring& levelId) {
    // File dataFile(dir, L"level.dat");
    ConsoleSavePath dataFile = ConsoleSavePath(std::wstring(L"level.dat"));
    if (saveFile->doesFileExist(dataFile)) {
        ConsoleSaveFileInputStream fis =
            ConsoleSaveFileInputStream(saveFile, dataFile);
        CompoundTag* root = NbtIo::readCompressed(&fis);
        CompoundTag* tag = root->getCompound(L"Data");
        LevelData* ret = new LevelData(tag);
        delete root;
        return ret;
    }

    return nullptr;
}

void DirectoryLevelStorageSource::renameLevel(
    const std::wstring& levelId, const std::wstring& newLevelName) {
    ConsoleSaveFileOriginal tempSave(levelId);

    // File dataFile = File(dir, L"level.dat");
    ConsoleSavePath dataFile = ConsoleSavePath(std::wstring(L"level.dat"));
    if (tempSave.doesFileExist(dataFile)) {
        ConsoleSaveFileInputStream fis =
            ConsoleSaveFileInputStream(&tempSave, dataFile);
        CompoundTag* root = NbtIo::readCompressed(&fis);
        CompoundTag* tag = root->getCompound(L"Data");
        tag->putString(L"LevelName", newLevelName);

        ConsoleSaveFileOutputStream fos =
            ConsoleSaveFileOutputStream(&tempSave, dataFile);
        NbtIo::writeCompressed(root, &fos);
    }
}

bool DirectoryLevelStorageSource::isNewLevelIdAcceptable(
    const std::wstring& levelId) {
    // 4J Jev, removed try/catch.

    File levelFolder = File(baseDir, levelId);
    if (levelFolder.exists()) {
        return false;
    }

    levelFolder.mkdir();

    return true;
}

void DirectoryLevelStorageSource::deleteLevel(const std::wstring& levelId) {
    File dir = File(baseDir, levelId);
    if (!dir.exists()) return;

    deleteRecursive(dir.listFiles());
    dir._delete();
}

void DirectoryLevelStorageSource::deleteRecursive(std::vector<File*>* files) {
    AUTO_VAR(itEnd, files->end());
    for (AUTO_VAR(it, files->begin()); it != itEnd; it++) {
        File* file = *it;
        if (file->isDirectory()) {
            deleteRecursive(file->listFiles());
        }
        file->_delete();
    }
}

std::shared_ptr<LevelStorage> DirectoryLevelStorageSource::selectLevel(
    ConsoleSaveFile* saveFile, const std::wstring& levelId,
    bool createPlayerDir) {
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
