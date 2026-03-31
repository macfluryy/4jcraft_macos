#pragma once

#include "Minecraft.World/net/minecraft/world/SimpleContainer.h"

class AnimalChest : public SimpleContainer {
public:
    AnimalChest(const std::wstring& name, int size);
    AnimalChest(int iTitle, const std::wstring& name, bool hasCustomName,
                int size);  // 4J Added iTitle param
};