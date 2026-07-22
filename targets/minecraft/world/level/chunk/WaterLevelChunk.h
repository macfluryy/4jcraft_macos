#pragma once
#include <stdint.h>

#include <format>
#include <memory>
#include <typeinfo>
#include <vector>

#include "LevelChunk.h"
#include "minecraft/world/level/LightLayer.h"

class Level;
class AABB;
class Entity;

class WaterLevelChunk : public LevelChunk {
public:
    using LevelChunk::getEntities;
    using LevelChunk::getEntitiesOfClass;

    WaterLevelChunk(Level* level, std::vector<uint8_t>& blocks, int x, int z);
    bool isAt(int x, int z);
    void recalcBlockLights();
    void recalcHeightmapOnly();
    void recalcHeightmap();
    void lightLava();
    bool setTileAndData(int x, int y, int z, int _tile, int _data);
    bool setTile(int x, int y, int z, int _tile);
    bool setData(int x, int y, int z, int val, int mask,
                 bool* maskedBitsChanged);  
    void setBrightness(LightLayer::variety layer, int x, int y, int z,
                       int brightness);
    void setLevelChunkBrightness(
        LightLayer::variety layer, int x, int y, int z,
        int brightness);  
                          
    void addEntity(std::shared_ptr<Entity> e);
    void removeEntity(std::shared_ptr<Entity> e);
    void removeEntity(std::shared_ptr<Entity> e, int yc);
    void skyBrightnessChanged();
    std::shared_ptr<TileEntity> getTileEntity(int x, int y, int z);
    void addTileEntity(std::shared_ptr<TileEntity> te);
    void setTileEntity(int x, int y, int z,
                       std::shared_ptr<TileEntity> tileEntity);
    void removeTileEntity(int x, int y, int z);
    void load();
    void unload(bool unloadTileEntities);  
    bool containsPlayer();                 
    void markUnsaved();
    void getEntities(std::shared_ptr<Entity> except, AABB bb,
                     std::vector<std::shared_ptr<Entity> >& es);
    void getEntitiesOfClass(const std::type_info& ec, AABB bb,
                            std::vector<std::shared_ptr<Entity> >& es);
    int countEntities();
    bool shouldSave(bool force);
    void setBlocks(std::vector<uint8_t>& newBlocks, int sub);
    int setBlocksAndData(
        std::vector<uint8_t>& data, int x0, int y0, int z0, int x1, int y1,
        int z1, int p,
        bool includeLighting = true);  
    bool testSetBlocksAndData(std::vector<uint8_t>& data, int x0, int y0,
                              int z0, int x1, int y1, int z1, int p);
    Random* getRandom(int64_t l);
    virtual Biome* getBiome(int x, int z, BiomeSource* biomeSource);
    virtual void reSyncLighting() {};  
};
