#include "MinecartChest.h"

#include "minecraft/network/packet/ContainerOpenPacket.h"
#include "minecraft/world/entity/item/MinecartContainer.h"
#include "minecraft/world/level/tile/ChestTile.h"
#include "minecraft/world/level/tile/Tile.h"

class Level;

MinecartChest::MinecartChest(Level* level) : MinecartContainer(level) {
    
    
    this->defineSynchedData();
}

MinecartChest::MinecartChest(Level* level, double x, double y, double z)
    : MinecartContainer(level, x, y, z) {
    
    
    this->defineSynchedData();
}


int MinecartChest::getContainerType() {
    return ContainerOpenPacket::MINECART_CHEST;
}

void MinecartChest::destroy(DamageSource* source) {
    MinecartContainer::destroy(source);

    spawnAtLocation(Tile::chest_Id, 1, 0);
}

unsigned int MinecartChest::getContainerSize() { return 9 * 3; }

int MinecartChest::getType() { return TYPE_CHEST; }

Tile* MinecartChest::getDefaultDisplayTile() { return Tile::chest; }

int MinecartChest::getDefaultDisplayOffset() { return 8; }