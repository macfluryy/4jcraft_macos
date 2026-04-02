#include "app/common/src/UI/Controls/UIControl_Base.h"

#include <string>
#include <vector>

#include "app/common/src/UI/Controls/UIControl.h"
#include "app/common/src/UI/UIScene.h"
#include "app/common/src/UI/UIString.h"
#include "app/linux/Iggy/include/iggy.h"
#include "app/linux/Iggy/include/rrCore.h"
#include "console_helpers/StringHelpers.h"

UIControl_Base::UIControl_Base() {
    m_bLabelChanged = false;
    m_id = 0;
}

bool UIControl_Base::setupControl(UIScene* scene, IggyValuePath* parent,
                                  const std::string& controlName) {
    bool success = UIControl::setupControl(scene, parent, controlName);

    m_setLabelFunc = registerFastName(L"SetLabel");
    m_initFunc = registerFastName(L"Init");
    m_funcGetLabel = registerFastName(L"GetLabel");
    m_funcCheckLabelWidths = registerFastName(L"CheckLabelWidths");

    return success;
}

void UIControl_Base::tick() {
    UIControl::tick();

    if (m_label.needsUpdating() || m_bLabelChanged) {
        // app.DebugPrintf("Calling SetLabel - '%ls'\n", m_label.c_str());
        m_bLabelChanged = false;

        const std::u16string convLabel =
            wstring_to_u16string(m_label.getString());

        IggyDataValue result;
        IggyDataValue value[1];
        value[0].type = IGGY_DATATYPE_string_UTF16;
        IggyStringUTF16 stringVal;

        stringVal.string = convLabel.c_str();
        stringVal.length = convLabel.length();
        value[0].string16 = stringVal;

        IggyResult out = IggyPlayerCallMethodRS(m_parentScene->getMovie(),
                                                &result, getIggyValuePath(),
                                                m_setLabelFunc, 1, value);

        m_label.setUpdated();
    }
}

void UIControl_Base::setLabel(UIString label, bool instant, bool force) {
    if (force ||
        ((!m_label.empty() || !label.empty()) && m_label.compare(label) != 0))
        m_bLabelChanged = true;
    m_label = label;

    if (m_bLabelChanged && instant) {
        m_bLabelChanged = false;

        const std::u16string convLabel =
            wstring_to_u16string(m_label.getString());

        IggyDataValue result;
        IggyDataValue value[1];
        value[0].type = IGGY_DATATYPE_string_UTF16;
        IggyStringUTF16 stringVal;

        stringVal.string = convLabel.c_str();
        stringVal.length = convLabel.length();
        value[0].string16 = stringVal;

        IggyResult out = IggyPlayerCallMethodRS(m_parentScene->getMovie(),
                                                &result, getIggyValuePath(),
                                                m_setLabelFunc, 1, value);
    }
}

const wchar_t* UIControl_Base::getLabel() {
    IggyDataValue result;
    IggyResult out =
        IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
                               getIggyValuePath(), m_funcGetLabel, 0, nullptr);

    if (result.type == IGGY_DATATYPE_string_UTF16) {
        m_label = u16string_to_wstring(result.string16.string);
    }

    return m_label.c_str();
}

void UIControl_Base::setAllPossibleLabels(int labelCount,
                                          wchar_t labels[][256]) {
    IggyDataValue result;
    IggyDataValue* value = new IggyDataValue[labelCount];
    IggyStringUTF16* stringVal = new IggyStringUTF16[labelCount];

    std::vector<std::u16string> conv;
    conv.reserve(labelCount);

    for (int i = 0; i < labelCount; ++i) {
        conv.push_back(wstring_to_u16string(labels[i]));
        stringVal[i].string = conv[i].c_str();
        stringVal[i].length = (S32)conv[i].length();
        value[i].type = IGGY_DATATYPE_string_UTF16;
        value[i].string16 = stringVal[i];
    }

    IggyResult out = IggyPlayerCallMethodRS(
        m_parentScene->getMovie(), &result, getIggyValuePath(),
        m_funcCheckLabelWidths, labelCount, value);

    delete[] value;
    delete[] stringVal;
}

bool UIControl_Base::hasFocus() { return m_parentScene->controlHasFocus(this); }
