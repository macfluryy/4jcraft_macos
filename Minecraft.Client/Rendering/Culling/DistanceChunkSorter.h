#pragma once
class Entity;
class Chunk;

class DistanceChunkSorter
    : public std::binary_function<const Chunk*, const Chunk*, bool> {
private:
    double ix, iy, iz;

public:
    DistanceChunkSorter(std::shared_ptr<Entity> player);
    bool operator()(const Chunk* a, const Chunk* b) const;
};