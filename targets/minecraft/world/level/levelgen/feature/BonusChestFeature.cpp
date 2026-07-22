#include "BonusChestFeature.h"

#include <memory>

#include "java/Random.h"
#include "minecraft/util/WeighedTreasure.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/tile/Tile.h"
#include "minecraft/world/level/tile/entity/ChestTileEntity.h"

BonusChestFeature::BonusChestFeature(
    std::vector<WeighedTreasure*>& treasureList, int numRolls)
    : treasureList(treasureList), numRolls(numRolls) {}


bool BonusChestFeature::place(Level* level, Random* random, int x, int y,
                              int z) {
    return place(level, random, x, y, z, false);
}










bool BonusChestFeature::place(Level* level, Random* random, int x, int y, int z,
                              bool force) {
    if (!force) {
        int t = 0;
        while (((t = level->getTile(x, y, z)) == 0 || t == Tile::leaves_Id) &&
               y > 1)
            y--;

        if (y < 1) {
            return false;
        }
        y++;
    }

    for (int i = 0; i < 4; i++) {
        int x2, y2, z2;

        if (force) {
            x2 = x;
            y2 = y - 1;  
                         
                         
                         
            
            
            
            z2 = z;
        } else {
            x2 = x + random->nextInt(4) - random->nextInt(4);
            y2 = y + random->nextInt(3) - random->nextInt(3);
            z2 = z + random->nextInt(4) - random->nextInt(4);
        }

        if (force || (level->isEmptyTile(x2, y2, z2) &&
                      level->isTopSolidBlocking(x2, y2 - 1, z2))) {
            level->setTileAndData(x2, y2, z2, Tile::chest_Id, 0,
                                  Tile::UPDATE_CLIENTS);
            std::shared_ptr<ChestTileEntity> chest =
                std::dynamic_pointer_cast<ChestTileEntity>(
                    level->getTileEntity(x2, y2, z2));
            if (chest != nullptr) {
                WeighedTreasure::addChestItems(random, treasureList, chest,
                                               numRolls);
                chest->isBonusChest = true;  
            }
            if (level->isEmptyTile(x2 - 1, y2, z2) &&
                level->isTopSolidBlocking(x2 - 1, y2 - 1, z2)) {
                level->setTileAndData(x2 - 1, y2, z2, Tile::torch_Id, 0,
                                      Tile::UPDATE_CLIENTS);
            }
            if (level->isEmptyTile(x2 + 1, y2, z2) &&
                level->isTopSolidBlocking(x2 - 1, y2 - 1, z2)) {
                level->setTileAndData(x2 + 1, y2, z2, Tile::torch_Id, 0,
                                      Tile::UPDATE_CLIENTS);
            }
            if (level->isEmptyTile(x2, y2, z2 - 1) &&
                level->isTopSolidBlocking(x2 - 1, y2 - 1, z2)) {
                level->setTileAndData(x2, y2, z2 - 1, Tile::torch_Id, 0,
                                      Tile::UPDATE_CLIENTS);
            }
            if (level->isEmptyTile(x2, y2, z2 + 1) &&
                level->isTopSolidBlocking(x2 - 1, y2 - 1, z2)) {
                level->setTileAndData(x2, y2, z2 + 1, Tile::torch_Id, 0,
                                      Tile::UPDATE_CLIENTS);
            }
            return true;
        }
    }

    return false;
}
