#pragma once

#include <memory>

#include "app/common/src/UI/Controls/UIControl.h"
#include "app/common/src/UI/Controls/UIControl_EnchantmentBook.h"
#include "app/common/src/UI/UIScene.h"
#include "app/linux/Iggy/include/iggy.h"
#include "UIControl.h"
#include "java/Random.h"

class UIScene_EnchantingMenu;
class BookModel;
class ItemInstance;

class UIControl_EnchantmentBook : public UIControl {
private:
    BookModel* model;
    Random random;

    // 4J JEV: Book animation variables.
    int time;
    float flip, oFlip, flipT, flipA;
    float open, oOpen;

    // bool m_bDirty;
    // float m_fScale,m_fAlpha;
    // int	m_iPad;
    std::shared_ptr<ItemInstance> last;

    // float m_fScreenWidth,m_fScreenHeight;
    // float m_fRawWidth,m_fRawHeight;

    void tickBook();

public:
    UIControl_EnchantmentBook();

    void render(IggyCustomDrawCallbackRegion* region);
};
