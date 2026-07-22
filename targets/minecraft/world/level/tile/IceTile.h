#pragma once
#include "HalfTransparentTile.h"

class Random;

class IceTile : public HalfTransparentTile {
public:
    IceTile(int id);
    virtual int getRenderLayer();
    virtual bool shouldRenderFace(LevelSource* level, int x, int y, int z,
                                  int face);
    virtual void playerDestroy(Level* level, std::shared_ptr<Player> player,
                               int x, int y, int z, int data);
    virtual int getResourceCount(Random* random);
    virtual void tick(Level* level, int x, int y, int z, Random* random);
    virtual int getPistonPushReaction();

    
    
    virtual bool shouldTileTick(Level* level, int x, int y, int z);
};
