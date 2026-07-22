#pragma once

#include <memory>

#include "LocatableSource.h"

class Tile;
class Material;
class TileEntity;

class BlockSource : public LocatableSource {
public:
    


    virtual double getX() = 0;

    


    virtual double getY() = 0;

    


    virtual double getZ() = 0;

    virtual int getBlockX() = 0;
    virtual int getBlockY() = 0;
    virtual int getBlockZ() = 0;

    virtual Tile* getType() = 0;
    virtual int getData() = 0;
    virtual Material* getMaterial() = 0;

    virtual std::shared_ptr<TileEntity> getEntity() = 0;
};