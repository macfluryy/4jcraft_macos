#pragma once

#include "Minecraft.World/net/minecraft/world/entity/PathfinderMob.h"
#include "Minecraft.World/net/minecraft/world/entity/Creature.h"

class Level;
class CompoundTag;

class Golem : public PathfinderMob, public Creature {
public:
    Golem(Level* level);

protected:
    virtual void causeFallDamage(float distance);
    virtual int getAmbientSound();
    virtual int getHurtSound();
    virtual int getDeathSound();

public:
    virtual int getAmbientSoundInterval();

protected:
    virtual bool removeWhenFarAway();
};