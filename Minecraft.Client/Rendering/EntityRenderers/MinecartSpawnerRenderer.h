#pragma once
#include "MinecartRenderer.h"

class MinecartSpawner;

class MinecartSpawnerRenderer : public MinecartRenderer {
protected:
    void renderMinecartContents(std::shared_ptr<MinecartSpawner> cart, float a,
                                Tile* tile, int tileData);
};