#include "../../Minecraft.World/Platform/stdafx.h"
#include "../../Minecraft.Client/Minecraft.h"
#include "../../Minecraft.Client/Player/LocalPlayer.h"
#include "../../Minecraft.Client/GameState/StatsCounter.h"
#include "../../Minecraft.World/Headers/net.minecraft.stats.h"
#include "StatTask.h"

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