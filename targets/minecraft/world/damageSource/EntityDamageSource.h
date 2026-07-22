#pragma once

#include <memory>

#include "DamageSource.h"
#include "minecraft/network/packet/ChatPacket.h"

class Entity;
class Player;

class EntityDamageSource : public DamageSource {
protected:
    std::shared_ptr<Entity> entity;

public:
    
    
    EntityDamageSource(ChatPacket::EChatPacketMessage msgId,
                       ChatPacket::EChatPacketMessage msgWithItemId,
                       std::shared_ptr<Entity> entity);
    virtual ~EntityDamageSource() {}

    std::shared_ptr<Entity> getEntity();

    
    
    
    virtual std::shared_ptr<ChatPacket> getDeathMessagePacket(
        std::shared_ptr<LivingEntity> player);

    virtual bool scalesWithDifficulty();

    virtual DamageSource* copy();
};