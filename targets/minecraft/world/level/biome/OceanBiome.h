#pragma once

#include "Biome.h"

class OceanBiome : public Biome {
public:
    OceanBiome(int id) : Biome(id) {
        friendlies.clear();
        friendlies_chicken.clear();  
                                     
        friendlies_wolf
            .clear();  
    }
};
