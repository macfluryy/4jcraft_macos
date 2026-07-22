#include "MemoryChunkStorage.h"




LevelChunk* MemoryChunkStorage::load(Level* level, int x,
                                     int z)  
{
    return nullptr;
}

void MemoryChunkStorage::save(Level* level,
                              LevelChunk* levelChunk)  
{}

void MemoryChunkStorage::saveEntities(
    Level* level, LevelChunk* levelChunk)  
{}

void MemoryChunkStorage::tick() {}

void MemoryChunkStorage::flush() {}