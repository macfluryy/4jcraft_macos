#include "../../Platform/stdafx.h"
#include "../../Headers/net.minecraft.world.level.h"
#include "../../Headers/net.minecraft.world.level.tile.h"
#include "DaylightDetectorTileEntity.h"

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