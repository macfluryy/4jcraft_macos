#pragma once

#include "Stat.h"

class DescFormatter;

class Achievement : public Stat {
public:
    const int x, y;
    Achievement*
        requires;

private:
    const std::wstring desc;
    DescFormatter* descFormatter;

public:
    const std::shared_ptr<ItemInstance> icon;

private:
    bool isGoldenVar;
    void _init();

public:
    Achievement(int id, const std::wstring& name, int x, int y, Item* icon,
                Achievement* requires);
    Achievement(int id, const std::wstring& name, int x, int y, Tile* icon,
                Achievement* requires);
    Achievement(int id, const std::wstring& name, int x, int y,
                std::shared_ptr<ItemInstance> icon, Achievement* requires);

    Achievement* setAwardLocallyOnly();
    Achievement* setGolden();
    Achievement* postConstruct();
    bool isAchievement();
    std::wstring getDescription();
    Achievement* setDescFormatter(DescFormatter* descFormatter);
    bool isGolden();
    int getAchievementID();
};
