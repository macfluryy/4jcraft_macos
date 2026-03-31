#include <vector>

#include "GeneralStat.h"
#include "Minecraft.World/net/minecraft/stats/Stat.h"
#include "Minecraft.World/net/minecraft/stats/Stats.h"

class StatFormatter;

GeneralStat::GeneralStat(int id, const std::wstring& name,
                         StatFormatter* formatter)
    : Stat(id, name, formatter) {}

GeneralStat::GeneralStat(int id, const std::wstring& name) : Stat(id, name) {}

Stat* GeneralStat::postConstruct() {
    Stat::postConstruct();
    Stats::generalStats->push_back(this);
    return this;
}