#include <memory>

#include "DaylightDetectorTileEntity.h"
#include "Minecraft.World/net/minecraft/SharedConstants.h"
#include "Minecraft.World/net/minecraft/world/level/Level.h"
#include "Minecraft.World/net/minecraft/world/level/tile/DaylightDetectorTile.h"
#include "Minecraft.World/net/minecraft/world/level/tile/Tile.h"

DaylightDetectorTileEntity::DaylightDetectorTileEntity() {}

void DaylightDetectorTileEntity::tick() {
    if (level != nullptr && !level->isClientSide &&
        (level->getGameTime() % SharedConstants::TICKS_PER_SECOND) == 0) {
        tile = getTile();
        if (tile != nullptr &&
            dynamic_cast<DaylightDetectorTile*>(tile) != nullptr) {
            ((DaylightDetectorTile*)tile)->updateSignalStrength(level, x, y, z);
        }
    }
}

// 4J Added
std::shared_ptr<TileEntity> DaylightDetectorTileEntity::clone() {
    std::shared_ptr<DaylightDetectorTileEntity> result =
        std::shared_ptr<DaylightDetectorTileEntity>(
            new DaylightDetectorTileEntity());
    TileEntity::clone(result);

    return result;
}