#include "UIComponent_DebugUIMarketingGuide.h"

#include "app/common/src/UI/UIScene.h"
#include "app/mac/Iggy/include/iggy.h"
#ifndef _ENABLEIGGY
#include "app/mac/Stubs/iggy_stubs.h"
#endif
#include "app/mac/Iggy/include/rrCore.h"

class UILayer;

UIComponent_DebugUIMarketingGuide::UIComponent_DebugUIMarketingGuide(
    int iPad, void* initData, UILayer* parentLayer)
    : UIScene(iPad, parentLayer) {
    
    initialiseMovie();

    IggyDataValue result;
    IggyDataValue value[1];
    value[0].type = IGGY_DATATYPE_number;
    value[0].number = (F64)0;  
#if defined(_WINDOWS64) || defined(__linux__)
    value[0].number = (F64)0;
#endif
    IggyResult out = IggyPlayerCallMethodRS(getMovie(), &result,
                                            IggyPlayerRootPath(getMovie()),
                                            m_funcSetPlatform, 1, value);
}

std::wstring UIComponent_DebugUIMarketingGuide::getMoviePath() {
    return L"DebugUIMarketingGuide";
}
