#include "SpringFeature.h"

#include "app/common/src/GameRules/LevelGeneration/LevelGenerationOptions.h"
#include "app/mac/MacGame.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/LevelType.h"
#include "minecraft/world/level/storage/LevelData.h"
#include "minecraft/world/level/tile/Tile.h"

SpringFeature::SpringFeature(int tile) { this->tile = tile; }

bool SpringFeature::place(Level* level, Random* random, int x, int y, int z) {
    
    
    if (app.getLevelGenerationOptions() != nullptr) {
        LevelGenerationOptions* levelGenOptions =
            app.getLevelGenerationOptions();
        bool intersects = levelGenOptions->checkIntersects(x, y, z, x, y, z);
        if (intersects) {
            
            
            return false;
        }
    }

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    if (level->getLevelData() != nullptr &&
        level->getLevelData()->getGenerator() == LevelType::lvl_amplified) {
        int solidRing = 0;
        for (int dx = -2; dx <= 2; dx++) {
            for (int dz = -2; dz <= 2; dz++) {
                if (dx == 0 && dz == 0) continue;
                if (level->getTile(x + dx, y, z + dz) == Tile::stone_Id) {
                    solidRing++;
                }
            }
        }
        if (solidRing < 14) return false;

        for (int dy = 1; dy <= 8; dy++) {
            if (level->isEmptyTile(x, y + dy, z)) {
                return false;
            }
        }
    }

    if (level->getTile(x, y + 1, z) != Tile::stone_Id) return false;
    if (level->getTile(x, y - 1, z) != Tile::stone_Id) return false;

    if (level->getTile(x, y, z) != 0 &&
        level->getTile(x, y, z) != Tile::stone_Id)
        return false;

    int rockCount = 0;
    if (level->getTile(x - 1, y, z) == Tile::stone_Id) rockCount++;
    if (level->getTile(x + 1, y, z) == Tile::stone_Id) rockCount++;
    if (level->getTile(x, y, z - 1) == Tile::stone_Id) rockCount++;
    if (level->getTile(x, y, z + 1) == Tile::stone_Id) rockCount++;

    int holeCount = 0;
    if (level->isEmptyTile(x - 1, y, z)) holeCount++;
    if (level->isEmptyTile(x + 1, y, z)) holeCount++;
    if (level->isEmptyTile(x, y, z - 1)) holeCount++;
    if (level->isEmptyTile(x, y, z + 1)) holeCount++;

    if (rockCount == 3 && holeCount == 1) {
        level->setTileAndData(x, y, z, tile, 0, Tile::UPDATE_CLIENTS);
        level->setInstaTick(true);
        Tile::tiles[tile]->tick(level, x, y, z, random);
        level->setInstaTick(false);
    }

    return true;
}