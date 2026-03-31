#include "JumpControl.h"
#include "Minecraft.World/net/minecraft/world/entity/Mob.h"

JumpControl::JumpControl(Mob* mob) {
    _jump = false;

    this->mob = mob;
}

void JumpControl::jump() { _jump = true; }

void JumpControl::tick() {
    mob->setJumping(_jump);
    _jump = false;
}