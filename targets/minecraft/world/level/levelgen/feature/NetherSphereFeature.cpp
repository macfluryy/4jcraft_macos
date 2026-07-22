#include "NetherSphereFeature.h"

#include "minecraft/world/entity/Entity.h"

class Level;

NetherSphere::NetherSphere(Level* level) : Entity(level) {
    
    
    this->defineSynchedData();

    setSize(4, 4);
}

void NetherSphere::defineSynchedData() {}

void NetherSphere::readAdditionalSaveData(CompoundTag* tag) {}

void NetherSphere::addAdditonalSaveData(CompoundTag* tag) {}