#pragma once
#include "../../Minecraft.World/Headers/net.minecraft.world.level.h"

class DemoLevel : public Level {
private:
    static const __int64 DEMO_LEVEL_SEED =
        0;  // 4J - TODO - was "Don't Look Back".hashCode();
    static const int DEMO_SPAWN_X = 796;
    static const int DEMO_SPAWN_Y = 72;
    static const int DEMO_SPAWN_Z = -731;
    static LevelSettings DEMO_LEVEL_SETTINGS;

public:
    DemoLevel(std::shared_ptr<LevelStorage> levelStorage,
              const std::wstring& levelName);
    DemoLevel(Level* level, Dimension* dimension);

protected:
    virtual void setInitialSpawn();
};
