#pragma once

class EntityTile {
public:
    virtual std::shared_ptr<TileEntity> newTileEntity(Level* level) = 0;
};