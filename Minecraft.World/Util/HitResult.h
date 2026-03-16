#pragma once
#include "Vec3.h"

class HitResult {
public:
    enum Type { TILE, ENTITY };

    Type type;
    int x, y, z, f;
    Vec3* pos;
    std::shared_ptr<Entity> entity;

    HitResult(int x, int y, int z, int f, Vec3* pos);

    HitResult(std::shared_ptr<Entity> entity);

    double distanceTo(std::shared_ptr<Entity> e);
};