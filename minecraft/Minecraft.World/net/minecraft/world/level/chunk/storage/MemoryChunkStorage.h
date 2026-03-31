#pragma once

#include "Minecraft.World/net/minecraft/world/level/chunk/ChunkSource.h"

class MemoryChunkStorage : public ChunkStorage {
public:
    virtual LevelChunk* load(Level* level, int x, int z);
    virtual void save(Level* level, LevelChunk* levelChunk);
    virtual void saveEntities(Level* level, LevelChunk* levelChunk);
    virtual void tick();
    virtual void flush();
};