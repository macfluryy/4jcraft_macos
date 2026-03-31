#include "Minecraft.Client/net/minecraft/client/Minecraft.h"
#include "Minecraft.Client/net/minecraft/stats/StatsCounter.h"
#include "StatTask.h"
#include "4J_Profile.h"
#include "Minecraft.Client/Common/Source Files/Tutorial/Tasks/TutorialTask.h"

class Tutorial;

StatTask::StatTask(Tutorial* tutorial, int descriptionId,
                   bool enablePreCompletion, Stat* stat, int variance /*= 1*/)
    : TutorialTask(tutorial, descriptionId, enablePreCompletion, nullptr) {
    this->stat = stat;

    Minecraft* minecraft = Minecraft::GetInstance();
    targetValue =
        minecraft->stats[ProfileManager.GetPrimaryPad()]->getTotalValue(stat) +
        variance;
}

bool StatTask::isCompleted() {
    if (bIsCompleted) return true;

    Minecraft* minecraft = Minecraft::GetInstance();
    bIsCompleted =
        minecraft->stats[ProfileManager.GetPrimaryPad()]->getTotalValue(stat) >=
        (unsigned int)targetValue;
    return bIsCompleted;
}