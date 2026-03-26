#include "../../Minecraft.World/Platform/stdafx.h"
#include "UI.h"
#include "UIControl_LeaderboardList.h"

UIControl_LeaderboardList::UIControl_LeaderboardList() {}

bool UIControl_LeaderboardList::setupControl(UIScene* scene,
                                             IggyValuePath* parent,
                                             const std::string& controlName) {
    UIControl::setControlType(UIControl::eLeaderboardList);
    bool success = UIControl_Base::setupControl(scene, parent, controlName);

    // UIControl_LeaderboardList specific initialisers
    m_funcInitLeaderboard = registerFastName(L"InitLeaderboard");
    m_funcAddDataSet = registerFastName(L"AddDataSet");
    m_funcResetLeaderboard = registerFastName(L"ResetLeaderboard");
    m_funcSetupTitles = registerFastName(L"SetupTitles");
    m_funcSetColumnIcon = registerFastName(L"SetColumnIcon");

    return success;
}

void UIControl_LeaderboardList::init(int id) {
    m_id = id;

    IggyDataValue result;
    IggyDataValue value[1];
    value[0].type = IGGY_DATATYPE_number;
    value[0].number = id;
    IggyResult out =
        IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
                               getIggyValuePath(), m_initFunc, 1, value);
}

void UIControl_LeaderboardList::ReInit() {
    UIControl_Base::ReInit();
    init(m_id);
}

void UIControl_LeaderboardList::clearList() {
    IggyDataValue result;
    IggyResult out = IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
                                            getIggyValuePath(),
                                            m_funcResetLeaderboard, 0, NULL);
}

void UIControl_LeaderboardList::setupTitles(const std::wstring& rank,
                                            const std::wstring& gamertag) {
    IggyDataValue result;
    IggyDataValue value[2];

    const std::u16string convRank = wstring_to_u16string(rank);

    IggyStringUTF16 stringVal0;
    stringVal0.string = convRank.c_str();
    stringVal0.length = convRank.length();
    value[0].type = IGGY_DATATYPE_string_UTF16;
    value[0].string16 = stringVal0;

    const std::u16string convGamertag = wstring_to_u16string(gamertag);

    IggyStringUTF16 stringVal1;
    stringVal1.string = convGamertag.c_str();
    stringVal1.length = convGamertag.length();
    value[1].type = IGGY_DATATYPE_string_UTF16;
    value[1].string16 = stringVal1;

    IggyResult out =
        IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
                               getIggyValuePath(), m_funcSetupTitles, 2, value);
}

void UIControl_LeaderboardList::initLeaderboard(int iFirstFocus,
                                                int iTotalEntries,
                                                int iNumColumns) {
    IggyDataValue result;
    IggyDataValue value[3];
    value[0].type = IGGY_DATATYPE_number;
    value[0].number = iFirstFocus;

    value[1].type = IGGY_DATATYPE_number;
    value[1].number = iTotalEntries;

    value[2].type = IGGY_DATATYPE_number;
    value[2].number = iNumColumns;
    IggyResult out = IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
                                            getIggyValuePath(),
                                            m_funcInitLeaderboard, 3, value);

}

void UIControl_LeaderboardList::setColumnIcon(int iColumn, int iType) {
    IggyDataValue result;
    IggyDataValue value[2];
    value[0].type = IGGY_DATATYPE_number;
    value[0].number = iColumn;

    value[1].type = IGGY_DATATYPE_number;
    value[1].number = (iType <= 32000) ? 0 : (iType - 32000);

    IggyResult out = IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
                                            getIggyValuePath(),
                                            m_funcSetColumnIcon, 2, value);
}

void UIControl_LeaderboardList::addDataSet(
    bool bLast, int iId, int iRank, const std::wstring& gamertag,
    bool bDisplayMessage, const std::wstring& col0, const std::wstring& col1,
    const std::wstring& col2, const std::wstring& col3,
    const std::wstring& col4, const std::wstring& col5,
    const std::wstring& col6) {
    IggyDataValue result;
    IggyDataValue value[12];

    value[0].type = IGGY_DATATYPE_boolean;
    value[0].boolval = bLast;

    value[1].type = IGGY_DATATYPE_number;
    value[1].number = iId;

    value[2].type = IGGY_DATATYPE_number;
    value[2].number = iRank;

    const std::u16string convGamertag = wstring_to_u16string(gamertag);

    IggyStringUTF16 stringVal0;
    stringVal0.string = convGamertag.c_str();
    stringVal0.length = convGamertag.length();
    value[3].type = IGGY_DATATYPE_string_UTF16;
    value[3].string16 = stringVal0;

    value[4].type = IGGY_DATATYPE_boolean;
    value[4].boolval = bDisplayMessage;

    const std::u16string convCol0 = wstring_to_u16string(col0);

    IggyStringUTF16 stringVal1;
    stringVal1.string = convCol0.c_str();
    stringVal1.length = convCol0.length();
    value[5].type = IGGY_DATATYPE_string_UTF16;
    value[5].string16 = stringVal1;

    if (col1.empty()) {
        value[6].type = IGGY_DATATYPE_null;
    } else {
        const std::u16string convCol1 = wstring_to_u16string(col1);

        IggyStringUTF16 stringVal2;
        stringVal2.string = convCol1.c_str();
        stringVal2.length = convCol1.length();
        value[6].type = IGGY_DATATYPE_string_UTF16;
        value[6].string16 = stringVal2;
    }

    if (col2.empty()) {
        value[7].type = IGGY_DATATYPE_null;
    } else {
        const std::u16string convCol2 = wstring_to_u16string(col2);

        IggyStringUTF16 stringVal3;
        stringVal3.string = convCol2.c_str();
        stringVal3.length = convCol2.length();
        value[7].type = IGGY_DATATYPE_string_UTF16;
        value[7].string16 = stringVal3;
    }

    if (col3.empty()) {
        value[8].type = IGGY_DATATYPE_null;
    } else {
        const std::u16string convCol3 = wstring_to_u16string(col3);

        IggyStringUTF16 stringVal4;
        stringVal4.string = convCol3.c_str();
        stringVal4.length = convCol3.length();
        value[8].type = IGGY_DATATYPE_string_UTF16;
        value[8].string16 = stringVal4;
    }

    if (col4.empty()) {
        value[9].type = IGGY_DATATYPE_null;
    } else {
        const std::u16string convCol4 = wstring_to_u16string(col4);

        IggyStringUTF16 stringVal5;
        stringVal5.string = convCol4.c_str();
        stringVal5.length = convCol4.length();
        value[9].type = IGGY_DATATYPE_string_UTF16;
        value[9].string16 = stringVal5;
    }

    if (col5.empty()) {
        value[10].type = IGGY_DATATYPE_null;
    } else {
        const std::u16string convCol5 = wstring_to_u16string(col5);

        IggyStringUTF16 stringVal6;
        stringVal6.string = convCol5.c_str();
        stringVal6.length = convCol5.length();
        value[10].type = IGGY_DATATYPE_string_UTF16;
        value[10].string16 = stringVal6;
    }

    if (col6.empty()) {
        value[11].type = IGGY_DATATYPE_null;
    } else {
        const std::u16string convCol6 = wstring_to_u16string(col6);

        IggyStringUTF16 stringVal7;
        stringVal7.string = convCol6.c_str();
        stringVal7.length = convCol6.length();
        value[11].type = IGGY_DATATYPE_string_UTF16;
        value[11].string16 = stringVal7;
    }
    IggyResult out =
        IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result,
                               getIggyValuePath(), m_funcAddDataSet, 12, value);
}

