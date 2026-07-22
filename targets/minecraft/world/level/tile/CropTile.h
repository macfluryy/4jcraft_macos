#pragma once
#include "PlantTile.h"
#include "minecraft/world/level/material/Material.h"

class Random;
class Level;
class ChunkRebuildData;
class Icon;

class CropTile : public Bush {
    friend class Tile;
    friend class ChunkRebuildData;

private:
    Icon** icons;

protected:
    CropTile(int id);
    virtual bool mayPlaceOn(int tile);

public:
    
    virtual void updateDefaultShape();
    virtual void tick(Level* level, int x, int y, int z, Random* random);
    virtual void growCrops(Level* level, int x, int y, int z);

private:
    float getGrowthSpeed(Level* level, int x, int y, int z);

public:
    virtual Icon* getTexture(int face, int data);
    virtual int getRenderShape();

protected:
    virtual int getBaseSeedId();
    virtual int getBasePlantId();

public:
    



    virtual void spawnResources(Level* level, int x, int y, int z, int data,
                                float odds, int playerBonus);
    virtual int getResource(int data, Random* random, int playerBonusLevel);
    virtual int getResourceCount(Random* random);
    virtual int cloneTileId(Level* level, int x, int y, int z);
    
    virtual void registerIcons(IconRegister* iconRegister);
};