#include "../../../../../../Minecraft.World/Header Files/stdafx.h"
#include "MinecartSpawnerRenderer.h"
#include "../../../../../../Minecraft.World/net/minecraft/world/level/tile/Tile.h"
#include "../../../../../../Minecraft.World/net/minecraft/world/entity/item/net.minecraft.world.entity.item.h"
#include "../tileentity/MobSpawnerRenderer.h"

void MinecartSpawnerRenderer::renderMinecartContents(
    std::shared_ptr<MinecartSpawner> cart, float a, Tile* tile, int tileData) {
    MinecartRenderer::renderMinecartContents(cart, a, tile, tileData);

    if (tile == Tile::mobSpawner) {
        MobSpawnerRenderer::render(cart->getSpawner(), cart->x, cart->y,
                                   cart->z, a);
    }
}