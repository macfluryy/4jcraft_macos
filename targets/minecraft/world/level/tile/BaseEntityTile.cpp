#include "BaseEntityTile.h"

#include <memory>

#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/tile/Tile.h"
#include "minecraft/world/level/tile/entity/TileEntity.h"

class Material;

BaseEntityTile::BaseEntityTile(int id, Material* material,
                               bool isSolidRender )
    : Tile(id, material, isSolidRender) {
    _isEntityTile = true;
}

void BaseEntityTile::onPlace(Level* level, int x, int y, int z) {
    Tile::onPlace(level, x, y, z);
    
}

void BaseEntityTile::onRemove(Level* level, int x, int y, int z, int id,
                              int data) {
    Tile::onRemove(level, x, y, z, id, data);
    level->removeTileEntity(x, y, z);
}

bool BaseEntityTile::triggerEvent(Level* level, int x, int y, int z, int b0,
                                  int b1) {
    Tile::triggerEvent(level, x, y, z, b0, b1);
    std::shared_ptr<TileEntity> te = level->getTileEntity(x, y, z);
    if (te != nullptr) {
        return te->triggerEvent(b0, b1);
    }
    return false;
}