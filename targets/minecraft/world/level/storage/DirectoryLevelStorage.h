#pragma once

#if defined(_LARGE_WORLDS)


#define MAXIMUM_MAP_SAVE_DATA 8192  




#define MAP_OVERWORLD_DEFAULT_INDEX 65535
#define MAP_NETHER_DEFAULT_INDEX 65534
#define MAP_END_DEFAULT_INDEX 65533
#else
#define MAXIMUM_MAP_SAVE_DATA 256




#define MAP_OVERWORLD_DEFAULT_INDEX 255
#define MAP_NETHER_DEFAULT_INDEX 254
#define MAP_END_DEFAULT_INDEX 253
#endif


#define END_DIMENSION_MAP_MAPPINGS_SAVE_VERSION 5

#include <stdint.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "platform/PlatformTypes.h"
#include "LevelStorage.h"
#include "PlayerIO.h"
#include "java/File.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/ConsoleSavePath.h"
#include "nbt/CompoundTag.h"

class ConsoleSaveFile;
class ByteArrayOutputStream;
class DataInputStream;
class DataOutputStream;





typedef struct _MapDataMappings {
    PlayerUID xuids[MAXIMUM_MAP_SAVE_DATA];
    uint8_t dimensions[MAXIMUM_MAP_SAVE_DATA / 4];

    _MapDataMappings();
    int getDimension(int id);
    void setMapping(int id, PlayerUID xuid, int dimension);
} MapDataMappings;


typedef struct _MapDataMappings_old {
    PlayerUID xuids[MAXIMUM_MAP_SAVE_DATA];
    uint8_t dimensions[MAXIMUM_MAP_SAVE_DATA / 8];

    _MapDataMappings_old();
    int getDimension(int id);
    void setMapping(int id, PlayerUID xuid, int dimension);
} MapDataMappings_old;

class DirectoryLevelStorage : public LevelStorage, public PlayerIO {
private:
    


    const File dir;
    
    const ConsoleSavePath playerDir;
    
    const ConsoleSavePath dataDir;
    const int64_t sessionId;
    const std::wstring levelId;

    static const std::wstring sc_szPlayerDir;
    
#if defined(_LARGE_WORLDS)
    class PlayerMappings {
        friend class DirectoryLevelStorage;

    private:
        std::unordered_map<int64_t, short> m_mappings;

    public:
        void addMapping(int id, int centreX, int centreZ, int dimension,
                        int scale);
        bool getMapping(int& id, int centreX, int centreZ, int dimension,
                        int scale);
        void writeMappings(DataOutputStream* dos);
        void readMappings(DataInputStream* dis);
    };
    std::unordered_map<PlayerUID, PlayerMappings> m_playerMappings;
    std::vector<uint8_t> m_usedMappings;
#else
    MapDataMappings m_mapDataMappings;
    MapDataMappings m_saveableMapDataMappings;
#endif
    bool m_bHasLoadedMapDataMappings;

    std::unordered_map<std::wstring, ByteArrayOutputStream*> m_cachedSaveData;
    std::vector<short>
        m_mapFilesToDelete;  
                             

protected:
    ConsoleSaveFile* m_saveFile;

public:
    virtual ConsoleSaveFile* getSaveFile() { return m_saveFile; }
    virtual void flushSaveFile(bool autosave);

public:
    DirectoryLevelStorage(ConsoleSaveFile* saveFile, const File dir,
                          const std::wstring& levelId, bool createPlayerDir);
    ~DirectoryLevelStorage();

private:
    void initiateSession();

protected:
    File getFolder();

public:
    void checkSession();
    virtual ChunkStorage* createChunkStorage(Dimension* dimension);
    LevelData* prepareLevel();
    virtual void saveLevelData(LevelData* levelData,
                               std::vector<std::shared_ptr<Player> >* players);
    virtual void saveLevelData(LevelData* levelData);
    virtual void save(std::shared_ptr<Player> player);
    virtual CompoundTag* load(
        std::shared_ptr<Player>
            player);  
                      
    virtual CompoundTag* loadPlayerDataTag(PlayerUID xuid);
    virtual void clearOldPlayerFiles();  
    PlayerIO* getPlayerIO();
    virtual void closeAll();
    ConsoleSavePath getDataFile(const std::wstring& id);
    std::wstring getLevelId();

    
    virtual int getAuxValueForMap(PlayerUID xuid, int dimension, int centreXC,
                                  int centreZC, int scale);
    virtual void saveMapIdLookup();
    virtual void deleteMapFilesForPlayer(std::shared_ptr<Player> player);
    virtual void saveAllCachedData();
    void resetNetherPlayerPositions();  
    static std::wstring getPlayerDir() { return sc_szPlayerDir; }

private:
    void dontSaveMapMappingForPlayer(PlayerUID xuid);
    void deleteMapFilesForPlayer(PlayerUID xuid);
};
