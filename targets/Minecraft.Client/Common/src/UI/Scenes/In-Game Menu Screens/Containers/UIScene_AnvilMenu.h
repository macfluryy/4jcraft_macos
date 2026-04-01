#pragma once

#include <string>

#include "4J_Input.h"
#include "Minecraft.Client/Common/src/Tutorial/TutorialMode.h"
#include "Minecraft.Client/Common/src/UI/All Platforms/IUIScene_AnvilMenu.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_Label.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_SlotList.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_TextInput.h"
#include "UIScene_AbstractContainerMenu.h"
#include "minecraft/world/inventory/MerchantMenu.h"
#include "Minecraft.Client/Common/src/UI/All Platforms/UIEnums.h"
#include "Minecraft.Client/Common/src/UI/All Platforms/UIStructs.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"

class InventoryMenu;
class UILayer;

class UIScene_AnvilMenu : public UIScene_AbstractContainerMenu,
                          public IUIScene_AnvilMenu {
private:
    bool m_showingCross;

    enum EControls {
        eControl_TextInput,
    };

public:
    UIScene_AnvilMenu(int iPad, void* initData, UILayer* parentLayer);

    virtual EUIScene getSceneType() { return eUIScene_AnvilMenu; }

protected:
    UIControl_SlotList m_slotListItem1, m_slotListItem2, m_slotListResult;
    UIControl_Label m_labelAnvil;
    UIControl_TextInput m_textInputAnvil;

    IggyName m_funcShowRedCross, m_funcSetCostLabel;

    UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene_AbstractContainerMenu)
    UI_BEGIN_MAP_CHILD_ELEMENTS(m_controlMainPanel)
    UI_MAP_ELEMENT(m_slotListItem1, "Ingredient")
    UI_MAP_ELEMENT(m_slotListItem2, "Ingredient2")
    UI_MAP_ELEMENT(m_slotListResult, "Result")
    UI_MAP_ELEMENT(m_labelAnvil, "AnvilText")
    UI_MAP_ELEMENT(m_textInputAnvil, "AnvilTextInput")
    UI_END_MAP_CHILD_ELEMENTS()

    UI_MAP_NAME(m_funcShowRedCross, L"ShowRedCross")
    UI_MAP_NAME(m_funcSetCostLabel, L"SetCostLabel")
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

    static int KeyboardCompleteCallback(void* lpParam, bool bRes);
    virtual void handleEditNamePressed();
    virtual void setEditNameValue(const std::wstring& name);
    virtual void setEditNameEditable(bool enabled);
    virtual void handleDestroy();

    void setCostLabel(const std::wstring& label, bool canAfford);
    void showCross(bool show);
};
