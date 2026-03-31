#pragma once

#include "Minecraft.World/net/minecraft/world/Container.h"

class Level;

class Hopper : public virtual Container {
public:
    virtual Level* getLevel() = 0;
    virtual double getLevelX() = 0;
    virtual double getLevelY() = 0;
    virtual double getLevelZ() = 0;
};