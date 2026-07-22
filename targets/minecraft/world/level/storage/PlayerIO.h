#pragma once

#include "nbt/CompoundTag.h"




#define MAX_PLAYER_DATA_SAVES 80

class Player;

class PlayerIO {
public:
    virtual void save(std::shared_ptr<Player> player) = 0;
    virtual CompoundTag* load(std::shared_ptr<Player> player) = 0;
    virtual CompoundTag* loadPlayerDataTag(
        PlayerUID xuid) = 0;  

    
    virtual void clearOldPlayerFiles() = 0;
    virtual void saveMapIdLookup() = 0;
    virtual void deleteMapFilesForPlayer(std::shared_ptr<Player> player) = 0;
    virtual void saveAllCachedData() = 0;
};