#pragma once

#include "Minecraft.World/net/minecraft/world/entity/Entity.h"
#include "java/Class.h"
#include "Minecraft.World/net/minecraft/world/entity/Entity.h"

class Level;

class NetherSphere : public Entity {
public:
    eINSTANCEOF GetType() { return eTYPE_NETHER_SPHERE; };

public:
    NetherSphere(Level* level);

protected:
    virtual void defineSynchedData();
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
};