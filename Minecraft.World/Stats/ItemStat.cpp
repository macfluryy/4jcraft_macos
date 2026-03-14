#include "../Platform/stdafx.h"
#include "ItemStat.h"

ItemStat::ItemStat(int id, const std::wstring& name, int itemId)
    : Stat(id, name), itemId(itemId) {}

int ItemStat::getItemId() { return itemId; }
