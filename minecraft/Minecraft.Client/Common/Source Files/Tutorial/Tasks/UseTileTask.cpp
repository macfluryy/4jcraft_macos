#include "Minecraft.World/Header Files/stdafx.h"
#include "Minecraft.World/net/minecraft/world/entity/Entity.h"
#include "Minecraft.World/net/minecraft/world/level/Level.h"
#include "Minecraft.World/net/minecraft/world/item/ItemInstance.h"
#include "UseTileTask.h"

UseTileTask::UseTileTask(const int tileId, int x, int y, int z,
                         Tutorial* tutorial, int descriptionId,
                         bool enablePreCompletion,
                         std::vector<TutorialConstraint*>* inConstraints,
                         bool bShowMinimumTime, bool bAllowFade,
                         bool bTaskReminders)
    : TutorialTask(tutorial, descriptionId, enablePreCompletion, inConstraints,
                   bShowMinimumTime, bAllowFade, bTaskReminders),
      x(x),
      y(y),
      z(z),
      tileId(tileId) {
    useLocation = true;
}

UseTileTask::UseTileTask(const int tileId, Tutorial* tutorial,
                         int descriptionId, bool enablePreCompletion,
                         std::vector<TutorialConstraint*>* inConstraints,
                         bool bShowMinimumTime, bool bAllowFade,
                         bool bTaskReminders)
    : TutorialTask(tutorial, descriptionId, enablePreCompletion, inConstraints,
                   bShowMinimumTime, bAllowFade, bTaskReminders),
      tileId(tileId) {
    useLocation = false;
}

bool UseTileTask::isCompleted() { return bIsCompleted; }

void UseTileTask::useItemOn(Level* level, std::shared_ptr<ItemInstance> item,
                            int x, int y, int z, bool bTestUseOnly) {
    if (bTestUseOnly) return;

    if (!enablePreCompletion && !bHasBeenActivated) return;

    if (!useLocation || (x == this->x && y == this->y && z == this->z)) {
        int t = level->getTile(x, y, z);
        if (t == tileId) bIsCompleted = true;
    }
}