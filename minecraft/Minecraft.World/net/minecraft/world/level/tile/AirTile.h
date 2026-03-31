#pragma once
#include "Tile.h"
#include "Minecraft.World/net/minecraft/world/level/material/Material.h"

class AirTile : public Tile {
    friend class Tile;

protected:
    AirTile(int id);
};