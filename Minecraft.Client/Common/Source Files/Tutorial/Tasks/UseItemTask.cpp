#include "../../../../../Minecraft.World/Header Files/stdafx.h"
#include "../../../../../Minecraft.World/net/minecraft/world/entity/Entity.h"
#include "../../../../../Minecraft.World/net/minecraft/world/level/Level.h"
#include "../../../../../Minecraft.World/net/minecraft/world/item/ItemInstance.h"
#include "UseItemTask.h"

UseItemTask::UseItemTask(const int itemId, Tutorial* tutorial,
                         int descriptionId, bool enablePreCompletion,
                         std::vector<TutorialConstraint*>* inConstraints,
                         bool bShowMinimumTime, bool bAllowFade,
                         bool bTaskReminders)
    : TutorialTask(tutorial, descriptionId, enablePreCompletion, inConstraints,
                   bShowMinimumTime, bAllowFade, bTaskReminders),
      itemId(itemId) {}

bool UseItemTask::isCompleted() { return bIsCompleted; }

void UseItemTask::useItem(std::shared_ptr<ItemInstance> item,
                          bool bTestUseOnly) {
    if (bTestUseOnly) return;

    if (item->id == itemId) bIsCompleted = true;
}