#include "StoneMonsterTile.h"

#include <memory>

#include "app/mac/MacGame.h"
#include "java/Class.h"
#include "minecraft/world/entity/monster/Silverfish.h"
#include "minecraft/world/item/ItemInstance.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/material/Material.h"
#include "minecraft/world/level/tile/FireTile.h"
#include "minecraft/world/level/tile/Tile.h"
#include "strings.h"

const unsigned int
    StoneMonsterTile::STONE_MONSTER_NAMES[STONE_MONSTER_NAMES_LENGTH] = {
        IDS_TILE_STONE_SILVERFISH,
        IDS_TILE_STONE_SILVERFISH_COBBLESTONE,
        IDS_TILE_STONE_SILVERFISH_STONE_BRICK,
};

StoneMonsterTile::StoneMonsterTile(int id) : Tile(id, Material::clay) {
    setDestroyTime(0);
}

Icon* StoneMonsterTile::getTexture(int face, int data) {
#ifndef _CONTENT_PACKAGE
    if (app.DebugArtToolsOn()) {
        return Tile::fire->getTexture(face, 0);
    }
#endif
    if (data == HOST_COBBLE) {
        return Tile::cobblestone->getTexture(face);
    }
    if (data == HOST_STONEBRICK) {
        return Tile::stoneBrick->getTexture(face);
    }
    return Tile::stone->getTexture(face);
}

void StoneMonsterTile::registerIcons(IconRegister* iconRegister) {
    
}

void StoneMonsterTile::destroy(Level* level, int x, int y, int z, int data) {
    if (!level->isClientSide) {
        
        
        
        if (level->countInstanceOf(eTYPE_MONSTER, false) < 70) {
            
            if (level->countInstanceOf(eTYPE_SILVERFISH, true) < 15) {
                std::shared_ptr<Silverfish> silverfish =
                    std::make_shared<Silverfish>(level);
                silverfish->moveTo(x + .5, y, z + .5, 0, 0);
                level->addEntity(silverfish);

                silverfish->spawnAnim();
            }
        }
    }
    Tile::destroy(level, x, y, z, data);
}

int StoneMonsterTile::getResourceCount(Random* random) { return 0; }

bool StoneMonsterTile::isCompatibleHostBlock(int block) {
    return block == Tile::stone_Id || block == Tile::cobblestone_Id ||
           block == Tile::stoneBrick_Id;
}

int StoneMonsterTile::getDataForHostBlock(int block) {
    if (block == Tile::cobblestone_Id) {
        return HOST_COBBLE;
    }
    if (block == Tile::stoneBrick_Id) {
        return HOST_STONEBRICK;
    }
    return HOST_ROCK;
}

Tile* StoneMonsterTile::getHostBlockForData(int data) {
    switch (data) {
        case HOST_COBBLE:
            return Tile::cobblestone;
        case HOST_STONEBRICK:
            return Tile::stoneBrick;
        default:
            return Tile::stone;
    }
}

std::shared_ptr<ItemInstance> StoneMonsterTile::getSilkTouchItemInstance(
    int data) {
    Tile* tile = Tile::stone;
    if (data == HOST_COBBLE) {
        tile = Tile::cobblestone;
    }
    if (data == HOST_STONEBRICK) {
        tile = Tile::stoneBrick;
    }
    return std::make_shared<ItemInstance>(tile);
}

int StoneMonsterTile::cloneTileData(Level* level, int x, int y, int z) {
    return level->getData(x, y, z);
}

unsigned int StoneMonsterTile::getDescriptionId(int iData ) {
    if (iData < 0) iData = 0;
    return StoneMonsterTile::STONE_MONSTER_NAMES[iData];
}