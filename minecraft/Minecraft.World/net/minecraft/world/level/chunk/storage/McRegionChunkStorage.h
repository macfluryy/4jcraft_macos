#pragma once

#include <mutex>

#include "ChunkStorage.h"
#include "../LevelChunk.h"
#include "RegionFileCache.h"
#include "nbt/com.mojang.nbt.h"
#include "OldChunkStorage.h"

class ConsoleSaveFile;

class McRegionChunkStorage : public ChunkStorage {
private:
    const std::wstring m_prefix;
    ConsoleSaveFile* m_saveFile;
    static std::mutex cs_memory;

    std::unordered_map<int64_t, std::vector<uint8_t>> m_entityData;

    static std::deque<DataOutputStream*> s_chunkDataQueue;
    static int s_runningThreadCount;
    static C4JThread* s_saveThreads[3];

public:
    McRegionChunkStorage(ConsoleSaveFile* saveFile, const std::wstring& prefix);
    ~McRegionChunkStorage();
    static void staticCtor();

    virtual LevelChunk* load(Level* level, int x, int z);
    virtual void save(Level* level, LevelChunk* levelChunk);
    virtual void saveEntities(Level* level, LevelChunk* levelChunk);
    virtual void loadEntities(Level* level, LevelChunk* levelChunk);
    virtual void tick();
    virtual void flush();
    virtual void WaitForAll();                 // 4J Added
    virtual void WaitIfTooManyQueuedChunks();  // 4J Added

private:
    static void WaitForAllSaves();
    static void WaitForSaves();
    static int runSaveThreadProc(void* lpParam);
};
