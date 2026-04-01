#include <memory>

#include "Minecraft.World/net/minecraft/world/item/ItemInstance.h"
#include "UseItemTask.h"
#include "Minecraft.Client/Common/src/Tutorial/Tasks/TutorialTask.h"

class Tutorial;
class TutorialConstraint;

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