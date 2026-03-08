#pragma once


#include "EntityDamageSource.h"

class Entity;
class Player;

class IndirectEntityDamageSource : public EntityDamageSource
{
private:
	std::shared_ptr<Entity> owner;

public:
	//IndirectEntityDamageSource(const std::wstring &msgId, std::shared_ptr<Entity> entity, std::shared_ptr<Entity> owner);
	IndirectEntityDamageSource(ChatPacket::EChatPacketMessage msgId, std::shared_ptr<Entity> entity, std::shared_ptr<Entity> owner);
	virtual ~IndirectEntityDamageSource() { 	}

	virtual std::shared_ptr<Entity> getDirectEntity(); // 4J Stu - Brought forward from 1.2.3 to fix #46422
	virtual std::shared_ptr<Entity> getEntity();

	// 4J Stu - Made return a packet
	//virtual std::wstring getLocalizedDeathMessage(std::shared_ptr<Player> player);
	virtual std::shared_ptr<ChatPacket> getDeathMessagePacket(std::shared_ptr<Player> player);
};