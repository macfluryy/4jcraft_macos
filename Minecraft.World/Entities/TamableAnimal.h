#pragma once

#include "Mobs/Animal.h"

class SitGoal;

class TamableAnimal : public Animal {
protected:
    static const int DATA_FLAGS_ID = 16;
    static const int DATA_OWNERUUID_ID = 17;

    SitGoal* sitGoal;

public:
    TamableAnimal(Level* level);
    virtual ~TamableAnimal();

protected:
    virtual void defineSynchedData();

public:
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);

protected:
    virtual void spawnTamingParticles(bool success);

public:
    virtual void handleEntityEvent(uint8_t id);
    virtual bool isTame();
    virtual void setTame(bool value);
    virtual bool isSitting();
    virtual void setSitting(bool value);
    virtual std::wstring getOwnerUUID();
    virtual void setOwnerUUID(const std::wstring& name);
    virtual std::shared_ptr<Mob> getOwner();
    virtual SitGoal* getSitGoal();
};