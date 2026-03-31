#include <memory>

#include "TheEndPortalTileEntity.h"
#include "Minecraft.World/net/minecraft/world/level/tile/entity/TileEntity.h"

// 4J Added
std::shared_ptr<TileEntity> TheEndPortalTileEntity::clone() {
    std::shared_ptr<TheEndPortalTileEntity> result =
        std::make_shared<TheEndPortalTileEntity>();
    TileEntity::clone(result);
    return result;
}