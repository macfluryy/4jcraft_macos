
#include "UIScene_TrialExitUpsell.h"

#include "platform/InputActions.h"
#include "platform/sdl2/Profile.h"
#include "app/common/App_Defines.h"
#include "app/common/src/UI/UIScene.h"
#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"
#include "minecraft/sounds/SoundTypes.h"
#include "strings.h"

class UILayer;

UIScene_TrialExitUpsell::UIScene_TrialExitUpsell(int iPad, void* initData,
                                                 UILayer* parentLayer)
    : UIScene(iPad, parentLayer) {
    
    initialiseMovie();
}

std::wstring UIScene_TrialExitUpsell::getMoviePath() {
    return L"TrialExitUpsell";
}

void UIScene_TrialExitUpsell::updateTooltips() {
    ui.SetTooltips(DEFAULT_XUI_MENU_USER, IDS_EXIT_GAME, IDS_TOOLTIPS_BACK,
                   IDS_UNLOCK_TITLE);
}

void UIScene_TrialExitUpsell::handleInput(int iPad, int key, bool repeat,
                                          bool pressed, bool released,
                                          bool& handled) {
    
    
    

    ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

    switch (key) {
        case ACTION_MENU_CANCEL:
            navigateBack();
            break;
        case ACTION_MENU_OK:
            if (pressed) {
                
                ui.PlayUISFX(eSFX_Press);
                app.ExitGame();
            }
            break;
        case ACTION_MENU_X:
            if (ProfileManager.IsSignedIn(iPad)) {
                
                ui.PlayUISFX(eSFX_Press);
            }
            break;
    }
}

void UIScene_TrialExitUpsell::handleAnimationEnd() {
    
    ui.NavigateToScene(0, eUIScene_SaveMessage);
}
