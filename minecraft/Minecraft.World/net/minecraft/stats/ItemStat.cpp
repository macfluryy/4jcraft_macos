#include "ItemStat.h"
#include "Minecraft.World/net/minecraft/stats/Stat.h"

ItemStat::ItemStat(int id, const std::wstring& name, int itemId)
    : Stat(id, name), itemId(itemId) {}

int ItemStat::getItemId() { return itemId; }
