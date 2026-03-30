#include "../../../../Header Files/stdafx.h"
#include "../../network/packet/net.minecraft.network.packet.h"
#include "../entity/player/net.minecraft.world.entity.player.h"
#include "../level/net.minecraft.world.level.h"
#include "ComplexItem.h"

ComplexItem::ComplexItem(int id) : Item(id) {}

bool ComplexItem::isComplex() { return true; }

std::shared_ptr<Packet> ComplexItem::getUpdatePacket(
    std::shared_ptr<ItemInstance> itemInstance, Level* level,
    std::shared_ptr<Player> player) {
    return nullptr;
}