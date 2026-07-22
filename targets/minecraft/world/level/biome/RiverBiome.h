#pragma once

#include "Biome.h"

class RiverBiome : public Biome {
public:
    RiverBiome(int id) : Biome(id) {
        friendlies.clear();
        friendlies_chicken.clear();  
                                     
        friendlies_wolf
            .clear();  
    }
};