#include "../Platform/stdafx.h"

#include "AnimalChest.h"

AnimalChest::AnimalChest(const std::wstring& name, int size)
    : SimpleContainer(IDS_CONTAINER_ANIMAL, name, false, size) {}

AnimalChest::AnimalChest(int iTitle, const std::wstring& name,
                         bool hasCustomName, int size)
    : SimpleContainer(iTitle, name, hasCustomName, size) {}