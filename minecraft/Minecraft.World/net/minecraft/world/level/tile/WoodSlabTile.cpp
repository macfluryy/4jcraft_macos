#include "WoodSlabTile.h"
#include "WoodTile.h"
#include "TreeTile.h"
#include "Minecraft.World/net/minecraft/world/level/net.minecraft.world.level.h"
#include "Minecraft.World/net/minecraft/world/level/biome/net.minecraft.world.level.biome.h"
#include "Minecraft.World/net/minecraft/world/item/net.minecraft.world.item.h"
#include "Minecraft.World/net/minecraft/stats/net.minecraft.stats.h"

#include "strings.h"

const unsigned int WoodSlabTile::SLAB_NAMES[SLAB_NAMES_LENGTH] = {
    IDS_TILE_STONESLAB_OAK,
    IDS_TILE_STONESLAB_SPRUCE,
    IDS_TILE_STONESLAB_BIRCH,
    IDS_TILE_STONESLAB_JUNGLE,
};

// 	public static final String[] WOOD_NAMES = {
// 		"oak", "spruce", "birch", "jungle"
// 	};

WoodSlabTile::WoodSlabTile(int id, bool fullSize)
    : HalfSlabTile(id, fullSize, Material::wood) {}

Icon* WoodSlabTile::getTexture(int face, int data) {
    return Tile::wood->getTexture(face, data & TYPE_MASK);
}

int WoodSlabTile::getResource(int data, Random* random, int playerBonusLevel) {
    return Tile::woodSlabHalf_Id;
}

std::shared_ptr<ItemInstance> WoodSlabTile::getSilkTouchItemInstance(int data) {
    return std::shared_ptr<ItemInstance>(
        new ItemInstance(Tile::woodSlabHalf, 2, data & TYPE_MASK));
}

int WoodSlabTile::getAuxName(int auxValue) {
    if (auxValue < 0 || auxValue >= SLAB_NAMES_LENGTH) {
        auxValue = 0;
    }
    return SLAB_NAMES[auxValue];  // super.getDescriptionId() + "." +
                                  // SLAB_NAMES[auxValue];
}

void WoodSlabTile::registerIcons(IconRegister* iconRegister) {
    // None
}

unsigned int WoodSlabTile::getDescriptionId(int iData) {
    if (iData < 0 || iData >= SLAB_NAMES_LENGTH) {
        iData = 0;
    }
    return SLAB_NAMES[iData];
}