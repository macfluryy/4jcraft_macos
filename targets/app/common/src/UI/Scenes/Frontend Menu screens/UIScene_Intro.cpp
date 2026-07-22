#include "UIScene_Intro.h"

#include "platform/InputActions.h"
#include "app/common/Game.h"
#include "app/common/src/UI/All Platforms/UIEnums.h"
#include "app/common/src/UI/UIScene.h"
#include "app/mac/Iggy/include/iggy.h"
#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"

class UILayer;

#if !defined(_ENABLEIGGY)
static int s_introTickCount = 0;
#endif

UIScene_Intro::UIScene_Intro(int iPad, void* initData, UILayer* parentLayer)
    : UIScene(iPad, parentLayer) {
    
    initialiseMovie();
    m_bIgnoreNavigate = false;
    m_bAnimationEnded = false;
#if !defined(_ENABLEIGGY)
    s_introTickCount = 0;
#endif

    bool bSkipESRB = false;
    bool bChina = false;

    
#if defined(_WINDOWS64) || defined(__linux__) || defined(__APPLE__)
    int platformIdx = 0;
#endif

    IggyDataValue result;
    IggyDataValue value[3];
    value[0].type = IGGY_DATATYPE_number;
    value[0].number = platformIdx;

    value[1].type = IGGY_DATATYPE_boolean;
    value[1].boolval = bChina ? true : bSkipESRB;

    value[2].type = IGGY_DATATYPE_boolean;
    value[2].boolval = bChina;

    IggyResult out = IggyPlayerCallMethodRS(getMovie(), &result,
                                            IggyPlayerRootPath(getMovie()),
                                            m_funcSetIntroPlatform, 3, value);
}

std::wstring UIScene_Intro::getMoviePath() { return L"Intro"; }

void UIScene_Intro::handleInput(int iPad, int key, bool repeat, bool pressed,
                                bool released, bool& handled) {
    ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

    switch (key) {
        case ACTION_MENU_OK:
            if (!m_bIgnoreNavigate) {
                m_bIgnoreNavigate = true;
                
                ui.NavigateToScene(0, eUIScene_SaveMessage);
            }
            break;
    }
}

void UIScene_Intro::handleAnimationEnd() {
    if (!m_bIgnoreNavigate) {
        m_bIgnoreNavigate = true;
        
        ui.NavigateToScene(0, eUIScene_SaveMessage);
    }
}

void UIScene_Intro::handleGainFocus(bool navBack) {
    
    
    
    if (m_bAnimationEnded) {
        ui.NavigateToScene(0, eUIScene_MainMenu);
    }
}

#if !defined(_ENABLEIGGY)
void UIScene_Intro::tick() {
    
    UIScene::tick();

    
    
    s_introTickCount++;
    if (s_introTickCount == 60 && !m_bIgnoreNavigate) {
        
        
        
        
        
        
        
        if (app.GetGameStarted()) {
            m_bIgnoreNavigate = true;
            fprintf(stderr,
                    "[Linux] Suppressing intro -> MainMenu nav: game already "
                    "started (direct-connect)\n");
            return;
        }
        fprintf(stderr,
                "[Linux] Auto-skipping intro -> MainMenu after %d ticks\n",
                s_introTickCount);
        m_bIgnoreNavigate = true;
        
        
        ui.NavigateToScene(0, eUIScene_MainMenu);
    }
}
#endif
