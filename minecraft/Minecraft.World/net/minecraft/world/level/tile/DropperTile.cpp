#include <memory>
#include <string>

#include "DropperTile.h"
#include "Minecraft.World/net/minecraft/Facing.h"
#include "Minecraft.World/net/minecraft/core/BlockSourceImpl.h"
#include "Minecraft.World/net/minecraft/core/DefaultDispenseItemBehavior.h"
#include "Minecraft.World/net/minecraft/core/DispenseItemBehavior.h"
#include "Minecraft.World/net/minecraft/world/IconRegister.h"
#include "Minecraft.World/net/minecraft/world/item/ItemInstance.h"
#include "Minecraft.World/net/minecraft/world/level/Level.h"
#include "Minecraft.World/net/minecraft/world/level/tile/DispenserTile.h"
#include "Minecraft.World/net/minecraft/world/level/tile/LevelEvent.h"
#include "Minecraft.World/net/minecraft/world/level/tile/entity/DispenserTileEntity.h"
#include "Minecraft.World/net/minecraft/world/level/tile/entity/DropperTileEntity.h"
#include "Minecraft.World/net/minecraft/world/level/tile/entity/HopperTileEntity.h"

class Container;

DropperTile::DropperTile(int id) : DispenserTile(id) {
    DISPENSE_BEHAVIOUR = new DefaultDispenseItemBehavior();
}

void DropperTile::registerIcons(IconRegister* iconRegister) {
    icon = iconRegister->registerIcon(L"furnace_side");
    iconTop = iconRegister->registerIcon(L"furnace_top");
    iconFront =
        iconRegister->registerIcon(getIconName() + L"_front_horizontal");
    iconFrontVertical =
        iconRegister->registerIcon(getIconName() + L"_front_vertical");
}

DispenseItemBehavior* DropperTile::getDispenseMethod(
    std::shared_ptr<ItemInstance> item) {
    return DISPENSE_BEHAVIOUR;
}

std::shared_ptr<TileEntity> DropperTile::newTileEntity(Level* level) {
    return std::make_shared<DropperTileEntity>();
}

void DropperTile::dispenseFrom(Level* level, int x, int y, int z) {
    BlockSourceImpl source(level, x, y, z);
    std::shared_ptr<DispenserTileEntity> trap =
        std::dynamic_pointer_cast<DispenserTileEntity>(source.getEntity());
    if (trap == nullptr) return;

    int slot = trap->getRandomSlot();
    if (slot < 0) {
        level->levelEvent(LevelEvent::SOUND_CLICK_FAIL, x, y, z, 0);
    } else {
        std::shared_ptr<ItemInstance> item = trap->getItem(slot);
        int face = level->getData(x, y, z) & DispenserTile::FACING_MASK;
        std::shared_ptr<Container> into = HopperTileEntity::getContainerAt(
            level, x + Facing::STEP_X[face], y + Facing::STEP_Y[face],
            z + Facing::STEP_Z[face]);
        std::shared_ptr<ItemInstance> remaining = nullptr;

        if (into != nullptr) {
            remaining =
                HopperTileEntity::addItem(into.get(), item->copy()->remove(1),
                                          Facing::OPPOSITE_FACING[face]);

            if (remaining == nullptr) {
                remaining = item->copy();
                if (--remaining->count == 0) remaining = nullptr;
            } else {
                // placing one item failed, so restore original count
                remaining = item->copy();
            }
        } else {
            remaining = DISPENSE_BEHAVIOUR->dispense(&source, item);
            if (remaining != nullptr && remaining->count == 0)
                remaining = nullptr;
        }

        trap->setItem(slot, remaining);
    }
}