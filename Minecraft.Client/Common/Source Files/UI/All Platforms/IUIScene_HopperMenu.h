#pragma once

#include "IUIScene_AbstractContainerMenu.h"
#include "../../../../../Minecraft.World/net/minecraft/world/Container.h"
#include "../../../../../Minecraft.World/net/minecraft/world/entity/player/Inventory.h"

class IUIScene_HopperMenu : public virtual IUIScene_AbstractContainerMenu {
public:
    virtual ESceneSection GetSectionAndSlotInDirection(ESceneSection eSection,
                                                       ETapState eTapDirection,
                                                       int* piTargetX,
                                                       int* piTargetY);
    int getSectionStartOffset(ESceneSection eSection);
};
