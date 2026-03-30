#include "../../../../Header Files/stdafx.h"
#include "../entity/player/net.minecraft.world.entity.player.h"
#include "../entity/net.minecraft.world.entity.h"
#include "net.minecraft.world.damagesource.h"
#include "../../network/packet/net.minecraft.network.packet.h"

// IndirectEntityDamageSource::IndirectEntityDamageSource(const wstring &msgId,
// shared_ptr<Entity> entity, shared_ptr<Entity> owner) :
// EntityDamageSource(msgId, entity)
IndirectEntityDamageSource::IndirectEntityDamageSource(
    ChatPacket::EChatPacketMessage msgId,
    ChatPacket::EChatPacketMessage msgWithItemId,
    std::shared_ptr<Entity> entity, std::shared_ptr<Entity> owner)
    : EntityDamageSource(msgId, msgWithItemId, entity) {
    this->owner = owner;
}

// 4J Stu - Brought forward from 1.2.3 to fix #46422
std::shared_ptr<Entity> IndirectEntityDamageSource::getDirectEntity() {
    return entity;
}

std::shared_ptr<Entity> IndirectEntityDamageSource::getEntity() {
    return owner;
}

// wstring
// IndirectEntityDamageSource::getLocalizedDeathMessage(shared_ptr<Player>
// player)
//{
//	return L"death." + msgId + player->name + owner->getAName();
//	//return I18n.get("death." + msgId, player.name, owner.getAName());
// }

std::shared_ptr<ChatPacket> IndirectEntityDamageSource::getDeathMessagePacket(
    std::shared_ptr<LivingEntity> player) {
    std::shared_ptr<ItemInstance> held =
        entity->instanceof(eTYPE_LIVINGENTITY)
            ? std::dynamic_pointer_cast<LivingEntity>(entity)->getCarriedItem()
            : nullptr;
    std::wstring additional = L"";
    int type;
    if (owner != nullptr) {
        type = owner->GetType();
        if (type == eTYPE_SERVERPLAYER) {
            std::shared_ptr<Player> sourcePlayer =
                std::dynamic_pointer_cast<Player>(owner);
            if (sourcePlayer != nullptr) additional = sourcePlayer->name;
        }
    } else {
        type = entity->GetType();
    }
    if (held != nullptr && held->hasCustomHoverName()) {
        return std::shared_ptr<ChatPacket>(
            new ChatPacket(player->getNetworkName(), m_msgWithItemId, type,
                           additional, held->getHoverName()));
    } else {
        return std::shared_ptr<ChatPacket>(new ChatPacket(
            player->getNetworkName(), m_msgId, type, additional));
    }
}

// 4J: Copy function
DamageSource* IndirectEntityDamageSource::copy() {
    return new IndirectEntityDamageSource(*this);
}