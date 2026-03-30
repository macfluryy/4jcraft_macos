#include "../../../../../Minecraft.World/Header Files/stdafx.h"
#include "DemoLevel.h"
#include "../../../../../Minecraft.World/net/minecraft/world/level/storage/net.minecraft.world.level.storage.h"

LevelSettings DemoLevel::DEMO_LEVEL_SETTINGS =
    LevelSettings(DemoLevel::DEMO_LEVEL_SEED, GameType::SURVIVAL, false, false,
                  false, LevelType::lvl_normal_1_1, LEVEL_MAX_WIDTH, 1.0);

DemoLevel::DemoLevel(std::shared_ptr<LevelStorage> levelStorage,
                     const std::wstring& levelName)
    : Level(levelStorage, levelName, &DEMO_LEVEL_SETTINGS) {}

void DemoLevel::setInitialSpawn() {
    levelData->setSpawn(DEMO_SPAWN_X, DEMO_SPAWN_Y, DEMO_SPAWN_Z);
}