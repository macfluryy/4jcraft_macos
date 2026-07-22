#include "WaterAnimal.h"

#include <memory>

#include "java/Random.h"
#include "minecraft/world/damageSource/DamageSource.h"
#include "minecraft/world/entity/PathfinderMob.h"
#include "minecraft/world/level/Level.h"

WaterAnimal::WaterAnimal(Level* level) : PathfinderMob(level) {
    
    

    
    
}

bool WaterAnimal::isWaterMob() {
    return true;  
}

bool WaterAnimal::canSpawn() { return level->isUnobstructed(&bb); }

int WaterAnimal::getAmbientSoundInterval() { return 20 * 6; }

bool WaterAnimal::removeWhenFarAway() { return true; }

int WaterAnimal::getExperienceReward(std::shared_ptr<Player> killedBy) {
    return 1 + level->random->nextInt(3);
}

void WaterAnimal::baseTick() {
    int airSupply = getAirSupply();

    PathfinderMob::baseTick();  

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
