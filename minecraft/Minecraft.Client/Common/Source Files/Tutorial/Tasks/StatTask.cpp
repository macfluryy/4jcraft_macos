#include "Minecraft.World/Header Files/stdafx.h"
#include "../../../../net/minecraft/client/Minecraft.h"
#include "../../../../net/minecraft/client/player/LocalPlayer.h"
#include "../../../../net/minecraft/stats/StatsCounter.h"
#include "Minecraft.World/net/minecraft/stats/net.minecraft.stats.h"
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