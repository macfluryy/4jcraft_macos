#include "EndPodiumFeature.h"

#include <cmath>

#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/tile/Tile.h"

EndPodiumFeature::EndPodiumFeature(int tile) {
    this->tile = tile;
    
}

bool EndPodiumFeature::place(Level* level, Random* random, int x, int y,
                             int z) {
    
    int r = 4;

    for (int yy = y - 1; yy <= y + 32; yy++) {
        for (int xx = x - r; xx <= x + r; xx++) {
            for (int zz = z - r; zz <= z + r; zz++) {
                double xd = xx - x;
                double zd = zz - z;
                double d = sqrt(xd * xd + zd * zd);
                if (d <= r - 0.5) {
                    if (yy < y) {
                        if (d > r - 1 - 0.5) {
                        } else {
                            
                            placeBlock(level, xx, yy, zz, Tile::unbreakable_Id,
                                       0);
                        }
                    } else if (yy > y) {
                        
                        placeBlock(level, xx, yy, zz, 0, 0);
                    } else {
                        if (d > r - 1 - 0.5) {
                            
                            placeBlock(level, xx, yy, zz, Tile::unbreakable_Id,
                                       0);
                        }
                    }
                }
            }
        }
    }

    placeBlock(level, x, y + 0, z, Tile::unbreakable_Id, 0);
    placeBlock(level, x, y + 1, z, Tile::unbreakable_Id, 0);
    placeBlock(level, x, y + 2, z, Tile::unbreakable_Id, 0);
    placeBlock(level, x - 1, y + 2, z, Tile::torch_Id, 0);
    placeBlock(level, x + 1, y + 2, z, Tile::torch_Id, 0);
    placeBlock(level, x, y + 2, z - 1, Tile::torch_Id, 0);
    placeBlock(level, x, y + 2, z + 1, Tile::torch_Id, 0);
    placeBlock(level, x, y + 3, z, Tile::unbreakable_Id, 0);
    

    
    
    for (int yy = y - 5; yy < y - 1; yy++) {
        for (int xx = x - (r - 1); xx <= x + (r - 1); xx++) {
            for (int zz = z - (r - 1); zz <= z + (r - 1); zz++) {
                if (level->isEmptyTile(xx, yy, zz)) {
                    placeBlock(level, xx, yy, zz, Tile::endStone_Id, 0);
                }
            }
        }
    }

    return true;
}
