#include "../../../../../Header Files/stdafx.h"
#include "../../entity/item/net.minecraft.world.entity.item.h"
#include "../net.minecraft.world.level.h"
#include "../redstone/net.minecraft.world.level.redstone.h"
#include "../../item/net.minecraft.world.item.h"
#include "../../entity/Entity.h"
#include "WeightedPressurePlateTile.h"
#include "../../phys/AABB.h"

WeightedPressurePlateTile::WeightedPressurePlateTile(int id,
                                                     const std::wstring& tex,
                                                     Material* material,
                                                     int maxWeight)
    : BasePressurePlateTile(id, tex, material) {
    this->maxWeight = maxWeight;

    // 4J Stu - Move this from base class to use virtual function
    updateShape(getDataForSignal(Redstone::SIGNAL_MAX));
}

int WeightedPressurePlateTile::getSignalStrength(Level* level, int x, int y,
                                                 int z) {
    AABB at_bb = getSensitiveAABB(x, y, z);
    int weightOfEntities =
        level->getEntitiesOfClass(typeid(Entity), &at_bb)->size();
    int count = std::min(weightOfEntities, maxWeight);

    if (count <= 0) {
        return 0;
    } else {
        float pct = std::min(maxWeight, count) / (float)maxWeight;
        return Mth::ceil(pct * Redstone::SIGNAL_MAX);
    }
}

int WeightedPressurePlateTile::getSignalForData(int data) { return data; }

int WeightedPressurePlateTile::getDataForSignal(int signal) { return signal; }

int WeightedPressurePlateTile::getTickDelay(Level* level) {
    return SharedConstants::TICKS_PER_SECOND / 2;
}
