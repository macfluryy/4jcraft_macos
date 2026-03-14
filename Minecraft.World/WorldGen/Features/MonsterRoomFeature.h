#pragma once
#include "Feature.h"
#include "../../Blocks/Material.h"

class MonsterRoomFeature : public Feature {
private:
    // int tile;

public:
    virtual bool place(Level* level, Random* random, int x, int y, int z);

private:
    std::shared_ptr<ItemInstance> randomItem(Random* random);
    std::wstring randomEntityId(Random* random);
};
