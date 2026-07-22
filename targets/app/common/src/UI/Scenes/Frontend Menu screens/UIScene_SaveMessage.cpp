
#include "UIScene_SaveMessage.h"

#include "platform/PlatformTypes.h"
#include "platform/InputActions.h"
#include "platform/sdl2/Input.h"
#include "platform/sdl2/Profile.h"
#include "app/common/App_Defines.h"
#include "app/common/src/UI/Controls/UIControl_Button.h"
#include "app/common/src/UI/Controls/UIControl_Label.h"
#include "app/common/src/UI/UILayer.h"
#include "app/common/src/UI/UIScene.h"
#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"
#include "minecraft/sounds/SoundTypes.h"
#include "strings.h"

#define PROFILE_LOADED_TIMER_ID 0
#define PROFILE_LOADED_TIMER_TIME 50

UIScene_SaveMessage::UIScene_SaveMessage(int iPad, void* initData,
                                         UILayer* parentLayer)
    : UIScene(iPad, parentLayer) {
    
    initialiseMovie();

    parentLayer->addComponent(iPad, eUIComponent_Panorama);
    parentLayer->addComponent(iPad, eUIComponent_Logo);

    m_buttonConfirm.init(app.GetString(IDS_CONFIRM_OK), eControl_Confirm);
    m_labelDescription.init(app.GetString(IDS_SAVE_ICON_MESSAGE));

    IggyDataValue result;

    
    IggyResult out = IggyPlayerCallMethodRS(getMovie(), &result,
                                            IggyPlayerRootPath(getMovie()),
                                            m_funcAutoResize, 0, nullptr);

    
    for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i) {
        if ((InputManager.IsPadConnected(i) || ProfileManager.IsSignedIn(i))) {
            if (!app.DLCInstallProcessCompleted() && !app.DLCInstallPending()) {
                app.StartInstallDLCProcess(i);
                break;
            }
        }
    }

    m_bIgnoreInput = false;

    
}

UIScene_SaveMessage::~UIScene_SaveMessage() {
    m_parentLayer->removeComponent(eUIComponent_Panorama);
    m_parentLayer->removeComponent(eUIComponent_Logo);
}

std::wstring UIScene_SaveMessage::getMoviePath() { return L"SaveMessage"; }

void UIScene_SaveMessage::updateTooltips() {
    ui.SetTooltips(DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT);
}

void UIScene_SaveMessage::handleInput(int iPad, int key, bool repeat,
                                      bool pressed, bool released,
                                      bool& handled) {
    if (m_bIgnoreInput) return;

    ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

    switch (key) {
        case ACTION_MENU_OK:
            sendInputToMovie(key, repeat, pressed, released);
            break;
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
    }
}

void UIScene_SaveMessage::handlePress(F64 controlId, F64 childId) {
    switch ((int)controlId) {
        case eControl_Confirm:

            
            ui.PlayUISFX(eSFX_Press);

            m_bIgnoreInput = true;

            ui.NavigateToHomeMenu();
            break;
    };
}

void UIScene_SaveMessage::handleTimerComplete(int id) {
    switch (id) {
        case PROFILE_LOADED_TIMER_ID: {
        }

        break;
    }
}
