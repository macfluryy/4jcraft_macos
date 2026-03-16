#include "../Platform/stdafx.h"
#include "../Headers/net.minecraft.world.entity.player.h"
#include "../Headers/net.minecraft.world.entity.h"
#include "../Headers/net.minecraft.world.damagesource.h"
#include "../Headers/net.minecraft.network.packet.h"

// IndirectEntityDamageSource::IndirectEntityDamageSource(const std::wstring
// &msgId, std::shared_ptr<Entity> entity, std::shared_ptr<Entity> owner) :
// EntityDamageSource(msgId, entity)
IndirectEntityDamageSource::IndirectEntityDamageSource(
    ChatPacket::EChatPacketMessage msgId, std::shared_ptr<Entity> entity,
    std::shared_ptr<Entity> owner)
    : EntityDamageSource(msgId, entity) {
    this->owner = owner;
}

// 4J Stu - Brought forward from 1.2.3 to fix #46422
std::shared_ptr<Entity> IndirectEntityDamageSource::getDirectEntity() {
    return entity;
}

std::shared_ptr<Entity> IndirectEntityDamageSource::getEntity() {
    return owner;
}

// std::wstring
// IndirectEntityDamageSource::getLocalizedDeathMessage(std::shared_ptr<Player>
// player)
//{
//	return L"death." + msgId + player->name + owner->getAName();
//	//return I18n.get("death." + msgId, player.name, owner.getAName());
// }

std::shared_ptr<ChatPacket> IndirectEntityDamageSource::getDeathMessagePacket(
    std::shared_ptr<Player> player) {
    std::wstring additional = L"";
    int type;
    if (owner != NULL) {
        type = owner->GetType();
        if (type == eTYPE_SERVERPLAYER) {
            std::shared_ptr<Player> sourcePlayer =
                std::dynamic_pointer_cast<Player>(owner);
            if (sourcePlayer != NULL) additional = sourcePlayer->name;
        }
    } else {
        type = entity->GetType();
    }
    return std::shared_ptr<ChatPacket>(
        new ChatPacket(player->name, m_msgId, type, additional));
}