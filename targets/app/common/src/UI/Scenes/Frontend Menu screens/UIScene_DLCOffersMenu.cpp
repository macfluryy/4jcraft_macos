
#include "UIScene_DLCOffersMenu.h"

#include <stdint.h>

#include "platform/PlatformTypes.h"
#include "platform/InputActions.h"
#include "platform/sdl2/Render.h"
#include "app/common/src/UI/All Platforms/UIStructs.h"
#include "app/common/src/UI/Controls/UIControl_DLCList.h"
#include "app/common/src/UI/Controls/UIControl_HTMLLabel.h"
#include "app/common/src/UI/Controls/UIControl_Label.h"
#include "app/common/src/UI/UIScene.h"
#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"
#include "strings.h"

class UILayer;

#define PLAYER_ONLINE_TIMER_ID 0
#define PLAYER_ONLINE_TIMER_TIME 100

UIScene_DLCOffersMenu::UIScene_DLCOffersMenu(int iPad, void* initData,
                                             UILayer* parentLayer)
    : UIScene(iPad, parentLayer) {
    m_bProductInfoShown = false;
    DLCOffersParam* param = (DLCOffersParam*)initData;
    m_iProductInfoIndex = param->iType;
    m_iCurrentDLC = 0;
    m_iTotalDLC = 0;
    m_bAddAllDLCButtons = true;

    
    initialiseMovie();
    
    app.SetLiveLinkRequired(true);

    m_bIsSD = !RenderManager.IsHiDef() && !RenderManager.IsWidescreen();

    m_labelOffers.init(app.GetString(IDS_DOWNLOADABLE_CONTENT_OFFERS));
    m_buttonListOffers.init(eControl_OffersList);
    m_labelHTMLSellText.init(L" ");
    m_labelPriceTag.init(L" ");

    m_bHasPurchased = false;
    m_bIsSelected = false;

    if (m_loadedResolution == eSceneResolution_1080) {
        m_labelXboxStore.init(L"");
    }
}

UIScene_DLCOffersMenu::~UIScene_DLCOffersMenu() {
    
    
    app.SetLiveLinkRequired(false);
}

void UIScene_DLCOffersMenu::handleTimerComplete(int id) {}

int UIScene_DLCOffersMenu::ExitDLCOffersMenu(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    UIScene_DLCOffersMenu* pClass = (UIScene_DLCOffersMenu*)pParam;

    ui.NavigateToHomeMenu();  

    return 0;
}

std::wstring UIScene_DLCOffersMenu::getMoviePath() { return L"DLCOffersMenu"; }

void UIScene_DLCOffersMenu::updateTooltips() {
    int iA = -1;
    if (m_bIsSelected) {
        if (!m_bHasPurchased) {
            iA = IDS_TOOLTIPS_INSTALL;
        } else {
            iA = IDS_TOOLTIPS_REINSTALL;
        }
    }
    ui.SetTooltips(m_iPad, iA, IDS_TOOLTIPS_BACK);
}

void UIScene_DLCOffersMenu::handleInput(int iPad, int key, bool repeat,
                                        bool pressed, bool released,
                                        bool& handled) {
    
    
    
    ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

    switch (key) {
        case ACTION_MENU_CANCEL:
            if (pressed) {
                navigateBack();
            }
            break;
        case ACTION_MENU_OK:
            sendInputToMovie(key, repeat, pressed, released);
            break;
        case ACTION_MENU_UP:
            if (pressed) {
                
                
                if (m_iTotalDLC > 0) {
                    if (m_iCurrentDLC > 0) m_iCurrentDLC--;

                    m_bProductInfoShown = false;
                }
            }
            sendInputToMovie(key, repeat, pressed, released);
            break;

        case ACTION_MENU_DOWN:
            if (pressed) {
                
                
                if (m_iTotalDLC > 0) {
                    if (m_iCurrentDLC < (m_iTotalDLC - 1)) m_iCurrentDLC++;

                    m_bProductInfoShown = false;
                }
            }
            sendInputToMovie(key, repeat, pressed, released);
            break;

        case ACTION_MENU_LEFT:
            










































        case ACTION_MENU_RIGHT:
        case ACTION_MENU_OTHER_STICK_DOWN:
        case ACTION_MENU_OTHER_STICK_UP:
            
            
            
            
            
            sendInputToMovie(key, repeat, pressed, released);
            break;
    }
}

void UIScene_DLCOffersMenu::handlePress(F64 controlId, F64 childId) {
    switch ((int)controlId) {
        case eControl_OffersList: {
            int iIndex = (int)childId;

            uint64_t ullIndexA[1];
            ullIndexA[0] = StorageManager.GetOffer(iIndex).qwOfferID;
            StorageManager.InstallOffer(1, ullIndexA, nullptr);
        } break;
    }
}

void UIScene_DLCOffersMenu::handleSelectionChanged(F64 selectedId) {}

void UIScene_DLCOffersMenu::handleFocusChange(F64 controlId, F64 childId) {
    app.DebugPrintf("UIScene_DLCOffersMenu::handleFocusChange\n");
}

void UIScene_DLCOffersMenu::tick() { UIScene::tick(); }
