#pragma once

#include "IUIScene_AbstractContainerMenu.h"
#include "../../../../../Minecraft.World/net/minecraft/world/Container.h"
#include "../../../../../Minecraft.World/net/minecraft/world/entity/player/Inventory.h"
#include "../../../../../Minecraft.World/net/minecraft/world/entity/animal/EntityHorse.h"

class IUIScene_HorseInventoryMenu
    : public virtual IUIScene_AbstractContainerMenu {
protected:
    std::shared_ptr<Inventory> m_inventory;
    std::shared_ptr<Container> m_container;
    std::shared_ptr<EntityHorse> m_horse;

public:
    virtual ESceneSection GetSectionAndSlotInDirection(ESceneSection eSection,
                                                       ETapState eTapDirection,
                                                       int* piTargetX,
                                                       int* piTargetY);
    int getSectionStartOffset(ESceneSection eSection);
    bool IsSectionSlotList(ESceneSection eSection);
    bool IsVisible(ESceneSection eSection);
};
