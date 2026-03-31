#include "MinecartSpawnerRenderer.h"
#include "Minecraft.World/net/minecraft/world/level/tile/Tile.h"
#include "Minecraft.Client/net/minecraft/client/renderer/tileentity/MobSpawnerRenderer.h"
#include "Minecraft.World/net/minecraft/world/entity/item/MinecartSpawner.h"
#include "Minecraft.Client/net/minecraft/client/renderer/entity/MinecartRenderer.h"

void MinecartSpawnerRenderer::renderMinecartContents(
    std::shared_ptr<MinecartSpawner> cart, float a, Tile* tile, int tileData) {
    MinecartRenderer::renderMinecartContents(cart, a, tile, tileData);

    if (tile == Tile::mobSpawner) {
        MobSpawnerRenderer::render(cart->getSpawner(), cart->x, cart->y,
                                   cart->z, a);
    }
}