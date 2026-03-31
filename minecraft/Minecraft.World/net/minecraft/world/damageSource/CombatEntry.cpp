#include <limits>
#include <memory>

#include "CombatEntry.h"
#include "java/Class.h"
#include "Minecraft.World/net/minecraft/world/damageSource/CombatTracker.h"
#include "Minecraft.World/net/minecraft/world/damageSource/DamageSource.h"
#include "Minecraft.World/net/minecraft/world/entity/Entity.h"

CombatEntry::CombatEntry(DamageSource* source, int time, float health,
                         float damage, CombatTracker::eLOCATION location,
                         float fallDistance) {
    this->source = nullptr;
    if (source != nullptr) {
        // 4J: this might actually be a derived damage source so use copy func
        this->source = source->copy();
    }
    this->time = time;
    this->damage = damage;
    this->health = health;
    this->location = location;
    this->fallDistance = fallDistance;
}

CombatEntry::~CombatEntry() { delete source; }

DamageSource* CombatEntry::getSource() { return source; }

int CombatEntry::getTime() { return time; }

float CombatEntry::getDamage() { return damage; }

float CombatEntry::getHealthBeforeDamage() { return health; }

float CombatEntry::getHealthAfterDamage() { return health - damage; }

bool CombatEntry::isCombatRelated() {
    return source->getEntity() &&
           source->getEntity()->instanceof(eTYPE_LIVINGENTITY);
}

CombatTracker::eLOCATION CombatEntry::getLocation() { return location; }

std::wstring CombatEntry::getAttackerName() {
    return getSource()->getEntity() == nullptr
               ? L""
               : getSource()->getEntity()->getNetworkName();
}

float CombatEntry::getFallDistance() {
    if (source == DamageSource::outOfWorld)
        return std::numeric_limits<float>::max();
    return fallDistance;
}
