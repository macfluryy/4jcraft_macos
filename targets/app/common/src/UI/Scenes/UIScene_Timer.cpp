#include "UIScene_Timer.h"

#include "app/common/src/UI/Controls/UIControl.h"
#include "app/common/src/UI/UIScene.h"

class UILayer;

UIScene_Timer::UIScene_Timer(int iPad, void* initData, UILayer* parentLayer)
    : UIScene(iPad, parentLayer) {
    
    initialiseMovie();

    
    
    if (initData == 0) {
        m_controlBackground.setVisible(false);
    }
}

std::wstring UIScene_Timer::getMoviePath() { return L"Timer"; }

void UIScene_Timer::reloadMovie(bool force) {
    
}

bool UIScene_Timer::needsReloaded() {
    
    return false;
}