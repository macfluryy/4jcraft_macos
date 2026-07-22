#include "TheEndBiomeDecorator.h"

#include <memory>

#include "java/Random.h"
#include "minecraft/world/entity/boss/enderdragon/EnderDragon.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/biome/BiomeDecorator.h"
#include "minecraft/world/level/levelgen/feature/EndPodiumFeature.h"
#include "minecraft/world/level/levelgen/feature/Feature.h"
#include "minecraft/world/level/levelgen/feature/SpikeFeature.h"
#include "minecraft/world/level/tile/Tile.h"

class Biome;








TheEndBiomeDecorator::SPIKE TheEndBiomeDecorator::SpikeValA[8] = {
    
    
    

    
    
    {32, -16, 40, 0, 2},      
    {16, 16, 28, 28, 2},      
    {-16, 32, 0, 40, 2},      
    {-32, 16, -28, 28, 3},    
    {-48, -16, -40, 0, 3},    
    {-32, -32, -28, -28, 3},  
    {-16, -48, 0, -40, 4},    
    {16, -32, 28, -28, 4},    
};

TheEndBiomeDecorator::TheEndBiomeDecorator(Biome* biome)
    : BiomeDecorator(biome) {
    spikeFeature = new SpikeFeature(Tile::endStone_Id);
    endPodiumFeature = new EndPodiumFeature(Tile::endStone_Id);
}

void TheEndBiomeDecorator::decorate() {
    decorateOres();

    
    int y = level->getTopSolidBlock(xo + 8, zo + 8);
    if (y > level->GetHighestY()) level->SetHighestY(y);

    

    
    for (int i = 0; i < 8; i++) {
        if ((xo == SpikeValA[i].iChunkX) && (zo == SpikeValA[i].iChunkZ)) {
            
            spikeFeature->placeWithIndex(level, random, SpikeValA[i].x,
                                         level->GetHighestY(), SpikeValA[i].z,
                                         i, SpikeValA[i].radius);
        }
    }
    if (xo == 0 && zo == 0) {
        std::shared_ptr<EnderDragon> enderDragon =
            std::make_shared<EnderDragon>(level);
        enderDragon->AddParts();  
        enderDragon->moveTo(0, 128, 0, random->nextFloat() * 360, 0);
        level->addEntity(enderDragon);
    }

    
    
    
    if (xo == -16 && zo == -16) {
        endPodiumFeature->place(level, random, 0, level->seaLevel, 0);
    }
}