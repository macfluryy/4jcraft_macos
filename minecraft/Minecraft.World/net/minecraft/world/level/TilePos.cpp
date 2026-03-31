#include "../../../../Header Files/stdafx.h"

#include "TilePos.h"
#include "../phys/Vec3.h"

TilePos::TilePos(int x, int y, int z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

// 4J - brought forward from 1.2.3
TilePos::TilePos(Vec3* p) {
    this->x = GameMath::floor(p->x);
    this->y = GameMath::floor(p->y);
    this->z = GameMath::floor(p->z);
}

int TilePos::hash_fnct(const TilePos& k) {
    return k.x * 8976890 + k.y * 981131 + k.z;
}

bool TilePos::eq_test(const TilePos& x, const TilePos& y) {
    return x.x == y.x && x.y == y.y && x.z == y.z;
}