#include "TheEndPortalTileEntity.h"

#include <memory>

#include "minecraft/world/level/tile/entity/TileEntity.h"


std::shared_ptr<TileEntity> TheEndPortalTileEntity::clone() {
    std::shared_ptr<TheEndPortalTileEntity> result =
        std::make_shared<TheEndPortalTileEntity>();
    TileEntity::clone(result);
    return result;
}