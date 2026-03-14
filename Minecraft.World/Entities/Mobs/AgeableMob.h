#pragma once

#include "../PathfinderMob.h"

class AgableMob : public PathfinderMob {
private:
    static const int DATA_AGE_ID = 12;

    float registeredBBWidth;
    float registeredBBHeight;

public:
    AgableMob(Level* level);

    virtual bool interact(std::shared_ptr<Player> player);

protected:
    virtual void defineSynchedData();

public:
    virtual std::shared_ptr<AgableMob> getBreedOffspring(
        std::shared_ptr<AgableMob> target) = 0;
    virtual int getAge();
    virtual void setAge(int age);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void aiStep();
    virtual bool isBaby();
    virtual void updateSize(bool isBaby);

protected:
    virtual void setSize(float w, float h);
    void internalSetSize(float scale);
};