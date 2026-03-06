#pragma once


#include "../Util/DamageSource.h"

class Entity;
class Player;

class EntityDamageSource : public DamageSource
{
protected:
	std::shared_ptr<Entity> entity;

public:
	//EntityDamageSource(const std::wstring &msgId, std::shared_ptr<Entity> entity);
	EntityDamageSource(ChatPacket::EChatPacketMessage msgId, std::shared_ptr<Entity> entity);
	virtual ~EntityDamageSource() { }

    std::shared_ptr<Entity> getEntity();

	// 4J Stu - Made return a packet
	//virtual std::wstring getLocalizedDeathMessage(std::shared_ptr<Player> player);
	virtual std::shared_ptr<ChatPacket> getDeathMessagePacket(std::shared_ptr<Player> player);

	virtual bool scalesWithDifficulty();
};