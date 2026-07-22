#include "FlowerFeature.h"

#include "app/common/src/GameRules/LevelGeneration/LevelGenerationOptions.h"
#include "app/mac/MacGame.h"
#include "java/Random.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/dimension/Dimension.h"
#include "minecraft/world/level/tile/Tile.h"

FlowerFeature::FlowerFeature(int tile) { this->tile = tile; }

bool FlowerFeature::place(Level* level, Random* random, int x, int y, int z) {
    
    
    if (app.getLevelGenerationOptions() != nullptr) {
        LevelGenerationOptions* levelGenOptions =
            app.getLevelGenerationOptions();
        bool intersects = levelGenOptions->checkIntersects(x - 8, y - 4, z - 8,
                                                           x + 8, y + 4, z + 8);
        if (intersects) {
            
            
            return false;
        }
    }

    for (int i = 0; i < 64; i++) {
        int x2 = x + random->nextInt(8) - random->nextInt(8);
        int y2 = y + random->nextInt(4) - random->nextInt(4);
        int z2 = z + random->nextInt(8) - random->nextInt(8);
        if (level->isEmptyTile(x2, y2, z2) &&
            (!level->dimension->hasCeiling || y2 < Level::genDepthMinusOne)) {
            if (Tile::tiles[tile]->canSurvive(level, x2, y2, z2)) {
                level->setTileAndData(x2, y2, z2, tile, 0,
                                      Tile::UPDATE_CLIENTS);
            }
        }
    }

    return true;
}