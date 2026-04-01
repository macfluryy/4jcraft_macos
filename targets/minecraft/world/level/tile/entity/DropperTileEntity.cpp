#include "DropperTileEntity.h"

#include <memory>
#include <string>

#include "Minecraft.Client/Linux/Linux_App.h"
#include "strings.h"
#include "minecraft/world/level/tile/entity/TileEntity.h"

std::wstring DropperTileEntity::getName() {
    return hasCustomName() ? name : app.GetString(IDS_CONTAINER_DROPPER);
}

// 4J Added
std::shared_ptr<TileEntity> DropperTileEntity::clone() {
    std::shared_ptr<DropperTileEntity> result =
        std::make_shared<DropperTileEntity>();
    TileEntity::clone(result);

    result->name = name;

    return result;
}