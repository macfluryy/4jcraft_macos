#pragma once
#include "Biome.h"
#include "minecraft/world/level/biome/Biome.h"

class LevelSource;

class SwampBiome : public Biome {
    
    
public:
    SwampBiome(int id);

public:
    virtual Feature* getTreeFeature(Random* random);

    
    
    
};