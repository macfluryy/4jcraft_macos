#include "UIControl_DLCList.h"

#include "Minecraft.Client/Common/src/UI/Controls/UIControl.h"
#include "Minecraft.Client/Common/src/UI/Controls/UIControl_ButtonList.h"
#include "Minecraft.Client/Common/src/UI/UIScene.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "Minecraft.Client/Linux/Iggy/include/rrCore.h"
#include "console_helpers/StringHelpers.h"

bool UIControl_DLCList::setupControl(UIScene* scene, IggyValuePath* parent,
                                     const std::string& controlName) {
    UIControl::setControlType(UIControl::eDLCList);
    bool success =
        UIControl_ButtonList::setupControl(scene, parent, controlName);

    // SlotList specific initialisers
    m_funcShowTick = registerFastName(L"ShowTick");

    return success;
}

void UIControl_DLCList::addItem(const std::string& label, bool showTick,
                                int iId) {
    IggyDataValue result;
    IggyDataValue value[3];

    IggyStringUTF8 stringVal;
    stringVal.string = (char*)label.c_str();
    stringVal.length = (S32)label.length();
    value[0].type = IGGY_DATATYPE_string_UTF8;
    value[0].string8 = stringVal;

    value[1].type = IGGY_DATATYPE_number;
    value[1].number = iId;

    value[2].type = IGGY_DATATYPE_boolean;
    value[2].boolval = showTick;
    IggyResult out =
        IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
                               getIggyValuePath(), m_addNewItemFunc, 3, value);

    ++m_itemCount;
}

void UIControl_DLCList::addItem(const std::wstring& label, bool showTick,
                                int iId) {
    IggyDataValue result;
    IggyDataValue value[3];

    const std::u16string convLabel = wstring_to_u16string(label);

    IggyStringUTF16 stringVal;
    stringVal.string = convLabel.c_str();
    stringVal.length = (S32)convLabel.length();
    value[0].type = IGGY_DATATYPE_string_UTF16;
    value[0].string16 = stringVal;

    value[1].type = IGGY_DATATYPE_number;
    value[1].number = iId;

    value[2].type = IGGY_DATATYPE_boolean;
    value[2].boolval = showTick;
    IggyResult out =
        IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
                               getIggyValuePath(), m_addNewItemFunc, 3, value);

    ++m_itemCount;
}

void UIControl_DLCList::showTick(int iId, bool showTick) {
    IggyDataValue result;
    IggyDataValue value[2];

    value[0].type = IGGY_DATATYPE_number;
    value[0].number = iId;

    value[1].type = IGGY_DATATYPE_boolean;
    value[1].boolval = showTick;
    IggyResult out =
        IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
                               getIggyValuePath(), m_funcShowTick, 2, value);
}
