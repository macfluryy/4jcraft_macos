#include "UIControl_Cursor.h"

#include "app/common/src/UI/Controls/UIControl.h"
#include "app/common/src/UI/Controls/UIControl_Base.h"
#include "app/mac/Iggy/include/iggy.h"
#ifndef _ENABLEIGGY
#include "app/mac/Stubs/iggy_stubs.h"
#endif

UIControl_Cursor::UIControl_Cursor() {}

bool UIControl_Cursor::setupControl(UIScene* scene, IggyValuePath* parent,
                                    const std::string& controlName) {
    UIControl::setControlType(UIControl::eCursor);
    bool success = UIControl_Base::setupControl(scene, parent, controlName);

    

    return success;
}
