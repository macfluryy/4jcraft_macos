#pragma once

#include <stdint.h>

#include <format>
#include <memory>
#include <mutex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "CompressedTileStorage.h"
#include "SparseDataStorage.h"
#include "SparseLightStorage.h"
#include "minecraft/world/entity/Entity.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/LightLayer.h"
#include "minecraft/world/level/TilePos.h"

class DataLayer;
class TileEntity;
class Random;
class ChunkSource;
class EntitySelector;
class AABB;
class Biome;
class BiomeSource;
class ChunkPos;
class CompoundTag;
class CompressedTileStorage;
class DataInputStream;
class DataOutputStream;
class Entity;
class SparseDataStorage;
class SparseLightStorage;

#define SHARING_ENABLED
class TileCompressData_SPU;

class LevelChunk {
    friend class TileCompressData_SPU;
    friend class LevelRenderer;

public:
    std::vector<uint8_t> biomes;  

    
    const int ENTITY_BLOCKS_LENGTH;
    static const int BLOCKS_LENGTH = Level::CHUNK_TILE_COUNT;  

    static bool touchedSky;

    enum EColumnFlag {
        eColumnFlag_recheck = 1,
        eColumnFlag_biomeOk = 2,
        eColumnFlag_biomeHasSnow = 4,
        eColumnFlag_biomeHasRain = 8,
    };

    
    
    
private:
    CompressedTileStorage* lowerBlocks;  
    CompressedTileStorage* upperBlocks;  
public:
    bool isRenderChunkEmpty(int y);
    void setBlockData(
        std::vector<uint8_t>& data);  
                                      
    void getBlockData(std::vector<uint8_t>&
                          data);  
                                  
    int getBlocksAllocatedSize(int* count0, int* count1, int* count2,
                               int* count4, int* count8);

    bool loaded;
    unsigned char
        rainHeights[16 * 16];  
                               
    unsigned char columnFlags[16 * 8];  
                                        
                                        
                                        
    Level* level;

    
    
private:
    SparseDataStorage* lowerData;  
    SparseDataStorage* upperData;  
public:
    void setDataData(
        std::vector<uint8_t>& data);  
                                      
    void getDataData(
        std::vector<uint8_t>& data);  
                                      

    
private:
    
    

    SparseLightStorage* lowerSkyLight;    
    SparseLightStorage* upperSkyLight;    
    SparseLightStorage* lowerBlockLight;  
    SparseLightStorage* upperBlockLight;  
public:
    void getSkyLightData(
        std::vector<uint8_t>&
            data);  
                    
    void getBlockLightData(
        std::vector<uint8_t>&
            data);  
                    
                    
    void setSkyLightData(
        std::vector<uint8_t>&
            data);  
                    
                    
    void setBlockLightData(
        std::vector<uint8_t>&
            data);  
                    
                    
    void setSkyLightDataAllBright();  
    bool isLowerBlockStorageCompressed();
    int isLowerBlockLightStorageCompressed();
    int isLowerDataStorageCompressed();

    void writeCompressedBlockData(DataOutputStream* dos);
    void writeCompressedDataData(DataOutputStream* dos);
    void writeCompressedSkyLightData(DataOutputStream* dos);
    void writeCompressedBlockLightData(DataOutputStream* dos);

    void readCompressedBlockData(DataInputStream* dis);
    void readCompressedDataData(DataInputStream* dis);
    void readCompressedSkyLightData(DataInputStream* dis);
    void readCompressedBlockLightData(DataInputStream* dis);

    std::vector<uint8_t> heightmap;
    int minHeight;
    int x, z;

private:
    bool hasGapsToCheck;

public:
    std::unordered_map<TilePos, std::shared_ptr<TileEntity>, TilePosKeyHash,
                       TilePosKeyEq>
        tileEntities;
    std::vector<std::shared_ptr<Entity> >** entityBlocks;

    static const int sTerrainPopulatedFromHere = 2;
    static const int sTerrainPopulatedFromW = 4;
    static const int sTerrainPopulatedFromS = 8;
    static const int sTerrainPopulatedFromSW = 16;
    static const int sTerrainPopulatedAllAffecting =
        30;  
             
    static const int sTerrainPopulatedFromNW = 32;
    static const int sTerrainPopulatedFromN = 64;
    static const int sTerrainPopulatedFromNE = 128;
    static const int sTerrainPopulatedFromE = 256;
    static const int sTerrainPopulatedFromSE = 512;
    static const int sTerrainPopulatedAllNeighbours =
        1022;  
               
    static const int sTerrainPostPostProcessed =
        1024;  
               

    short terrainPopulated;  
    short* serverTerrainPopulated;  

    void setUnsaved(bool unsaved);  
protected:
    
    bool m_unsaved;

public:
    bool dontSave;
    bool lastSaveHadEntities;
#if defined(SHARING_ENABLED)
    bool sharingTilesAndData;  
#endif
    bool emissiveAdded;                              
    void stopSharingTilesAndData();                  
    virtual void reSyncLighting();                   
    void startSharingTilesAndData(int forceMs = 0);  
    int64_t lastUnsharedTime;                        
    int64_t lastSaveTime;
    bool seenByPlayer;
    int lowestHeightmap;
    int64_t inhabitedTime;

#if defined(_LARGE_WORLDS)
    bool m_bUnloaded;
    CompoundTag* m_unloadedEntitiesTag;
#endif

    
private:
    int checkLightPosition;

public:
    virtual void init(Level* level, int x, int z);
    LevelChunk(Level* level, int x, int z);
    LevelChunk(Level* level, std::vector<uint8_t>& blocks, int x, int z);
    LevelChunk(Level* level, int x, int z, LevelChunk* lc);
    virtual ~LevelChunk();

    virtual bool isAt(int x, int z);

    virtual int getHeightmap(int x, int z);
    int getHighestSectionPosition();
    virtual void recalcBlockLights();

    virtual void recalcHeightmapOnly();

    virtual void recalcHeightmap();

    virtual void lightLava();

private:
    void lightGaps(int x, int z);
    
public:
    void recheckGaps(bool bForce = false);  
private:
    void lightGap(int x, int z, int source);
    void lightGap(int x, int z, int y1, int y2);

    void recalcHeight(int x, int yStart, int z);

public:
    virtual int getTileLightBlock(int x, int y, int z);
    virtual int getTile(int x, int y, int z);
    virtual bool setTileAndData(int x, int y, int z, int _tile, int _data);
    virtual bool setTile(int x, int y, int z, int _tile);
    virtual int getData(int x, int y, int z);
    virtual bool setData(int x, int y, int z, int val, int mask,
                         bool* maskedBitsChanged);  
    virtual int getBrightness(LightLayer::variety layer, int x, int y, int z);
    virtual void getNeighbourBrightnesses(int* brightnesses,
                                          LightLayer::variety layer, int x,
                                          int y, int z);  
    virtual void setBrightness(LightLayer::variety layer, int x, int y, int z,
                               int brightness);
    virtual int getRawBrightness(int x, int y, int z, int skyDampen);
    virtual void addEntity(std::shared_ptr<Entity> e);
    virtual void removeEntity(std::shared_ptr<Entity> e);
    virtual void removeEntity(std::shared_ptr<Entity> e, int yc);
    virtual bool isSkyLit(int x, int y, int z);
    virtual void skyBrightnessChanged();
    virtual std::shared_ptr<TileEntity> getTileEntity(int x, int y, int z);
    virtual void addTileEntity(std::shared_ptr<TileEntity> te);
    virtual void setTileEntity(int x, int y, int z,
                               std::shared_ptr<TileEntity> tileEntity);
    virtual void removeTileEntity(int x, int y, int z);
    virtual void load();
    virtual void unload(bool unloadTileEntities);  
    virtual bool containsPlayer();                 
#if defined(_LARGE_WORLDS)
    virtual bool isUnloaded();
#endif
    virtual void markUnsaved();
    virtual void getEntities(std::shared_ptr<Entity> except, AABB* bb,
                             std::vector<std::shared_ptr<Entity> >& es,
                             const EntitySelector* selector);
    virtual void getEntitiesOfClass(const std::type_info& ec, AABB* bb,
                                    std::vector<std::shared_ptr<Entity> >& es,
                                    const EntitySelector* selector);
    virtual int countEntities();
    virtual bool shouldSave(bool force);
    virtual int getBlocksAndData(
        std::vector<uint8_t>* data, int x0, int y0, int z0, int x1, int y1,
        int z1, int p,
        bool includeLighting = true);  
    static void tileUpdatedCallback(int x, int y, int z, void* param,
                                    int yparam);  
    virtual int setBlocksAndData(
        std::vector<uint8_t>& data, int x0, int y0, int z0, int x1, int y1,
        int z1, int p,
        bool includeLighting = true);  
    virtual bool testSetBlocksAndData(std::vector<uint8_t>& data, int x0,
                                      int y0, int z0, int x1, int y1, int z1,
                                      int p);  
    virtual void setCheckAllLight();

    virtual Random* getRandom(int64_t l);
    virtual bool isEmpty();
    virtual void attemptCompression();

#if defined(SHARING_ENABLED)
    static std::recursive_mutex m_csSharing;  
#endif
    
    static std::recursive_mutex m_csEntities;
    static std::recursive_mutex m_csTileEntities;  
    static void staticCtor();
    void checkPostProcess(ChunkSource* source, ChunkSource* parent, int x,
                          int z);
    void checkChests(ChunkSource* source, int x, int z);  
    int getTopRainBlock(int x,
                        int z);  
    void tick();  
    ChunkPos* getPos();
    bool isYSpaceEmpty(int y1, int y2);
    void reloadBiomes();  
    virtual Biome* getBiome(int x, int z, BiomeSource* biomeSource);
    std::vector<uint8_t> getBiomes();
    void setBiomes(std::vector<uint8_t>& biomes);
    bool biomeHasRain(int x, int z);  
    bool biomeHasSnow(int x, int z);  
private:
    void updateBiomeFlags(int x, int z);  
public:
    void compressLighting();  
    void compressBlocks();    
    void compressData();      
    int getHighestNonEmptyY();
    std::vector<uint8_t> getReorderedBlocksAndData(int x, int y, int z, int xs,
                                                   int& ys, int zs);
    static void reorderBlocksAndDataToXZY(int y0, int xs, int ys, int zs,
                                          std::vector<uint8_t>* data);
#if defined(LIGHT_COMPRESSION_STATS)
    int getBlockLightPlanesLower() { return lowerBlockLight->count; }
    int getSkyLightPlanesLower() { return lowerSkyLight->count; }
    int getBlockLightPlanesUpper() { return upperBlockLight->count; }
    int getSkyLightPlanesUpper() { return upperSkyLight->count; }
#endif
#if defined(DATA_COMPRESSION_STATS)
    int getDataPlanes() { return data->count; }
#endif
};
