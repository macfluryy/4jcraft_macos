#include "../Platform/stdafx.h"
#include "../Headers/net.minecraft.world.entity.player.h"
#include "../Headers/net.minecraft.world.entity.h"
#include "../Headers/net.minecraft.world.damagesource.h"
#include "../Headers/net.minecraft.network.packet.h"

//EntityDamageSource::EntityDamageSource(const std::wstring &msgId, std::shared_ptr<Entity> entity) : DamageSource(msgId)
EntityDamageSource::EntityDamageSource(ChatPacket::EChatPacketMessage msgId, std::shared_ptr<Entity> entity) : DamageSource(msgId)
{
	this->entity = entity;
}

std::shared_ptr<Entity> EntityDamageSource::getEntity()
{
	return entity;
}

//std::wstring EntityDamageSource::getLocalizedDeathMessage(std::shared_ptr<Player> player)
//{
//	return L"death." + msgId + player->name + entity->getAName();
//	//return I18n.get("death." + msgId, player.name, entity.getAName());
//}

std::shared_ptr<ChatPacket> EntityDamageSource::getDeathMessagePacket(std::shared_ptr<Player> player)
{
	std::wstring additional = L"";
	if(entity->GetType() == eTYPE_SERVERPLAYER)
	{
		std::shared_ptr<Player> sourcePlayer = std::dynamic_pointer_cast<Player>(entity);
		if(sourcePlayer != NULL) additional = sourcePlayer->name;
	}
	return std::shared_ptr<ChatPacket>( new ChatPacket(player->name, m_msgId, entity->GetType(), additional ) );
}

bool EntityDamageSource::scalesWithDifficulty()
{
	return entity != NULL && std::dynamic_pointer_cast<Mob>(entity) && !(std::dynamic_pointer_cast<Player>(entity));
}