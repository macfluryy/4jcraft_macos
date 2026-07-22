#include "RegionFileCache.h"

#include <utility>

#include "util/StringHelpers.h"
#include "java/File.h"
#include "minecraft/world/level/chunk/storage/RegionFile.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFile.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/FileHeader.h"

class DataInputStream;
class DataOutputStream;

RegionFileCache RegionFileCache::s_defaultCache;

bool RegionFileCache::useSplitSaves(ESavePlatform platform) {
    switch (platform) {
        case SAVE_FILE_PLATFORM_XBONE:
        case SAVE_FILE_PLATFORM_PS4:
            return true;
        default:
            return false;
    };
}

RegionFile* RegionFileCache::_getRegionFile(
    ConsoleSaveFile* saveFile, const std::wstring& prefix, int chunkX,
    int chunkZ)  
{
    
    
    

    

    
    
    File file;
    if (useSplitSaves(saveFile->getSavePlatform())) {
        file = File(prefix + std::wstring(L"r.") + toWString(chunkX >> 4) +
                    L"." + toWString(chunkZ >> 4) + L".mcr");
    } else {
        file = File(prefix + std::wstring(L"r.") + toWString(chunkX >> 5) +
                    L"." + toWString(chunkZ >> 5) + L".mcr");
    }

    RegionFile* ref = nullptr;
    auto it = cache.find(file);
    if (it != cache.end()) ref = it->second;

    
    if (ref != nullptr) {
        return ref;
    }

    
    





    if (cache.size() >= MAX_CACHE_SIZE) {
        _clear();
    }

    RegionFile* reg = new RegionFile(saveFile, &file);
    cache[file] = reg;  
    return reg;
}

void RegionFileCache::_clear()  
{
    auto itEnd = cache.end();
    for (auto it = cache.begin(); it != itEnd; it++) {
        
        
        RegionFile* regionFile = it->second;
        if (regionFile != nullptr) {
            regionFile->close();
        }
        delete regionFile;
        
        
        
    }
    cache.clear();
}

int RegionFileCache::_getSizeDelta(ConsoleSaveFile* saveFile,
                                   const std::wstring& prefix, int chunkX,
                                   int chunkZ) {
    RegionFile* r = _getRegionFile(saveFile, prefix, chunkX, chunkZ);
    return r->getSizeDelta();
}

DataInputStream* RegionFileCache::_getChunkDataInputStream(
    ConsoleSaveFile* saveFile, const std::wstring& prefix, int chunkX,
    int chunkZ) {
    RegionFile* r = _getRegionFile(saveFile, prefix, chunkX, chunkZ);
    if (useSplitSaves(saveFile->getSavePlatform())) {
        return r->getChunkDataInputStream(chunkX & 15, chunkZ & 15);
    } else {
        return r->getChunkDataInputStream(chunkX & 31, chunkZ & 31);
    }
}

DataOutputStream* RegionFileCache::_getChunkDataOutputStream(
    ConsoleSaveFile* saveFile, const std::wstring& prefix, int chunkX,
    int chunkZ) {
    RegionFile* r = _getRegionFile(saveFile, prefix, chunkX, chunkZ);
    if (useSplitSaves(saveFile->getSavePlatform())) {
        return r->getChunkDataOutputStream(chunkX & 15, chunkZ & 15);
    } else {
        return r->getChunkDataOutputStream(chunkX & 31, chunkZ & 31);
    }
}

RegionFileCache::~RegionFileCache() { _clear(); }
