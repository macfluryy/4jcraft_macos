
#include "UIScene_HowToPlayMenu.h"

#include <stdint.h>

#include "platform/InputActions.h"
#include "app/common/src/UI/Controls/UIControl_ButtonList.h"
#include "app/common/src/UI/UILayer.h"
#include "app/common/src/UI/UIScene.h"
#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/sounds/SoundTypes.h"
#include "strings.h"


unsigned int UIScene_HowToPlayMenu::m_uiHTPButtonNameA[] = {
    IDS_HOW_TO_PLAY_MENU_WHATSNEW,     
    IDS_HOW_TO_PLAY_MENU_BASICS,       
    IDS_HOW_TO_PLAY_MENU_MULTIPLAYER,  
    IDS_HOW_TO_PLAY_MENU_HUD,          
    IDS_HOW_TO_PLAY_MENU_CREATIVE,     
    IDS_HOW_TO_PLAY_MENU_INVENTORY,    
    IDS_HOW_TO_PLAY_MENU_CHESTS,       
    IDS_HOW_TO_PLAY_MENU_CRAFTING,     
    IDS_HOW_TO_PLAY_MENU_FURNACE,      
    IDS_HOW_TO_PLAY_MENU_DISPENSER,    

    IDS_HOW_TO_PLAY_MENU_BREWING,      
    IDS_HOW_TO_PLAY_MENU_ENCHANTMENT,  
    IDS_HOW_TO_PLAY_MENU_ANVIL,
    IDS_HOW_TO_PLAY_MENU_FARMANIMALS,   
    IDS_HOW_TO_PLAY_MENU_BREEDANIMALS,  
    IDS_HOW_TO_PLAY_MENU_TRADING,

    IDS_HOW_TO_PLAY_MENU_HORSES,       IDS_HOW_TO_PLAY_MENU_BEACONS,
    IDS_HOW_TO_PLAY_MENU_FIREWORKS,    IDS_HOW_TO_PLAY_MENU_HOPPERS,
    IDS_HOW_TO_PLAY_MENU_DROPPERS,

    IDS_HOW_TO_PLAY_MENU_NETHERPORTAL,  
    IDS_HOW_TO_PLAY_MENU_THEEND,        
    IDS_HOW_TO_PLAY_MENU_HOSTOPTIONS,   
};


unsigned int UIScene_HowToPlayMenu::m_uiHTPSceneA[] = {
    eHowToPlay_WhatsNew,     eHowToPlay_Basics,
    eHowToPlay_Multiplayer,  eHowToPlay_HUD,
    eHowToPlay_Creative,     eHowToPlay_Inventory,
    eHowToPlay_Chest,        eHowToPlay_InventoryCrafting,
    eHowToPlay_Furnace,      eHowToPlay_Dispenser,

    eHowToPlay_Brewing,      eHowToPlay_Enchantment,
    eHowToPlay_Anvil,        eHowToPlay_FarmingAnimals,
    eHowToPlay_Breeding,     eHowToPlay_Trading,

    eHowToPlay_Horses,       eHowToPlay_Beacons,
    eHowToPlay_Fireworks,    eHowToPlay_Hoppers,
    eHowToPlay_Droppers,

    eHowToPlay_NetherPortal, eHowToPlay_TheEnd,
    eHowToPlay_HostOptions,
};

UIScene_HowToPlayMenu::UIScene_HowToPlayMenu(int iPad, void* initData,
                                             UILayer* parentLayer)
    : UIScene(iPad, parentLayer) {
    
    initialiseMovie();

    m_buttonListHowTo.init(eControl_Buttons);

    for (unsigned int i = 0; i < eHTPButton_Max; ++i) {
        
        {
            m_buttonListHowTo.addItem(app.GetString(m_uiHTPButtonNameA[i]),
                                      i);  
        }
    }

    doHorizontalResizeCheck();
}

std::wstring UIScene_HowToPlayMenu::getMoviePath() {
    if (app.GetLocalPlayerCount() > 1) {
        return L"HowToPlayMenuSplit";
    } else {
        return L"HowToPlayMenu";
    }
}

void UIScene_HowToPlayMenu::updateTooltips() {
    ui.SetTooltips(m_iPad, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK);
}

void UIScene_HowToPlayMenu::updateComponents() {
    bool bNotInGame = (Minecraft::GetInstance()->level == nullptr);
    if (bNotInGame) {
        m_parentLayer->showComponent(m_iPad, eUIComponent_Panorama, true);
        m_parentLayer->showComponent(m_iPad, eUIComponent_Logo, true);
    } else {
        m_parentLayer->showComponent(m_iPad, eUIComponent_Panorama, false);

        if (app.GetLocalPlayerCount() == 1)
            m_parentLayer->showComponent(m_iPad, eUIComponent_Logo, true);
        else
            m_parentLayer->showComponent(m_iPad, eUIComponent_Logo, false);
    }
}

void UIScene_HowToPlayMenu::handleReload() {
    for (unsigned int i = 0; i < eHTPButton_Max; ++i) {
        
        {
            m_buttonListHowTo.addItem(app.GetString(m_uiHTPButtonNameA[i]), i);
        }
    }

    doHorizontalResizeCheck();
}

void UIScene_HowToPlayMenu::handleInput(int iPad, int key, bool repeat,
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
        case ACTION_MENU_DOWN:
        case ACTION_MENU_PAGEUP:
        case ACTION_MENU_PAGEDOWN:
            sendInputToMovie(key, repeat, pressed, released);
            break;
    }
}

void UIScene_HowToPlayMenu::handlePress(F64 controlId, F64 childId) {
    if ((int)controlId == eControl_Buttons) {
        
        ui.PlayUISFX(eSFX_Press);

        unsigned int uiInitData;
        uiInitData =
            ((1 << 31) | (m_uiHTPSceneA[(int)childId] << 16) | (short)(m_iPad));
        ui.NavigateToScene(
            m_iPad, eUIScene_HowToPlay,
            reinterpret_cast<void*>(static_cast<uintptr_t>(uiInitData)));
    }
}
