#include "SpringFeature.h"

#include "app/common/src/GameRules/LevelGeneration/LevelGenerationOptions.h"
#include "app/mac/MacGame.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/LevelType.h"
#include "minecraft/world/level/storage/LevelData.h"
#include "minecraft/world/level/tile/Tile.h"

SpringFeature::SpringFeature(int tile) { this->tile = tile; }

bool SpringFeature::place(Level* level, Random* random, int x, int y, int z) {
    // 4J Stu Added to stop spring features generating areas previously place by
    // game rule generation
    if (app.getLevelGenerationOptions() != nullptr) {
        LevelGenerationOptions* levelGenOptions =
            app.getLevelGenerationOptions();
        bool intersects = levelGenOptions->checkIntersects(x, y, z, x, y, z);
        if (intersects) {
            // app.DebugPrintf("Skipping spring feature generation as it
            // overlaps a game rule structure\n");
            return false;
        }
    }

    // 4J macOS - Amplified spring placement guard. The vanilla check
    // below (3 stone neighbours + 1 air) is enough on flat terrain
    // but on amplified maps the spring can land inside a 1-block-
    // thin cliff face, leaving the source exposed to open sky after
    // surface / cave carving. The result is a floating water cube
    // hanging off a ridge, with no flow volume.
    //
    // We add two extra requirements ONLY for amplified worlds:
    //   1. The 5x5 horizontal ring around the source must contain
    //      at least 14 stone blocks (out of 24) - this means the
    //      spring is genuinely embedded in a rock body, not sitting
    //      on the edge of a cliff.
    //   2. The column directly above must stay solid (or at least
    //      non-empty) for the next 8 blocks - otherwise the source
    //      is exposed to open sky from above.
    //
    // Vanilla / largeBiomes / flat / triple keep the original spring
    // logic untouched.
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