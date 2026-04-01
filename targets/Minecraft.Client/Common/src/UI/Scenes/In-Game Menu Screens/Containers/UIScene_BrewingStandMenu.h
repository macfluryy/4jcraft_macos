#pragma once

#include <memory>
#include <string>

#include "Minecraft.Client/Common/src/UI/All Platforms/IUIScene_BrewingMenu.h"
#include "Minecraft.Client/Common/src/UI/All Platforms/UIEnums.h"
#include "Minecraft.Client/Common/src/UI/All Platforms/UIStructs.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_Label.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_Progress.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_SlotList.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"
#include "UIScene_AbstractContainerMenu.h"

class InventoryMenu;
class BrewingStandTileEntity;
class UILayer;

class UIScene_BrewingStandMenu : public UIScene_AbstractContainerMenu,
                                 public IUIScene_BrewingMenu {
private:
    std::shared_ptr<BrewingStandTileEntity> m_brewingStand;

public:
    UIScene_BrewingStandMenu(int iPad, void* initData, UILayer* parentLayer);

    virtual EUIScene getSceneType() { return eUIScene_BrewingStandMenu; }

protected:
    UIControl_SlotList m_slotListBottles[3], m_slotListIngredient;
    UIControl_Label m_labelBrewingStand;
    UIControl_Progress m_progressBrewingArrow, m_progressBrewingBubbles;

    UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene_AbstractContainerMenu)
    UI_BEGIN_MAP_CHILD_ELEMENTS(m_controlMainPanel)
    UI_MAP_ELEMENT(m_slotListBottles[0], "Bottle1")
    UI_MAP_ELEMENT(m_slotListBottles[1], "Bottle2")
    UI_MAP_ELEMENT(m_slotListBottles[2], "Bottle3")
    UI_MAP_ELEMENT(m_slotListIngredient, "Ingredient")
    UI_MAP_ELEMENT(m_labelBrewingStand, "BrewingStandText")

    UI_MAP_ELEMENT(m_progressBrewingArrow, "BrewingArrow")
    UI_MAP_ELEMENT(m_progressBrewingBubbles, "BrewingBubbles")
    UI_END_MAP_CHILD_ELEMENTS()
    UI_END_MAP_ELEMENTS_AND_NAMES()

    virtual std::wstring getMoviePath();
    virtual void handleReload();

    virtual void tick();

    virtual int getSectionColumns(ESceneSection eSection);
    virtual int getSectionRows(ESceneSection eSection);
    virtual void GetPositionOfSection(ESceneSection eSection,
                                      UIVec2D* pPosition);
    virtual void GetItemScreenData(ESceneSection eSection, int iItemIndex,
                                   UIVec2D* pPosition, UIVec2D* pSize);
    virtual void handleSectionClick(ESceneSection eSection) {}
    virtual void setSectionSelectedSlot(ESceneSection eSection, int x, int y);

    virtual UIControl* getSection(ESceneSection eSection);
};