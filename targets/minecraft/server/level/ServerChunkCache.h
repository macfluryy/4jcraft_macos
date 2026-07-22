#pragma once
#include <deque>
#include <format>
#include <mutex>
#include <string>
#include <vector>

#include "platform/C4JThread.h"
#include "java/File.h"
#include "java/JavaIntHash.h"
#include "minecraft/world/level/biome/Biome.h"
#include "minecraft/world/level/chunk/ChunkSource.h"
#include "minecraft/world/level/levelgen/RandomLevelSource.h"



class ServerLevel;
class ChunkStorage;
class LevelChunk;

class ServerChunkCache : public ChunkSource {
private:
    
private:
    LevelChunk* emptyChunk;
    ChunkSource* source;
    ChunkStorage* storage;

public:
    bool autoCreate;

private:
    LevelChunk** cache;
    std::vector<LevelChunk*> m_loadedChunkList;
    ServerLevel* level;

#ifdef _LARGE_WORLDS
    std::deque<LevelChunk*> m_toDrop;
    LevelChunk** m_unloadedCache;
#endif

    
    std::recursive_mutex m_csLoadCreate;
    
    int XZSIZE;
    int XZOFFSET;

public:
    ServerChunkCache(ServerLevel* level, ChunkStorage* storage,
                     ChunkSource* source);
    virtual ~ServerChunkCache();
    virtual bool hasChunk(int x, int z);
    std::vector<LevelChunk*>* getLoadedChunkList();
    void drop(int x, int z);
    void dropAll();
    virtual LevelChunk* create(int x, int z);
    LevelChunk* create(int x, int z, bool asyncPostProcess);  
    virtual LevelChunk* getChunk(int x, int z);
#ifdef _LARGE_WORLDS
    LevelChunk* getChunkLoadedOrUnloaded(int x, int z);  
    void overwriteLevelChunkFromSource(
        int x, int z);  
                        
    void overwriteHellLevelChunkFromSource(
        int x, int z, int minVal,
        int maxVal);  
                      
    void updateOverwriteHellChunk(LevelChunk* origChunk,
                                  LevelChunk* playerChunk, int xMin, int xMax,
                                  int zMin, int zMax);

#endif
    virtual LevelChunk** getCache() { return cache; }  

    
#ifdef _LARGE_WORLDS
    void dontDrop(int x, int z);
#endif

private:
    LevelChunk* load(int x, int z);
    void saveEntities(LevelChunk* levelChunk);
    void save(LevelChunk* levelChunk);

    void updatePostProcessFlag(short flag, int x, int z, int xo, int zo,
                               LevelChunk* lc);              
    void updatePostProcessFlags(int x, int z);               
    void flagPostProcessComplete(short flag, int x, int z);  
public:
    virtual void postProcess(ChunkSource* parent, int x, int z);

private:
#ifdef _LARGE_WORLDS
    static const int MAX_SAVES = 20;
#else
    
    
    static const int MAX_SAVES = 1;
#endif

public:
    virtual bool saveAllEntities();
    virtual bool save(bool force, ProgressListener* progressListener);
    virtual bool tick();
    virtual bool shouldSave();
    virtual std::wstring gatherStats();

    virtual std::vector<Biome::MobSpawnerData*>* getMobsAt(
        MobCategory* mobCategory, int x, int y, int z);
    virtual TilePos* findNearestMapFeature(Level* level,
                                           const std::wstring& featureName,
                                           int x, int y, int z);
    virtual void recreateLogicStructuresForChunk(int chunkX, int chunkZ);

private:
    typedef struct _SaveThreadData {
        ServerChunkCache* cache;
        LevelChunk* chunkToSave;
        bool saveEntities;
        bool useSharedThreadStorage;
        C4JThread::Event* notificationEvent;
        C4JThread::Event* wakeEvent;  
                                      
    } SaveThreadData;

public:
    static int runSaveThreadProc(void* lpParam);
};
