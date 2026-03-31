#pragma once
#include "Minecraft.World/net/minecraft/world/entity/Entity.h"
#include "Minecraft.World/net/minecraft/world/entity/Entity.h"

class Level;

// class GlobalEntity : public Entity
class GlobalEntity : public Entity {
public:
    GlobalEntity(Level* level) : Entity(level) {};
};