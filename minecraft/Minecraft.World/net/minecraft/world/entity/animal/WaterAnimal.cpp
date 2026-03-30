#include "../../../../../Header Files/stdafx.h"
#include "nbt/com.mojang.nbt.h"
#include "../../level/tile/net.minecraft.world.level.tile.h"
#include "../../phys/net.minecraft.world.phys.h"
#include "../../level/net.minecraft.world.level.h"
#include "../../damageSource/net.minecraft.world.damagesource.h"
#include "WaterAnimal.h"

WaterAnimal::WaterAnimal(Level* level) : PathfinderMob(level) {
    // 4J Stu - This function call had to be moved here from the Entity ctor to
    // ensure that the derived version of the function is called

    // This should only be called for the most derive classes
    // this->defineSynchedData();
}

bool WaterAnimal::isWaterMob() {
    return true;  // prevent drowning
}

bool WaterAnimal::canSpawn() { return level->isUnobstructed(&bb); }

int WaterAnimal::getAmbientSoundInterval() { return 20 * 6; }

bool WaterAnimal::removeWhenFarAway() { return true; }

int WaterAnimal::getExperienceReward(std::shared_ptr<Player> killedBy) {
    return 1 + level->random->nextInt(3);
}

void WaterAnimal::baseTick() {
    int airSupply = getAirSupply();

    PathfinderMob::baseTick();  // this modified the airsupply

    if (isAlive() && !isInWater()) {
        setAirSupply(--airSupply);
        if (getAirSupply() == -20) {
            setAirSupply(0);
            hurt(DamageSource::drown, 2);
        }
    } else {
        setAirSupply(TOTAL_AIR_SUPPLY);
    }
}
