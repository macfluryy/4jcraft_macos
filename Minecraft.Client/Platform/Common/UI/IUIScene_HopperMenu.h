#pragma once

#include "IUIScene_AbstractContainerMenu.h"
#include "../../../../Minecraft.World/Containers/Container.h"
#include "../../../../Minecraft.World/Containers/Inventory.h"

class IUIScene_HopperMenu : public virtual IUIScene_AbstractContainerMenu {
public:
    virtual ESceneSection GetSectionAndSlotInDirection(ESceneSection eSection,
                                                       ETapState eTapDirection,
                                                       int* piTargetX,
                                                       int* piTargetY);
    int getSectionStartOffset(ESceneSection eSection);
};
