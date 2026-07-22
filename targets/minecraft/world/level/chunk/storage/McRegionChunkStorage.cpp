#include "McRegionChunkStorage.h"

#include <stdio.h>
#include <string.h>

#include <chrono>
#include <mutex>
#include <thread>
#include <utility>

#include "IPlatformInput.h"
#include "app/common/src/Console_Debug_enum.h"
#include "app/mac/MacGame.h"
#include "platform/C4JThread.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/compression.h"
#include "java/InputOutputStream/BufferedOutputStream.h"
#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/ByteArrayOutputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/chunk/LevelChunk.h"
#include "minecraft/world/level/chunk/storage/OldChunkStorage.h"
#include "minecraft/world/level/chunk/storage/RegionFileCache.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFile.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFileInputStream.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSaveFileOutputStream.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSavePath.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/FileHeader.h"
#include "minecraft/world/level/storage/LevelData.h"
#include "nbt/CompoundTag.h"
#include "nbt/NbtIo.h"
#include "platform/PlatformServices.h"

class DataInput;

std::mutex McRegionChunkStorage::cs_memory;
std::condition_variable McRegionChunkStorage::s_queueCondition;
std::condition_variable McRegionChunkStorage::s_waitCondition;

std::deque<DataOutputStream*> McRegionChunkStorage::s_chunkDataQueue;
int McRegionChunkStorage::s_runningThreadCount = 0;
C4JThread* McRegionChunkStorage::s_saveThreads[3];

McRegionChunkStorage::McRegionChunkStorage(ConsoleSaveFile* saveFile,
                                           const std::wstring& prefix)
    : m_prefix(prefix) {
    m_saveFile = saveFile;

    
    
    
    if (prefix == L"") {
        m_saveFile->createFile(ConsoleSavePath(L"DIM-1r.-1.-1.mcr"));
        m_saveFile->createFile(ConsoleSavePath(L"DIM-1r.0.-1.mcr"));
        m_saveFile->createFile(ConsoleSavePath(L"DIM-1r.0.0.mcr"));
        m_saveFile->createFile(ConsoleSavePath(L"DIM-1r.-1.0.mcr"));
        m_saveFile->createFile(ConsoleSavePath(L"DIM1/r.-1.-1.mcr"));
        m_saveFile->createFile(ConsoleSavePath(L"DIM1/r.0.-1.mcr"));
        m_saveFile->createFile(ConsoleSavePath(L"DIM1/r.0.0.mcr"));
        m_saveFile->createFile(ConsoleSavePath(L"DIM1/r.-1.0.mcr"));
        m_saveFile->createFile(ConsoleSavePath(L"r.-1.-1.mcr"));
        m_saveFile->createFile(ConsoleSavePath(L"r.0.-1.mcr"));
        m_saveFile->createFile(ConsoleSavePath(L"r.0.0.mcr"));
        m_saveFile->createFile(ConsoleSavePath(L"r.-1.0.mcr"));
    }

#if defined(SPLIT_SAVES)
    ConsoleSavePath currentFile =
        ConsoleSavePath(m_prefix + std::wstring(L"entities.dat"));

    if (m_saveFile->doesFileExist(currentFile)) {
        ConsoleSaveFileInputStream fis =
            ConsoleSaveFileInputStream(m_saveFile, currentFile);
        DataInputStream dis(&fis);

        int count = dis.readInt();

        for (int i = 0; i < count; ++i) {
            int64_t index = dis.readLong();
            CompoundTag* tag = NbtIo::read(&dis);

            ByteArrayOutputStream bos;
            DataOutputStream dos(&bos);
            NbtIo::write(tag, &dos);
            delete tag;

            std::vector<uint8_t> savedData(bos.size());
            memcpy(savedData.data(), bos.buf.data(), bos.size());

            m_entityData[index] = savedData;
        }
    }
#endif
}

McRegionChunkStorage::~McRegionChunkStorage() {
    
}

LevelChunk* McRegionChunkStorage::load(Level* level, int x, int z) {
    DataInputStream* regionChunkInputStream =
        RegionFileCache::getChunkDataInputStream(m_saveFile, m_prefix, x, z);

#if defined(SPLIT_SAVES)
    
    
    if (regionChunkInputStream == nullptr) {
        
        
        uint64_t index =
            ((uint64_t)(uint32_t)(x) << 32) | (((uint64_t)(uint32_t)(z)));

        auto it = m_entityData.find(index);
        if (it != m_entityData.end()) {
            m_entityData.erase(it);
        }
    }
#endif

    LevelChunk* levelChunk = nullptr;

    if (m_saveFile->getOriginalSaveVersion() >=
        SAVE_FILE_VERSION_COMPRESSED_CHUNK_STORAGE) {
        if (regionChunkInputStream != nullptr) {
            levelChunk = OldChunkStorage::load(level, regionChunkInputStream);
            loadEntities(level, levelChunk);
            regionChunkInputStream->deleteChildStream();
            delete regionChunkInputStream;
        }
    } else {
        CompoundTag* chunkData;
        if (regionChunkInputStream != nullptr) {
            chunkData = NbtIo::read((DataInput*)regionChunkInputStream);
        } else {
            return nullptr;
        }

        regionChunkInputStream->deleteChildStream();
        delete regionChunkInputStream;

        if (!chunkData->contains(L"Level")) {
            char buf[256];
            sprintf(buf,
                    "Chunk file at %d, %d is missing level data, skipping\n", x,
                    z);
            app.DebugPrintf(buf);
            delete chunkData;
            return nullptr;
        }
        if (!chunkData->getCompound(L"Level")->contains(L"Blocks")) {
            char buf[256];
            sprintf(buf,
                    "Chunk file at %d, %d is missing block data, skipping\n", x,
                    z);
            app.DebugPrintf(buf);
            delete chunkData;
            return nullptr;
        }
        levelChunk =
            OldChunkStorage::load(level, chunkData->getCompound(L"Level"));
        if (!levelChunk->isAt(x, z)) {
            char buf[256];
            sprintf(buf,
                    "Chunk file at %d, %d is in the wrong location; "
                    "relocating. Expected %d, %d, got %d, %d\n",
                    x, z, x, z, levelChunk->x, levelChunk->z);
            app.DebugPrintf(buf);
            delete levelChunk;
            delete chunkData;
            return nullptr;

            
            
            
            
            
        }
#if defined(SPLIT_SAVES)
        loadEntities(level, levelChunk);
#endif
        delete chunkData;
    }
#if !defined(_CONTENT_PACKAGE)
    if (levelChunk && app.DebugSettingsOn() &&
        app.GetGameSettingsDebugMask(PlatformInput.GetPrimaryPad()) &
            (1L << eDebugSetting_EnableBiomeOverride)) {
        
        levelChunk->reloadBiomes();
    }
#endif
    return levelChunk;
}

void McRegionChunkStorage::save(Level* level, LevelChunk* levelChunk) {
    level->checkSession();

    
    

    
    
    
    
    
    DataOutputStream* output = RegionFileCache::getChunkDataOutputStream(
        m_saveFile, m_prefix, levelChunk->x, levelChunk->z);

    if (m_saveFile->getOriginalSaveVersion() >= SAVE_FILE_VERSION_COMPRESSED_CHUNK_STORAGE) {
            OldChunkStorage::save(levelChunk, level, output);

            {
                std::lock_guard<std::mutex> lock(cs_memory);
                s_chunkDataQueue.push_back(output);
            }
            
            s_queueCondition.notify_one();

    } else {
        CompoundTag* tag;
        {
            std::lock_guard<std::mutex> lock(cs_memory);
            tag = new CompoundTag();
            CompoundTag* levelData = new CompoundTag();
            tag->put(L"Level", levelData);
            OldChunkStorage::save(levelChunk, level, levelData);

            NbtIo::write(tag, output);
        }
        output->close();

        
        
        
        {
            std::lock_guard<std::mutex> lock(cs_memory);
            output->deleteChildStream();
            delete output;
            delete tag;
        }
    }

    LevelData* levelInfo = level->getLevelData();

    
    
    
    
    
    levelInfo->setSizeOnDisk(this->m_saveFile->getSizeOnDisk());
    
    
    
}

void McRegionChunkStorage::saveEntities(Level* level, LevelChunk* levelChunk) {
#if defined(SPLIT_SAVES)
    
    uint64_t index = ((uint64_t)(uint32_t)(levelChunk->x) << 32) |
                     (((uint64_t)(uint32_t)(levelChunk->z)));

    CompoundTag* newTag = new CompoundTag();
    bool savedEntities =
        OldChunkStorage::saveEntities(levelChunk, level, newTag);

    if (savedEntities) {
        ByteArrayOutputStream bos;
        DataOutputStream dos(&bos);
        NbtIo::write(newTag, &dos);

        std::vector<uint8_t> savedData(bos.size());
        memcpy(savedData.data(), bos.buf.data(), bos.size());

        m_entityData[index] = savedData;
    } else {
        auto it = m_entityData.find(index);
        if (it != m_entityData.end()) {
            m_entityData.erase(it);
        }
    }
    delete newTag;

#endif
}

void McRegionChunkStorage::loadEntities(Level* level, LevelChunk* levelChunk) {
#if defined(SPLIT_SAVES)
    int64_t index = ((int64_t)(levelChunk->x) << 32) |
                    (((int64_t)(levelChunk->z)) & 0x00000000FFFFFFFF);

    auto it = m_entityData.find(index);
    if (it != m_entityData.end()) {
        ByteArrayInputStream bais(it->second);
        DataInputStream dis(&bais);
        CompoundTag* tag = NbtIo::read(&dis);
        OldChunkStorage::loadEntities(levelChunk, level, tag);
        bais.reset();
        delete tag;
    }
#endif
}

void McRegionChunkStorage::tick() { m_saveFile->tick(); }

void McRegionChunkStorage::flush() {
#if defined(SPLIT_SAVES)
    ConsoleSavePath currentFile =
        ConsoleSavePath(m_prefix + std::wstring(L"entities.dat"));
    ConsoleSaveFileOutputStream fos =
        ConsoleSaveFileOutputStream(m_saveFile, currentFile);
    BufferedOutputStream bos(&fos, 1024 * 1024);
    DataOutputStream dos(&bos);

    dos.writeInt(m_entityData.size());

    for (auto it = m_entityData.begin(); it != m_entityData.end(); ++it) {
        dos.writeLong(it->first);
        dos.write(it->second, 0, it->second.size());
    }
    bos.flush();

#endif
}

void McRegionChunkStorage::staticCtor() {
    for (unsigned int i = 0; i < 3; ++i) {
        char threadName[256];
        sprintf(threadName, "McRegion Save thread %d\n", i);
        C4JThread::setThreadName(0, threadName);

        
        
        s_saveThreads[i] =
            new C4JThread(runSaveThreadProc, nullptr, threadName);

        

        
        s_saveThreads[i]->run();
    }
}


int McRegionChunkStorage::runSaveThreadProc(void* lpParam) {
    Compression::CreateNewThreadStorage();

    bool running = true;
    DataOutputStream* dos = nullptr;
    while (running) {
        {
            std::unique_lock<std::mutex> lock(cs_memory);
            s_queueCondition.wait(lock, [] { return !s_chunkDataQueue.empty(); });
            dos = s_chunkDataQueue.front();
            s_chunkDataQueue.pop_front();
            s_runningThreadCount++;
        } 

        if (dos) {
            dos->close();
            dos->deleteChildStream();
            delete dos;
            dos = nullptr;
        }

        {
            std::lock_guard<std::mutex> lock(cs_memory);
            s_runningThreadCount--;
        }

        
        s_waitCondition.notify_all();
    }

    Compression::ReleaseThreadStorage();
    return 0;
}

void McRegionChunkStorage::WaitForAll() { WaitForAllSaves(); }

void McRegionChunkStorage::WaitIfTooManyQueuedChunks() { WaitForSaves(); }



void McRegionChunkStorage::WaitForAllSaves() {
    std::unique_lock<std::mutex> lock(cs_memory);
    
    s_waitCondition.wait(lock, [] {
        return s_chunkDataQueue.empty() && s_runningThreadCount == 0;
    });
}


void McRegionChunkStorage::WaitForSaves() {
    static const int MAX_QUEUE_SIZE = 12;
    static const int DESIRED_QUEUE_SIZE = 6;


    std::unique_lock<std::mutex> lock(cs_memory);
    if (s_chunkDataQueue.size() > MAX_QUEUE_SIZE) {
        
        s_waitCondition.wait(lock, [] {
            return s_chunkDataQueue.size() <= DESIRED_QUEUE_SIZE;
        });
    }
}
