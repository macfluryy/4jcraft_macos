#include "../../Platform/stdafx.h"

#include "AmbientCreature.h"

AmbientCreature::AmbientCreature(Level* level) : Mob(level) {}

bool AmbientCreature::canBeLeashed() { return false; }

bool AmbientCreature::mobInteract(std::shared_ptr<Player> player) {
    return false;
}