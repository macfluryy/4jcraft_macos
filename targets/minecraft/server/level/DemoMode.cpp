
#include "DemoMode.h"



DemoMode::DemoMode(Minecraft* minecraft) : SurvivalMode(minecraft) {
    demoHasEnded = false;
    demoEndedReminder = 0;
}

void DemoMode::tick() {
    SurvivalMode::tick();

    















































}

void DemoMode::outputDemoReminder() {
    





}

void DemoMode::startDestroyBlock(int x, int y, int z, int face) {
    if (demoHasEnded) {
        outputDemoReminder();
        return;
    }
    SurvivalMode::startDestroyBlock(x, y, z, face);
}

void DemoMode::continueDestroyBlock(int x, int y, int z, int face) {
    if (demoHasEnded) {
        return;
    }
    SurvivalMode::continueDestroyBlock(x, y, z, face);
}

bool DemoMode::destroyBlock(int x, int y, int z, int face) {
    if (demoHasEnded) {
        return false;
    }
    return SurvivalMode::destroyBlock(x, y, z, face);
}

bool DemoMode::useItem(std::shared_ptr<Player> player, Level* level,
                       std::shared_ptr<ItemInstance> item) {
    if (demoHasEnded) {
        outputDemoReminder();
        return false;
    }
    return SurvivalMode::useItem(player, level, item);
}

bool DemoMode::useItemOn(std::shared_ptr<Player> player, Level* level,
                         std::shared_ptr<ItemInstance> item, int x, int y,
                         int z, int face) {
    if (demoHasEnded) {
        outputDemoReminder();
        return false;
    }
    return SurvivalMode::useItemOn(player, level, item, x, y, z, face);
}

void DemoMode::attack(std::shared_ptr<Player> player,
                      std::shared_ptr<Entity> entity) {
    if (demoHasEnded) {
        outputDemoReminder();
        return;
    }
    SurvivalMode::attack(player, entity);
}
