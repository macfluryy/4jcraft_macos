#pragma once

#include "../TamableAnimal.h"
class DamageSource;

class Wolf : public TamableAnimal {
public:
    eINSTANCEOF GetType() { return eTYPE_WOLF; }
    static Entity* create(Level* level) { return new Wolf(level); }

private:
    // synch health in a separate field to show tame wolves' health
    static const int DATA_HEALTH_ID = 18;
    static const int DATA_INTERESTED_ID = 19;
    static const int DATA_COLLAR_COLOR = 20;
    static const int START_HEALTH = 8;
    static const int MAX_HEALTH = 20;
    static const int TAME_HEALTH = 20;

    float interestedAngle, interestedAngleO;
    bool m_isWet, isShaking;
    float shakeAnim, shakeAnimO;

public:
    Wolf(Level* level);
    virtual bool useNewAi();
    virtual void setTarget(std::shared_ptr<Mob> target);

protected:
    virtual void serverAiMobStep();

public:
    virtual int getMaxHealth();

protected:
    virtual void defineSynchedData();
    virtual bool makeStepSound();

public:
    virtual int getTexture();  // 4J - changed from std::wstring to ing
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);

protected:
    virtual bool removeWhenFarAway();
    virtual int getAmbientSound();
    virtual int getHurtSound();
    virtual int getDeathSound();
    virtual float getSoundVolume();
    virtual int getDeathLoot();

public:
    virtual void aiStep();
    virtual void tick();
    bool isWet();
    float getWetShade(float a);
    float getBodyRollAngle(float a, float offset);
    float getHeadRollAngle(float a);
    float getHeadHeight();
    int getMaxHeadXRot();
    virtual bool hurt(DamageSource* source, int dmg);
    virtual bool doHurtTarget(std::shared_ptr<Entity> target);
    virtual bool interact(std::shared_ptr<Player> player);
    virtual void handleEntityEvent(uint8_t id);
    float getTailAngle();
    virtual bool isFood(std::shared_ptr<ItemInstance> item);
    virtual int getMaxSpawnClusterSize();
    bool isAngry();
    void setAngry(bool value);
    int getCollarColor();
    void setCollarColor(int color);
    void tame(const std::wstring& wsOwnerUUID, bool bDisplayTamingParticles,
              bool bSetSitting);

    // For tooltips
    int GetSynchedHealth();

protected:
    virtual std::shared_ptr<AgableMob> getBreedOffspring(
        std::shared_ptr<AgableMob> target);

public:
    virtual void setIsInterested(bool isInterested);
    virtual bool canMate(std::shared_ptr<Animal> animal);
    bool isInterested();
};
