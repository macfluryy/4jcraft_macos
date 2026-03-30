#include "Minecraft.World/Header Files/stdafx.h"
#include "../UI.h"
#include "UIControl_Label.h"
#include "Minecraft.World/ConsoleHelpers/StringHelpers.h"

UIControl_Label::UIControl_Label() {}

bool UIControl_Label::setupControl(UIScene* scene, IggyValuePath* parent,
                                   const std::string& controlName) {
    UIControl::setControlType(UIControl::eLabel);
    bool success = UIControl_Base::setupControl(scene, parent, controlName);

    // Label specific initialisers

    return success;
}

void UIControl_Label::init(UIString label) {
    m_label = label;

    const std::u16string convLabel = wstring_to_u16string(label.getString());

    IggyDataValue result;
    IggyDataValue value[1];
    value[0].type = IGGY_DATATYPE_string_UTF16;
    IggyStringUTF16 stringVal;

    stringVal.string = convLabel.c_str();
    stringVal.length = convLabel.length();
    value[0].string16 = stringVal;
    IggyResult out =
        IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
                               getIggyValuePath(), m_initFunc, 1, value);
}

void UIControl_Label::ReInit() {
    UIControl_Base::ReInit();

    // 4J-JEV: This can't be reinitialised.
    if (m_reinitEnabled) {
        init(m_label);
    }
}
