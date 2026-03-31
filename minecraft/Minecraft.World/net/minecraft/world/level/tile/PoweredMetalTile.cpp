#include "PoweredMetalTile.h"
#include "Minecraft.World/net/minecraft/world/level/redstone/Redstone.h"
#include "Minecraft.World/net/minecraft/world/level/tile/MetalTile.h"

PoweredMetalTile::PoweredMetalTile(int id) : MetalTile(id) {}

bool PoweredMetalTile::isSignalSource() { return true; }

int PoweredMetalTile::getSignal(LevelSource* level, int x, int y, int z,
                                int dir) {
    return Redstone::SIGNAL_MAX;
}