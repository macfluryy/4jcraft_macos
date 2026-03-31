#include "Minecraft.World/Header Files/stdafx.h"

#include "DropperTileEntity.h"

std::wstring DropperTileEntity::getName() {
    return hasCustomName() ? name : app.GetString(IDS_CONTAINER_DROPPER);
}

// 4J Added
std::shared_ptr<TileEntity> DropperTileEntity::clone() {
    std::shared_ptr<DropperTileEntity> result =
        std::shared_ptr<DropperTileEntity>(new DropperTileEntity());
    TileEntity::clone(result);

    result->name = name;

    return result;
}