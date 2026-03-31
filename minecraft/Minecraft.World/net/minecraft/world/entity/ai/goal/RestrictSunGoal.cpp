#include "RestrictSunGoal.h"
#include "Minecraft.World/net/minecraft/world/entity/PathfinderMob.h"
#include "Minecraft.World/net/minecraft/world/entity/ai/navigation/PathNavigation.h"
#include "Minecraft.World/net/minecraft/world/level/Level.h"

RestrictSunGoal::RestrictSunGoal(PathfinderMob* mob) { this->mob = mob; }

bool RestrictSunGoal::canUse() { return mob->level->isDay(); }

void RestrictSunGoal::start() { mob->getNavigation()->setAvoidSun(true); }

void RestrictSunGoal::stop() { mob->getNavigation()->setAvoidSun(false); }