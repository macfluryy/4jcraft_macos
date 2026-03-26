#include "../../Minecraft.World/Platform/stdafx.h"
#include "UI.h"
#include "UIComponent_DebugUIMarketingGuide.h"

UIComponent_DebugUIMarketingGuide::UIComponent_DebugUIMarketingGuide(
    int iPad, void* initData, UILayer* parentLayer)
    : UIScene(iPad, parentLayer) {
    // Setup all the Iggy references we need for this scene
    initialiseMovie();

    IggyDataValue result;
    IggyDataValue value[1];
    value[0].type = IGGY_DATATYPE_number;
    value[0].number = (F64)0;  // WIN64
#if 0
    value[0].number = (F64)1;
#elif 0
    value[0].number = (F64)2;
#elif 0
    value[0].number = (F64)3;
#elif 0
    value[0].number = (F64)4;
#elif 0
    value[0].number = (F64)5;
#elif defined _WINDOWS64 || defined __linux__
    value[0].number = (F64)0;
#endif
    IggyResult out = IggyPlayerCallMethodRS(getMovie(), &result,
                                            IggyPlayerRootPath(getMovie()),
                                            m_funcSetPlatform, 1, value);
}

std::wstring UIComponent_DebugUIMarketingGuide::getMoviePath() {
    return L"DebugUIMarketingGuide";
}
