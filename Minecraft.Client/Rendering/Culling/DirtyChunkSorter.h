#pragma once
class Chunk;
class Mob;

class DirtyChunkSorter
    : public std::binary_function<const Chunk*, const Chunk*, bool> {
private:
    std::shared_ptr<LivingEntity> cameraEntity;
    int playerIndex;  // 4J added

public:
    DirtyChunkSorter(std::shared_ptr<LivingEntity> cameraEntity,
                     int playerIndex);  // 4J - added player index
    bool operator()(const Chunk* a, const Chunk* b) const;
};