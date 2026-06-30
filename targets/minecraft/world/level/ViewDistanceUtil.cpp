#include "minecraft/world/level/ViewDistanceUtil.h"

#include <algorithm>

// Use PlayerChunkMap's authoritative MIN/MAX_VIEW_DISTANCE constants directly
// rather than mirroring them, so the bounds can never drift out of sync.
// PlayerChunkMap.h is lightweight (it only forward-declares the heavy server
// types and pulls ChunkPos / JavaIntHash), so including it here does not leak
// server dependencies into callers of ViewDistanceUtil.h.
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
    // GameRenderer::setupCamera: renderDistance(blocks) = 16 * 16 >> viewDistance
    // -> chunks = renderDistance / 16 = 16 >> viewDistance
    //   option 0 (Far)    -> 16 chunks
    //   option 1 (Normal) ->  8 chunks
    //   option 2 (Short)  ->  4 chunks
    //   option 3 (Tiny)   ->  2 chunks
    // Guard the shift against out-of-range / negative option values to avoid
    // undefined behaviour: a negative option means "more than Far" (treated as
    // 0), and an option large enough to shift past the value width is treated
    // as 0 chunks (which then clamps up to MIN_VIEW_DISTANCE).
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
