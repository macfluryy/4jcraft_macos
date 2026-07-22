#include "TickNextTickData.h"

#include <cstdint>

#include "minecraft/world/level/tile/Tile.h"

int64_t TickNextTickData::C = 0;

TickNextTickData::TickNextTickData(int x, int y, int z, int tileId) {
    m_delay = 0;
    c = C++;

    this->x = x;
    this->y = y;
    this->z = z;
    this->tileId = tileId;
    priorityTilt = 0;
}

bool TickNextTickData::equals(const TickNextTickData* o) const {
    
    
    if (o != nullptr) {
        TickNextTickData* t = (TickNextTickData*)o;
        return x == t->x && y == t->y && z == t->z &&
               Tile::isMatching(tileId, t->tileId);
    }
    return false;
}

int TickNextTickData::hashCode() const {
    std::uint32_t hash = (((std::uint32_t)x * 1024u * 1024u) +
                          ((std::uint32_t)z * 1024u) + (std::uint32_t)y) *
                         256u;
    return (std::int32_t)hash;
}

TickNextTickData* TickNextTickData::delay(int64_t l) {
    m_delay = l;
    return this;
}

void TickNextTickData::setPriorityTilt(int priorityTilt) {
    this->priorityTilt = priorityTilt;
}

int TickNextTickData::compareTo(const TickNextTickData* tnd) const {
    if (m_delay < tnd->m_delay) return -1;
    if (m_delay > tnd->m_delay) return 1;
    if (priorityTilt != tnd->priorityTilt)
        return priorityTilt - tnd->priorityTilt;
    if (c < tnd->c) return -1;
    if (c > tnd->c) return 1;
    return 0;
}

bool TickNextTickData::operator==(const TickNextTickData& k) {
    return equals(&k);
}








bool TickNextTickData::compare_fnct(const TickNextTickData& x,
                                    const TickNextTickData& y) {
    return x.compareTo(&y) < 0;
}

int TickNextTickData::hash_fnct(const TickNextTickData& k) {
    return k.hashCode();
}

bool TickNextTickData::eq_test(const TickNextTickData& x,
                               const TickNextTickData& y) {
    return x.equals(&y);
}
