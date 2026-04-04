#pragma once

#include <string>

#include "Stat.h"

class ItemStat : public Stat {
private:
    const int itemId;

public:
    ItemStat(int id, const std::wstring& name, int itemId);
    int getItemId();
};
