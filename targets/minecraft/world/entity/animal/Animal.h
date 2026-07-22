#pragma once
#include <stdint.h>

#include <memory>

#include "minecraft/world/entity/AgeableMob.h"
#include "minecraft/world/entity/Creature.h"
#include "minecraft/world/entity/player/Player.h"

class Level;
class CompoundTag;
class DamageSource;
class ItemInstance;

class Animal : public AgableMob, public Creature {
private:
    static const int DATA_IN_LOVE = 13;  

    
    
    int loveTime;
    std::weak_ptr<Player> loveCause;

public:
    Animal(Level* level);

protected:
    virtual void defineSynchedData();
    virtual void serverAiMobStep();

public:
    virtual void aiStep();

protected:
    virtual void checkHurtTarget(std::shared_ptr<Entity> target, float d);

private:
    virtual void breedWith(std::shared_ptr<Animal> target);

public:
    virtual float getWalkTargetValue(int x, int y, int z);

public:
    virtual bool hurt(DamageSource* source, float dmg);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);

protected:
    virtual std::shared_ptr<Entity> findAttackTarget();

public:
    virtual bool canSpawn();
    virtual int getAmbientSoundInterval();

protected:
    virtual bool removeWhenFarAway();
    virtual int getExperienceReward(std::shared_ptr<Player> killedBy);

public:
    virtual bool isFood(std::shared_ptr<ItemInstance> itemInstance);
    virtual bool mobInteract(std::shared_ptr<Player> player);

protected:
    int getInLoveValue();  

public:
    void setInLoveValue(int value);  
    void setInLove(std::shared_ptr<Player>
                       player);  
                                 
    virtual void setInLove();
    std::shared_ptr<Player> getLoveCause();
    bool isInLove();
    void resetLove();
    virtual bool canMate(std::shared_ptr<Animal> partner);
    virtual void handleEntityEvent(uint8_t id);

    
private:
    bool m_isDespawnProtected;

    static const int MAX_WANDER_DISTANCE =
        20;  
             
    short m_minWanderX,
        m_maxWanderX;  
                       
    short m_minWanderZ,
        m_maxWanderZ;  
                       
    void updateDespawnProtectedState();

public:
    virtual bool isDespawnProtected();
    virtual void setDespawnProtected();
};
