#include "minecraft/world/level/ViewDistanceUtil.h"

#include <algorithm>






#include "minecraft/server/level/PlayerChunkMap.h"

int clampViewDistance(int chunks) {
    if (chunks < PlayerChunkMap::MIN_VIEW_DISTANCE) {
        return PlayerChunkMap::MIN_VIEW_DISTANCE;
    }
    if (chunks > PlayerChunkMap::MAX_VIEW_DISTANCE) {
        return PlayerChunkMap::MAX_VIEW_DISTANCE;
    }
    return chunks;
}

int viewDistanceOptionToChunks(int optionVD) {
    
    
    
    
    
    
    
    
    
    
    int option = optionVD;
    if (option < 0) option = 0;

    int chunks;
    if (option >= 31) {
        chunks = 0;
    } else {
        chunks = 16 >> option;
    }

    return clampViewDistance(chunks);
}

int effectiveViewDistance(int server, int client) {
    return clampViewDistance(std::min(server, client));
}
