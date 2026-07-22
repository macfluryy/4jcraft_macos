#include "SandFeature.h"

#include "app/common/src/GameRules/LevelGeneration/LevelGenerationOptions.h"
#include "app/mac/MacGame.h"
#include "java/Random.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/material/Material.h"
#include "minecraft/world/level/tile/Tile.h"

SandFeature::SandFeature(int radius, int tile) {
    this->tile = tile;
    this->radius = radius;
}

bool SandFeature::place(Level* level, Random* random, int x, int y, int z) {
    if (level->getMaterial(x, y, z) != Material::water) return false;

    
    
    
    
    level->setInstaTick(true);

    int r = random->nextInt(radius - 2) + 2;
    int yr = 2;

    
    
    if (app.getLevelGenerationOptions() != nullptr) {
        LevelGenerationOptions* levelGenOptions =
            app.getLevelGenerationOptions();
        bool intersects = levelGenOptions->checkIntersects(
            x - r, y - yr, z - r, x + r, y + yr, z + r);
        if (intersects) {
            level->setInstaTick(false);
            
            
            return false;
        }
    }

    for (int xx = x - r; xx <= x + r; xx++) {
        for (int zz = z - r; zz <= z + r; zz++) {
            int xd = xx - x;
            int zd = zz - z;
            if (xd * xd + zd * zd > r * r) continue;
            for (int yy = y - yr; yy <= y + yr; yy++) {
                int t = level->getTile(xx, yy, zz);
                if (t == Tile::dirt_Id || t == Tile::grass_Id) {
                    level->setTileAndData(xx, yy, zz, tile, 0,
                                          Tile::UPDATE_CLIENTS);
                }
            }
        }
    }

    level->setInstaTick(false);

    return true;
}