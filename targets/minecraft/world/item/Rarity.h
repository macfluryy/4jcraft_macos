#pragma once

#include <string>

#include "app/common/App_enums.h"

class Rarity {
public:
    static const Rarity* common;
    static const Rarity* uncommon;
    static const Rarity* rare;
    static const Rarity* epic;

    const eMinecraftColour color;
    const std::wstring name;

    Rarity(eMinecraftColour color, const std::wstring& name);
};