#pragma once

#include <memory>

#include "app/common/src/UI/Controls/UIControl.h"
#include "app/common/src/UI/Controls/UIControl_EnchantmentBook.h"
#include "app/common/src/UI/UIScene.h"
#include "app/mac/Iggy/include/iggy.h"
#ifndef _ENABLEIGGY
#include "app/mac/Stubs/iggy_stubs.h"
#endif
#include "UIControl.h"
#include "java/Random.h"

class UIScene_EnchantingMenu;
class BookModel;
class ItemInstance;

class UIControl_EnchantmentBook : public UIControl {
private:
    BookModel* model;
    Random random;

    
    int time;
    float flip, oFlip, flipT, flipA;
    float open, oOpen;

    
    
    
    std::shared_ptr<ItemInstance> last;

    
    

    void tickBook();

public:
    UIControl_EnchantmentBook();

    void render(IggyCustomDrawCallbackRegion* region);
};
