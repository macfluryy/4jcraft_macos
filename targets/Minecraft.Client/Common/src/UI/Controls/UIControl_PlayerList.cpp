#include "UIControl_PlayerList.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_PlayerList.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_ButtonList.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"
#include "Minecraft.Client/Linux/Iggy/include/rrCore.h"
#include "console_helpers/StringHelpers.h"

bool UIControl_PlayerList::setupControl(UIScene* scene, IggyValuePath* parent,
                                        const std::string& controlName) {
    UIControl::setControlType(UIControl::ePlayerList);
    bool success =
        UIControl_ButtonList::setupControl(scene, parent, controlName);

    // SlotList specific initialisers
    m_funcSetPlayerIcon = registerFastName(L"SetPlayerIcon");
    m_funcSetVOIPIcon = registerFastName(L"SetVOIPIcon");

    return success;
}

void UIControl_PlayerList::addItem(const std::wstring& label, int iPlayerIcon,
                                   int iVOIPIcon) {
    IggyDataValue result;
    IggyDataValue value[4];

    const std::u16string convLabel = wstring_to_u16string(label);

    IggyStringUTF16 stringVal;
    stringVal.string = convLabel.c_str();
    stringVal.length = (S32)convLabel.length();
    value[0].type = IGGY_DATATYPE_string_UTF16;
    value[0].string16 = stringVal;

    value[1].type = IGGY_DATATYPE_number;
    value[1].number = m_itemCount;

    value[2].type = IGGY_DATATYPE_number;
    value[2].number = iPlayerIcon + 1;

    value[3].type = IGGY_DATATYPE_number;
    value[3].number = iVOIPIcon + 1;
    IggyResult out =
        IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
                               getIggyValuePath(), m_addNewItemFunc, 4, value);

    ++m_itemCount;
}

void UIControl_PlayerList::setPlayerIcon(int iId, int iPlayerIcon) {
    IggyDataValue result;
    IggyDataValue value[2];

    value[0].type = IGGY_DATATYPE_number;
    value[0].number = iId;

    value[1].type = IGGY_DATATYPE_number;
    value[1].number = iPlayerIcon + 1;
    IggyResult out = IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
                                            getIggyValuePath(),
                                            m_funcSetPlayerIcon, 2, value);
}

void UIControl_PlayerList::setVOIPIcon(int iId, int iVOIPIcon) {
    IggyDataValue result;
    IggyDataValue value[2];

    value[0].type = IGGY_DATATYPE_number;
    value[0].number = iId;

    value[1].type = IGGY_DATATYPE_number;
    value[1].number = iVOIPIcon + 1;
    IggyResult out =
        IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
                               getIggyValuePath(), m_funcSetVOIPIcon, 2, value);
}
