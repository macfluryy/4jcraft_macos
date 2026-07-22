
#include "UIScene_PauseMenu.h"

#include <stddef.h>

#include <memory>

#include "platform/InputActions.h"
#include "platform/sdl2/Profile.h"
#include "app/common/App_enums.h"
#include "app/common/src/DLC/DLCManager.h"
#include "app/common/src/DLC/DLCPack.h"
#include "app/common/src/Network/GameNetworkManager.h"
#include "app/common/src/Tutorial/Tutorial.h"
#include "app/common/src/Tutorial/TutorialMode.h"
#include "app/common/src/UI/All Platforms/IUIScene_PauseMenu.h"
#include "app/common/src/UI/Controls/UIControl_Button.h"
#include "app/common/src/UI/UILayer.h"
#include "app/common/src/UI/UIScene.h"
#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/multiplayer/MultiPlayerLocalPlayer.h"
#include "minecraft/client/skins/DLCTexturePack.h"
#include "minecraft/client/skins/TexturePackRepository.h"
#include "minecraft/sounds/SoundTypes.h"
#include "strings.h"

class TexturePack;

UIScene_PauseMenu::UIScene_PauseMenu(int iPad, void* initData,
                                     UILayer* parentLayer)
    : UIScene(iPad, parentLayer) {
    
    initialiseMovie();
    m_bIgnoreInput = false;
    m_eAction = eAction_None;

    m_buttons[BUTTON_PAUSE_RESUMEGAME].init(app.GetString(IDS_RESUME_GAME),
                                            BUTTON_PAUSE_RESUMEGAME);
    m_buttons[BUTTON_PAUSE_HELPANDOPTIONS].init(
        app.GetString(IDS_HELP_AND_OPTIONS), BUTTON_PAUSE_HELPANDOPTIONS);
    m_buttons[BUTTON_PAUSE_LEADERBOARDS].init(app.GetString(IDS_LEADERBOARDS),
                                              BUTTON_PAUSE_LEADERBOARDS);
    m_buttons[BUTTON_PAUSE_ACHIEVEMENTS].init(app.GetString(IDS_ACHIEVEMENTS),
                                              BUTTON_PAUSE_ACHIEVEMENTS);
    m_buttons[BUTTON_PAUSE_SAVEGAME].init(app.GetString(IDS_SAVE_GAME),
                                          BUTTON_PAUSE_SAVEGAME);
    m_buttons[BUTTON_PAUSE_EXITGAME].init(app.GetString(IDS_EXIT_GAME),
                                          BUTTON_PAUSE_EXITGAME);

    updateControlsVisibility();

    doHorizontalResizeCheck();

    
    ui.HidePressStart();

#if TO_BE_IMPLEMENTED
    XuiSetTimer(m_hObj, IGNORE_KEYPRESS_TIMERID, IGNORE_KEYPRESS_TIME);
#endif

    
    
    
    if ( g_NetworkManager
            .GetPlayerCount() == 1) {
        app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),
                               eXuiServerAction_PauseServer, (void*)true);
    }

    Minecraft* pMinecraft = Minecraft::GetInstance();
    if (pMinecraft != nullptr && pMinecraft->localgameModes[iPad] != nullptr) {
        TutorialMode* gameMode =
            (TutorialMode*)pMinecraft->localgameModes[iPad];

        
        gameMode->getTutorial()->showTutorialPopup(false);
    }
    m_bErrorDialogRunning = false;
}

UIScene_PauseMenu::~UIScene_PauseMenu() {
    Minecraft* pMinecraft = Minecraft::GetInstance();
    if (pMinecraft != nullptr &&
        pMinecraft->localgameModes[m_iPad] != nullptr) {
        TutorialMode* gameMode =
            (TutorialMode*)pMinecraft->localgameModes[m_iPad];

        
        gameMode->getTutorial()->showTutorialPopup(true);
    }

    m_parentLayer->showComponent(m_iPad, eUIComponent_Panorama, false);
    m_parentLayer->showComponent(m_iPad, eUIComponent_MenuBackground, false);
    m_parentLayer->showComponent(m_iPad, eUIComponent_Logo, false);
}

std::wstring UIScene_PauseMenu::getMoviePath() {
    if (app.GetLocalPlayerCount() > 1) {
        return L"PauseMenuSplit";
    } else {
        return L"PauseMenu";
    }
}

void UIScene_PauseMenu::tick() { UIScene::tick(); }

void UIScene_PauseMenu::updateTooltips() {
    
    
    
    

    int iY = -1;
    int iRB = -1;
    int iX = -1;

    ui.SetTooltips(m_iPad, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, iX, iY, -1,
                   -1, -1, iRB);
}

void UIScene_PauseMenu::updateComponents() {
    m_parentLayer->showComponent(m_iPad, eUIComponent_Panorama, false);
    m_parentLayer->showComponent(m_iPad, eUIComponent_MenuBackground, true);

    if (app.GetLocalPlayerCount() == 1)
        m_parentLayer->showComponent(m_iPad, eUIComponent_Logo, true);
    else
        m_parentLayer->showComponent(m_iPad, eUIComponent_Logo, false);
}

void UIScene_PauseMenu::handlePreReload() {}

void UIScene_PauseMenu::handleReload() {
    updateTooltips();
    updateControlsVisibility();

    doHorizontalResizeCheck();
}

void UIScene_PauseMenu::updateControlsVisibility() {
    
    
    
    
    
    
    if (ProfileManager.GetPrimaryPad() ==
        m_iPad)  
    {
        
        
        if (app.GetLocalPlayerCount() > 1) {
            
            removeControl(&m_buttons[BUTTON_PAUSE_LEADERBOARDS], false);
            removeControl(&m_buttons[BUTTON_PAUSE_ACHIEVEMENTS], false);
        }

        if (!g_NetworkManager.IsHost()) {
            
            removeControl(&m_buttons[BUTTON_PAUSE_SAVEGAME], false);
        }
    } else {
        
        
        removeControl(&m_buttons[BUTTON_PAUSE_LEADERBOARDS], false);
        removeControl(&m_buttons[BUTTON_PAUSE_ACHIEVEMENTS], false);
        removeControl(&m_buttons[BUTTON_PAUSE_SAVEGAME], false);
    }

    
    if (StorageManager.GetSaveDisabled()) {
    }
}

void UIScene_PauseMenu::handleInput(int iPad, int key, bool repeat,
                                    bool pressed, bool released,
                                    bool& handled) {
    if (m_bIgnoreInput) {
        return;
    }

    
    
    
    ui.AnimateKeyPress(iPad, key, repeat, pressed, released);

    switch (key) {
        case ACTION_MENU_CANCEL:
            if (pressed) {
                
                
                
                if (iPad == ProfileManager.GetPrimaryPad() &&
                     g_NetworkManager
                            .GetPlayerCount() == 1) {
                    app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),
                                           eXuiServerAction_PauseServer,
                                           (void*)false);
                }

                ui.PlayUISFX(eSFX_Back);
                navigateBack();
            }
            break;
        case ACTION_MENU_OK:
        case ACTION_MENU_UP:
        case ACTION_MENU_DOWN:
            if (pressed) {
                sendInputToMovie(key, repeat, pressed, released);
            }
            break;

#if TO_BE_IMPLEMENTED
        case VK_PAD_X:
            
            if (bIsisPrimaryHost) {
                
                
                
                
                
                
                m_bIgnoreInput = true;

                StorageManager.SetSaveDevice(
                    &UIScene_PauseMenu::DeviceSelectReturned, this, true);
            }
            rfHandled = true;
            break;
#endif

        case ACTION_MENU_Y: {
#if TO_BE_IMPLEMENTED
            if (bUserisClientSide) {
                
                
                bool bCanScreenshot = true;
                for (int j = 0; j < XUSER_MAX_COUNT; ++j) {
                    if (app.GetXuiAction(j) ==
                        eAppAction_SocialPostScreenshot) {
                        bCanScreenshot = false;
                        break;
                    }
                }
                if (bCanScreenshot)
                    app.SetAction(pInputData->UserIndex, eAppAction_SocialPost);
            }
            rfHandled = true;
#endif
        } break;
    }
}

void UIScene_PauseMenu::handlePress(F64 controlId, F64 childId) {
    if (m_bIgnoreInput) return;

    switch ((int)controlId) {
        case BUTTON_PAUSE_RESUMEGAME:
            
            
            
            if (m_iPad == ProfileManager.GetPrimaryPad() &&
                 g_NetworkManager
                        .GetPlayerCount() == 1) {
                app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),
                                       eXuiServerAction_PauseServer,
                                       (void*)false);
            }
            navigateBack();
            break;
        case BUTTON_PAUSE_LEADERBOARDS: {
            unsigned int uiIDA[1];
            uiIDA[0] = IDS_OK;

            
            
            if (ProfileManager.IsGuest(m_iPad)) {
                ui.RequestAlertMessage(IDS_PRO_GUESTPROFILE_TITLE,
                                       IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1,
                                       ProfileManager.GetPrimaryPad());
            } else if (!ProfileManager.IsSignedInLive(m_iPad)) {
                unsigned int uiIDA[1] = {IDS_OK};
                ui.RequestErrorMessage(IDS_PRO_NOTONLINE_TITLE,
                                       IDS_PRO_NOTONLINE_TEXT, uiIDA, 1,
                                       m_iPad);
            } else {
                bool bContentRestricted = false;
                if (bContentRestricted) {
#if !defined(_WINDOWS64)
                    
                    
                    unsigned int uiIDA[1];
                    uiIDA[0] = IDS_CONFIRM_OK;
                    ui.RequestAlertMessage(IDS_ONLINE_SERVICE_TITLE,
                                           IDS_CONTENT_RESTRICTION, uiIDA, 1,
                                           m_iPad);
#endif
                } else {
                    ui.NavigateToScene(m_iPad, eUIScene_LeaderboardsMenu);
                }
            }
        } break;
        case BUTTON_PAUSE_ACHIEVEMENTS:
            
            if (ProfileManager.IsGuest(m_iPad)) {
                unsigned int uiIDA[1];
                uiIDA[0] = IDS_OK;
                ui.RequestAlertMessage(IDS_PRO_GUESTPROFILE_TITLE,
                                       IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1,
                                       ProfileManager.GetPrimaryPad());
            } else {
                
            }
            break;

        case BUTTON_PAUSE_HELPANDOPTIONS:
            ui.NavigateToScene(m_iPad, eUIScene_HelpAndOptionsMenu);
            break;
        case BUTTON_PAUSE_SAVEGAME:
            PerformActionSaveGame();
            break;
        case BUTTON_PAUSE_EXITGAME: {
            Minecraft* pMinecraft = Minecraft::GetInstance();
            unsigned int uiIDA[3];

            
            if (m_iPad == ProfileManager.GetPrimaryPad()) {
                int playTime = -1;
                if (pMinecraft->localplayers[m_iPad] != nullptr) {
                    playTime = (int)pMinecraft->localplayers[m_iPad]
                                   ->getSessionTimer();
                }

                if (StorageManager.GetSaveDisabled()) {
                    uiIDA[0] = IDS_CONFIRM_CANCEL;
                    uiIDA[1] = IDS_CONFIRM_OK;
                    ui.RequestAlertMessage(
                        IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_PROGRESS_LOST,
                        uiIDA, 2, m_iPad,
                        &IUIScene_PauseMenu::ExitGameDialogReturned,
                        (void*)GetCallbackUniqueId());
                } else {
                    if (g_NetworkManager.IsHost()) {
                        uiIDA[0] = IDS_CONFIRM_CANCEL;
                        uiIDA[1] = IDS_EXIT_GAME_SAVE;
                        uiIDA[2] = IDS_EXIT_GAME_NO_SAVE;

                        if (g_NetworkManager.GetPlayerCount() > 1) {
                            ui.RequestAlertMessage(
                                IDS_EXIT_GAME,
                                IDS_CONFIRM_EXIT_GAME_CONFIRM_DISCONNECT_SAVE,
                                uiIDA, 3, m_iPad,
                                &UIScene_PauseMenu::ExitGameSaveDialogReturned,
                                (void*)GetCallbackUniqueId());
                        } else {
                            ui.RequestAlertMessage(
                                IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 3,
                                m_iPad,
                                &UIScene_PauseMenu::ExitGameSaveDialogReturned,
                                (void*)GetCallbackUniqueId());
                        }
                    } else {
                        uiIDA[0] = IDS_CONFIRM_CANCEL;
                        uiIDA[1] = IDS_CONFIRM_OK;

                        ui.RequestAlertMessage(
                            IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 2,
                            m_iPad, &IUIScene_PauseMenu::ExitGameDialogReturned,
                            (void*)GetCallbackUniqueId());
                    }
                }
            } else {
                int playTime = -1;
                if (pMinecraft->localplayers[m_iPad] != nullptr) {
                    playTime = (int)pMinecraft->localplayers[m_iPad]
                                   ->getSessionTimer();
                }

                
                app.SetAction(m_iPad, eAppAction_ExitPlayer);
            }
        } break;
    }
}

void UIScene_PauseMenu::PerformActionSaveGame() {
    
    
    if (!Minecraft::GetInstance()->skins->isUsingDefaultSkin()) {
        TexturePack* tPack = Minecraft::GetInstance()->skins->getSelected();
        DLCTexturePack* pDLCTexPack = (DLCTexturePack*)tPack;

        m_pDLCPack =
            pDLCTexPack->getDLCInfoParentPack();  

        if (!m_pDLCPack->hasPurchasedFile(DLCManager::e_DLCType_Texture, L"")) {
            
            unsigned int uiIDA[2];
            uiIDA[0] = IDS_CONFIRM_OK;
            uiIDA[1] = IDS_CONFIRM_CANCEL;

            
            
            {
                ui.RequestAlertMessage(
                    IDS_WARNING_DLC_TRIALTEXTUREPACK_TITLE,
                    IDS_WARNING_DLC_TRIALTEXTUREPACK_TEXT, uiIDA, 2, m_iPad,
                    &UIScene_PauseMenu::WarningTrialTexturePackReturned,
                    (void*)GetCallbackUniqueId());
            }

            return;
        } else {
            m_bTrialTexturePack = false;
        }
    }

    
    bool bSaveExists;
    C4JStorage::ESaveGameState result =
        StorageManager.DoesSaveExist(&bSaveExists);

    {
        
        
        if (bSaveExists) {
            unsigned int uiIDA[2];
            uiIDA[0] = IDS_CONFIRM_CANCEL;
            uiIDA[1] = IDS_CONFIRM_OK;
            ui.RequestAlertMessage(IDS_TITLE_SAVE_GAME, IDS_CONFIRM_SAVE_GAME,
                                   uiIDA, 2, m_iPad,
                                   &IUIScene_PauseMenu::SaveGameDialogReturned,
                                   (void*)GetCallbackUniqueId());
        } else {
            
            app.SetAction(m_iPad, eAppAction_SaveGame);
        }
    }
}

void UIScene_PauseMenu::ShowScene(bool show) {
    app.DebugPrintf("UIScene_PauseMenu::ShowScene is not implemented\n");
}

void UIScene_PauseMenu::HandleDLCInstalled() {
    
    if (app.StartInstallDLCProcess(m_iPad) == false) {
        
        
        app.DebugPrintf(
            "UIScene_PauseMenu::HandleDLCInstalled - m_bIgnoreInput false\n");
    } else {
        
        
        
        app.DebugPrintf(
            "UIScene_PauseMenu::HandleDLCInstalled - m_bIgnoreInput true\n");
    }
    
}

void UIScene_PauseMenu::HandleDLCMountingComplete() {
    

    
    app.DebugPrintf(
        "UIScene_PauseMenu::HandleDLCMountingComplete - m_bIgnoreInput false "
        "\n");
}

int UIScene_PauseMenu::UnlockFullSaveReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    Minecraft* pMinecraft = Minecraft::GetInstance();

    return 0;
}

int UIScene_PauseMenu::SaveGame_SignInReturned(void* pParam, bool bContinue,
                                               int iPad) {
    UIScene_PauseMenu* pClass =
        (UIScene_PauseMenu*)ui.GetSceneFromCallbackId((size_t)pParam);
    if (pClass) pClass->SetIgnoreInput(false);

    if (bContinue == true) {
        if (pClass) pClass->PerformActionSaveGame();
    }

    return 0;
}

void UIScene_PauseMenu::SetIgnoreInput(bool ignoreInput) {
    m_bIgnoreInput = ignoreInput;
}
