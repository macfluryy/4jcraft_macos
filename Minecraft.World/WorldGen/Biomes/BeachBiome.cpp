#include "../../Platform/stdafx.h"
#include "../../Headers/net.minecraft.world.level.tile.h"
#include "BiomeDecorator.h"
#include "BeachBiome.h"

BeachBiome::BeachBiome(int id) : Biome(id)
{
	// remove default mob spawn settings
	friendlies.clear();
	friendlies_chicken.clear();	// 4J added
	this->topMaterial = (uint8_t) Tile::sand_Id;
	this->material = (uint8_t) Tile::sand_Id;

	decorator->treeCount = -999;
	decorator->deadBushCount = 0;
	decorator->reedsCount = 0;
	decorator->cactusCount = 0;
}