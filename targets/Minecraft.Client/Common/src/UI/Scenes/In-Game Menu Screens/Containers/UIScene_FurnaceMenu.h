#pragma once

#include <memory>
#include <string>

#include "Minecraft.Client/Common/src/UI/All Platforms/IUIScene_FurnaceMenu.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_Label.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_Progress.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_SlotList.h"
#include "UIScene_AbstractContainerMenu.h"
#include "Minecraft.Client/Common/src/UI/All Platforms/UIEnums.h"
#include "Minecraft.Client/Common/src/UI/All Platforms/UIStructs.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"

class InventoryMenu;
class FurnaceTileEntity;
class UILayer;

class UIScene_FurnaceMenu : public UIScene_AbstractContainerMenu,
                            public IUIScene_FurnaceMenu {
private:
    std::shared_ptr<FurnaceTileEntity> m_furnace;

public:
    UIScene_FurnaceMenu(int iPad, void* initData, UILayer* parentLayer);

    virtual EUIScene getSceneType() { return eUIScene_FurnaceMenu; }

protected:
    UIControl_SlotList m_slotListFuel, m_slotListIngredient, m_slotListResult;
    UIControl_Label m_labelFurnace, m_labelIngredient, m_labelFuel;
    UIControl_Progress m_progressFurnaceFire, m_progressFurnaceArrow;

    UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene_AbstractContainerMenu)
    UI_BEGIN_MAP_CHILD_ELEMENTS(m_controlMainPanel)
    UI_MAP_ELEMENT(m_slotListIngredient, "Ingredient")
    UI_MAP_ELEMENT(m_slotListFuel, "Fuel")
    UI_MAP_ELEMENT(m_slotListResult, "Result")
    UI_MAP_ELEMENT(m_labelFurnace, "Furnace_text")
    UI_MAP_ELEMENT(m_labelIngredient, "Ingredient_Label")
    UI_MAP_ELEMENT(m_labelFuel, "Fuel_Label")

    UI_MAP_ELEMENT(m_progressFurnaceFire, "FurnaceFire")
    UI_MAP_ELEMENT(m_progressFurnaceArrow, "FurnaceArrow")
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