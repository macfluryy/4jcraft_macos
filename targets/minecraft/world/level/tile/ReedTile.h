#pragma once

#include <optional>

#include "Tile.h"

class Random;
class Level;

class ReedTile : public Tile {
    friend class Tile;

protected:
    ReedTile(int id);

public:
    virtual void updateDefaultShape();  
    void tick(Level* level, int x, int y, int z, Random* random);

public:
    bool mayPlace(Level* level, int x, int y, int z);

public:
    void neighborChanged(Level* level, int x, int y, int z, int type);

protected:
    const void checkAlive(Level* level, int x, int y, int z);

public:
    bool canSurvive(Level* level, int x, int y, int z);

public:
    std::optional<AABB> getAABB(Level* level, int x, int y, int z);

public:
    int getResource(int data, Random* random, int playerBonusLevel);

public:
    bool blocksLight();

public:
    bool isSolidRender(bool isServerLevel = false);

public:
    bool isCubeShaped();

public:
    int getRenderShape();
    virtual int cloneTileId(Level* level, int x, int y, int z);

    
    
    virtual bool shouldTileTick(Level* level, int x, int y, int z);
};
