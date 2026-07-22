#pragma once

#include <memory>

#include "EntityDamageSource.h"
#include "minecraft/network/packet/ChatPacket.h"

class Entity;
class Player;

class IndirectEntityDamageSource : public EntityDamageSource {
private:
    std::shared_ptr<Entity> owner;

public:
    
    
    IndirectEntityDamageSource(ChatPacket::EChatPacketMessage msgId,
                               ChatPacket::EChatPacketMessage msgWithItemId,
                               std::shared_ptr<Entity> entity,
                               std::shared_ptr<Entity> owner);
    virtual ~IndirectEntityDamageSource() {}

    virtual std::shared_ptr<Entity>
    getDirectEntity();  
    virtual std::shared_ptr<Entity> getEntity();

    
    
    
    virtual std::shared_ptr<ChatPacket> getDeathMessagePacket(
        std::shared_ptr<LivingEntity> player);

    virtual DamageSource* copy();
};