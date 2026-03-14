#include "../../Platform/stdafx.h"
#include "../../Headers/net.minecraft.world.level.levelgen.structure.h"

MineShaftStart::MineShaftStart(Level* level, Random* random, int chunkX,
                               int chunkZ) {
    // 4jcraft added to unsigned
    MineShaftPieces::MineShaftRoom* mineShaftRoom =
        new MineShaftPieces::MineShaftRoom(0, random,
                                           ((unsigned)chunkX << 4) + 2,
                                           ((unsigned)chunkZ << 4) + 2);
    pieces.push_back(mineShaftRoom);
    mineShaftRoom->addChildren(mineShaftRoom, &pieces, random);

    calculateBoundingBox();
    moveBelowSeaLevel(level, random, 10);
}
