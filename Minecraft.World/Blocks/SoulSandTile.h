#pragma once
#include "Tile.h"
#include "../Util/Definitions.h"

class SoulSandTile : public Tile {
public:
    SoulSandTile(int id);
    virtual AABB* getAABB(Level* level, int x, int y, int z);
    virtual void entityInside(Level* level, int x, int y, int z,
                              std::shared_ptr<Entity> entity);
};