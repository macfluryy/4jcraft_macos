#pragma once

#include "Tile.h"

class SnowTile : public Tile {
    friend class Tile;

protected:
    SnowTile(int id);

public:
    int getResource(int data, Random* random, int playerBonusLevel);

    int getResourceCount(Random* random);

    void tick(Level* level, int x, int y, int z, Random* random);

    
    
    virtual bool shouldTileTick(Level* level, int x, int y, int z);
};