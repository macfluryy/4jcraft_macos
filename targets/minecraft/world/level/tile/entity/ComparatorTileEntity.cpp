#include "ComparatorTileEntity.h"

#include <memory>
#include <string>

#include "nbt/CompoundTag.h"
#include "minecraft/world/level/tile/entity/TileEntity.h"

void ComparatorTileEntity::save(CompoundTag* tag) {
    TileEntity::save(tag);
    tag->putInt(L"OutputSignal", output);
}

void ComparatorTileEntity::load(CompoundTag* tag) {
    TileEntity::load(tag);
    output = tag->getInt(L"OutputSignal");
}

int ComparatorTileEntity::getOutputSignal() { return output; }

void ComparatorTileEntity::setOutputSignal(int value) { output = value; }

// 4J Added
std::shared_ptr<TileEntity> ComparatorTileEntity::clone() {
    std::shared_ptr<ComparatorTileEntity> result =
        std::make_shared<ComparatorTileEntity>();
    TileEntity::clone(result);

    result->output = output;

    return result;
}