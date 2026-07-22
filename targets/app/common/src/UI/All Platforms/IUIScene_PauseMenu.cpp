#include "IUIScene_PauseMenu.h"

#include <stdint.h>

#include <chrono>
#include <cstddef>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "platform/sdl2/Profile.h"
#include "app/common/App_enums.h"
#include "app/common/src/DLC/DLCManager.h"
#include "app/common/src/DLC/DLCPack.h"
#include "app/common/src/GameRules/GameRuleManager.h"
#include "app/common/src/Network/GameNetworkManager.h"
#include "app/common/src/UI/UIScene.h"
#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"
#include "app/mac/Stubs/winapi_stubs.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/compression.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/ProgressRenderer.h"
#include "minecraft/client/multiplayer/MultiPlayerLevel.h"
#include "minecraft/client/skins/DLCTexturePack.h"
#include "minecraft/client/skins/TexturePackRepository.h"
#include "minecraft/network/packet/DisconnectPacket.h"
#include "minecraft/client/multiplayer/DisconnectedScreen.h"
#include "minecraft/server/MinecraftServer.h"
#include "strings.h"

class TexturePack;

int IUIScene_PauseMenu::ExitGameDialogReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    IUIScene_PauseMenu* pScene = dynamic_cast<IUIScene_PauseMenu*>(
        ui.GetSceneFromCallbackId((std::size_t)pParam));

    
    if (result == C4JStorage::EMessage_ResultDecline) {
        if (pScene) pScene->SetIgnoreInput(true);
        app.SetAction(iPad, eAppAction_ExitWorld);
    }
    return 0;
}

int IUIScene_PauseMenu::ExitGameSaveDialogReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    IUIScene_PauseMenu* pScene = dynamic_cast<IUIScene_PauseMenu*>(
        ui.GetSceneFromCallbackId((std::size_t)pParam));

    
    
    if (result == C4JStorage::EMessage_ResultDecline ||
        result == C4JStorage::EMessage_ResultThirdOption) {
        if (result == C4JStorage::EMessage_ResultDecline)  
        {
            
            
            if (!Minecraft::GetInstance()->skins->isUsingDefaultSkin()) {
                TexturePack* tPack =
                    Minecraft::GetInstance()->skins->getSelected();
                DLCTexturePack* pDLCTexPack = (DLCTexturePack*)tPack;

                DLCPack* pDLCPack =
                    pDLCTexPack
                        ->getDLCInfoParentPack();  
                if (!pDLCPack->hasPurchasedFile(DLCManager::e_DLCType_Texture,
                                                L"")) {
                    unsigned int uiIDA[2];
                    uiIDA[0] = IDS_CONFIRM_OK;
                    uiIDA[1] = IDS_CONFIRM_CANCEL;

                    
                    
                    ui.RequestAlertMessage(
                        IDS_WARNING_DLC_TRIALTEXTUREPACK_TITLE,
                        IDS_WARNING_DLC_TRIALTEXTUREPACK_TEXT, uiIDA, 2,
                        ProfileManager.GetPrimaryPad(),
                        &IUIScene_PauseMenu::WarningTrialTexturePackReturned,
                        pParam);

                    return 0;
                }
            }

            
            bool bSaveExists;
            StorageManager.DoesSaveExist(&bSaveExists);
            
            
            
            if (bSaveExists) {
                unsigned int uiIDA[2];
                uiIDA[0] = IDS_CONFIRM_CANCEL;
                uiIDA[1] = IDS_CONFIRM_OK;
                ui.RequestAlertMessage(
                    IDS_TITLE_SAVE_GAME, IDS_CONFIRM_SAVE_GAME, uiIDA, 2,
                    ProfileManager.GetPrimaryPad(),
                    &IUIScene_PauseMenu::ExitGameAndSaveReturned, pParam);
                return 0;
            } else {
                MinecraftServer::getInstance()->setSaveOnExit(true);
            }
        } else {
            
            unsigned int uiIDA[2];
            uiIDA[0] = IDS_CONFIRM_CANCEL;
            uiIDA[1] = IDS_CONFIRM_OK;
            ui.RequestAlertMessage(
                IDS_TITLE_DECLINE_SAVE_GAME, IDS_CONFIRM_DECLINE_SAVE_GAME,
                uiIDA, 2, ProfileManager.GetPrimaryPad(),
                &IUIScene_PauseMenu::ExitGameDeclineSaveReturned, pParam);
            return 0;
        }

        if (pScene) pScene->SetIgnoreInput(true);

        app.SetAction(iPad, eAppAction_ExitWorld);
    }
    return 0;
}

int IUIScene_PauseMenu::ExitGameAndSaveReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    
    IUIScene_PauseMenu* pScene = dynamic_cast<IUIScene_PauseMenu*>(
        ui.GetSceneFromCallbackId((std::size_t)pParam));

    
    if (result == C4JStorage::EMessage_ResultDecline) {
        
        
        
        
        
        if (pScene) pScene->SetIgnoreInput(true);
        MinecraftServer::getInstance()->setSaveOnExit(true);
        
        app.SetAction(iPad, eAppAction_ExitWorld);
    } else {
        
        
        if (ui.IsPauseMenuDisplayed(ProfileManager.GetPrimaryPad())) {
            unsigned int uiIDA[3];
            
            
            uiIDA[0] = IDS_CONFIRM_CANCEL;
            uiIDA[1] = IDS_EXIT_GAME_SAVE;
            uiIDA[2] = IDS_EXIT_GAME_NO_SAVE;

            if (g_NetworkManager.GetPlayerCount() > 1) {
                ui.RequestAlertMessage(
                    IDS_EXIT_GAME,
                    IDS_CONFIRM_EXIT_GAME_CONFIRM_DISCONNECT_SAVE, uiIDA, 3,
                    ProfileManager.GetPrimaryPad(),
                    &IUIScene_PauseMenu::ExitGameSaveDialogReturned, pParam);
            } else {
                ui.RequestAlertMessage(
                    IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 3,
                    ProfileManager.GetPrimaryPad(),
                    &IUIScene_PauseMenu::ExitGameSaveDialogReturned, pParam);
            }
        }
    }
    return 0;
}

int IUIScene_PauseMenu::ExitGameDeclineSaveReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    IUIScene_PauseMenu* pScene = dynamic_cast<IUIScene_PauseMenu*>(
        ui.GetSceneFromCallbackId((std::size_t)pParam));

    
    if (result == C4JStorage::EMessage_ResultDecline) {
        if (pScene) pScene->SetIgnoreInput(true);
        MinecraftServer::getInstance()->setSaveOnExit(false);
        
        app.SetAction(iPad, eAppAction_ExitWorld);
    } else {
        
        
        if (ui.IsPauseMenuDisplayed(ProfileManager.GetPrimaryPad())) {
            unsigned int uiIDA[3];
            
            
            uiIDA[0] = IDS_CONFIRM_CANCEL;
            uiIDA[1] = IDS_EXIT_GAME_SAVE;
            uiIDA[2] = IDS_EXIT_GAME_NO_SAVE;

            if (g_NetworkManager.GetPlayerCount() > 1) {
                ui.RequestAlertMessage(
                    IDS_EXIT_GAME,
                    IDS_CONFIRM_EXIT_GAME_CONFIRM_DISCONNECT_SAVE, uiIDA, 3,
                    ProfileManager.GetPrimaryPad(),
                    &IUIScene_PauseMenu::ExitGameSaveDialogReturned, pParam);
            } else {
                ui.RequestAlertMessage(
                    IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 3,
                    ProfileManager.GetPrimaryPad(),
                    &IUIScene_PauseMenu::ExitGameSaveDialogReturned, pParam);
            }
        }
    }
    return 0;
}

int IUIScene_PauseMenu::WarningTrialTexturePackReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    return 0;
}

int IUIScene_PauseMenu::SaveWorldThreadProc(void* lpParameter) {
    bool bAutosave = (bool)lpParameter;
    if (bAutosave) {
        app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),
                               eXuiServerAction_AutoSaveGame);
    } else {
        app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),
                               eXuiServerAction_SaveGame);
    }

    
    
    Compression::UseDefaultThreadStorage();

    Minecraft* pMinecraft = Minecraft::GetInstance();

    

    app.SetGameStarted(false);

    while (app.GetXuiServerAction(ProfileManager.GetPrimaryPad()) !=
               eXuiServerAction_Idle &&
           !MinecraftServer::serverHalted()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (!MinecraftServer::serverHalted() && !app.GetChangingSessionType())
        app.SetGameStarted(true);

    int32_t hr = 0;
    if (app.GetChangingSessionType()) {
        
        
        hr = ERROR_CANCELLED;
    }
    return hr;
}

int IUIScene_PauseMenu::ExitWorldThreadProc(void* lpParameter) {
    
    
    Compression::UseDefaultThreadStorage();

    

    _ExitWorld(lpParameter);

    return 0;
}



void IUIScene_PauseMenu::_ExitWorld(void* lpParameter) {
    Minecraft* pMinecraft = Minecraft::GetInstance();

    int exitReasonStringId = pMinecraft->progressRenderer->getCurrentTitle();
    int exitReasonTitleId = IDS_CONNECTION_LOST;

    bool saveStats = true;
    if (pMinecraft->isClientSide() || g_NetworkManager.IsInSession()) {
        if (lpParameter != nullptr) {
            
            
            
            
            
            switch (app.GetDisconnectReason()) {
                case DisconnectPacket::eDisconnect_Kicked:
                    exitReasonStringId = IDS_DISCONNECTED_KICKED;
                    break;
                case DisconnectPacket::eDisconnect_NoUGC_AllLocal:
                    exitReasonStringId =
                        IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_ALL_LOCAL;
                    exitReasonTitleId = IDS_CONNECTION_FAILED;
                    break;
                case DisconnectPacket::eDisconnect_NoUGC_Single_Local:
                    exitReasonStringId =
                        IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_SINGLE_LOCAL;
                    exitReasonTitleId = IDS_CONNECTION_FAILED;
                    break;
                case DisconnectPacket::eDisconnect_NoFlying:
                    exitReasonStringId = IDS_DISCONNECTED_FLYING;
                    break;
                case DisconnectPacket::eDisconnect_Quitting:
                    exitReasonStringId = IDS_DISCONNECTED_SERVER_QUIT;
                    break;
                case DisconnectPacket::eDisconnect_NoFriendsInGame:
                    exitReasonStringId = IDS_DISCONNECTED_NO_FRIENDS_IN_GAME;
                    exitReasonTitleId = IDS_CANTJOIN_TITLE;
                    break;
                case DisconnectPacket::eDisconnect_Banned:
                    exitReasonStringId = IDS_DISCONNECTED_BANNED;
                    exitReasonTitleId = IDS_CANTJOIN_TITLE;
                    break;
                case DisconnectPacket::eDisconnect_NotFriendsWithHost:
                    exitReasonStringId = IDS_NOTALLOWED_FRIENDSOFFRIENDS;
                    exitReasonTitleId = IDS_CANTJOIN_TITLE;
                    break;
                case DisconnectPacket::eDisconnect_OutdatedServer:
                    exitReasonStringId = IDS_DISCONNECTED_SERVER_OLD;
                    exitReasonTitleId = IDS_CANTJOIN_TITLE;
                    break;
                case DisconnectPacket::eDisconnect_OutdatedClient:
                    exitReasonStringId = IDS_DISCONNECTED_CLIENT_OLD;
                    exitReasonTitleId = IDS_CANTJOIN_TITLE;
                    break;
                case DisconnectPacket::eDisconnect_ServerFull:
                    exitReasonStringId = IDS_DISCONNECTED_SERVER_FULL;
                    exitReasonTitleId = IDS_CANTJOIN_TITLE;
                    break;

                default:
                    exitReasonStringId = IDS_CONNECTION_LOST_SERVER;
            }
            
            
            

            unsigned int uiIDA[1];
            uiIDA[0] = IDS_CONFIRM_OK;
            
            
            
            
            
            
            if (app.GetDisconnectReasonText().empty() &&
                ProfileManager.IsSignedIn(ProfileManager.GetPrimaryPad())) {
                ui.RequestErrorMessage(exitReasonTitleId, exitReasonStringId,
                                       uiIDA, 1,
                                       ProfileManager.GetPrimaryPad());
            }
            exitReasonStringId = -1;

            
            
            if (pMinecraft->levels[0] != nullptr)
                pMinecraft->levels[0]->disconnect(false);
            if (pMinecraft->levels[1] != nullptr)
                pMinecraft->levels[1]->disconnect(false);
            if (pMinecraft->levels[2] != nullptr)
                pMinecraft->levels[2]->disconnect(false);
        } else {
            exitReasonStringId = IDS_EXITING_GAME;
            pMinecraft->progressRenderer->progressStartNoAbort(
                IDS_EXITING_GAME);
            if (pMinecraft->levels[0] != nullptr)
                pMinecraft->levels[0]->disconnect();
            if (pMinecraft->levels[1] != nullptr)
                pMinecraft->levels[1]->disconnect();
            if (pMinecraft->levels[2] != nullptr)
                pMinecraft->levels[2]->disconnect();
        }

        
        
        MinecraftServer::HaltServer();

        
        
        
        saveStats = false;

        
        g_NetworkManager.LeaveGame(false);
    } else {
        if (lpParameter != nullptr &&
            ProfileManager.IsSignedIn(ProfileManager.GetPrimaryPad())) {
            switch (app.GetDisconnectReason()) {
                case DisconnectPacket::eDisconnect_Kicked:
                    exitReasonStringId = IDS_DISCONNECTED_KICKED;
                    break;
                case DisconnectPacket::eDisconnect_NoUGC_AllLocal:
                    exitReasonStringId =
                        IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_ALL_LOCAL;
                    exitReasonTitleId = IDS_CONNECTION_FAILED;
                    break;
                case DisconnectPacket::eDisconnect_NoUGC_Single_Local:
                    exitReasonStringId =
                        IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_SINGLE_LOCAL;
                    exitReasonTitleId = IDS_CONNECTION_FAILED;
                    break;
                case DisconnectPacket::eDisconnect_Quitting:
                    exitReasonStringId = IDS_DISCONNECTED_SERVER_QUIT;
                    break;
                case DisconnectPacket::eDisconnect_NoMultiplayerPrivilegesJoin:
                    exitReasonStringId = IDS_NO_MULTIPLAYER_PRIVILEGE_JOIN_TEXT;
                    break;
                case DisconnectPacket::eDisconnect_OutdatedServer:
                    exitReasonStringId = IDS_DISCONNECTED_SERVER_OLD;
                    exitReasonTitleId = IDS_CANTJOIN_TITLE;
                    break;
                case DisconnectPacket::eDisconnect_OutdatedClient:
                    exitReasonStringId = IDS_DISCONNECTED_CLIENT_OLD;
                    exitReasonTitleId = IDS_CANTJOIN_TITLE;
                    break;
                case DisconnectPacket::eDisconnect_ServerFull:
                    exitReasonStringId = IDS_DISCONNECTED_SERVER_FULL;
                    exitReasonTitleId = IDS_CANTJOIN_TITLE;
                    break;
                default:
                    exitReasonStringId = IDS_DISCONNECTED;
            }
            
            

            unsigned int uiIDA[1];
            uiIDA[0] = IDS_CONFIRM_OK;
            ui.RequestErrorMessage(exitReasonTitleId, exitReasonStringId, uiIDA,
                                   1, ProfileManager.GetPrimaryPad());
            exitReasonStringId = -1;
        }
    }
    
    
    
    while (g_NetworkManager.IsNetworkThreadRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    pMinecraft->setLevel(nullptr, exitReasonStringId, nullptr, saveStats);

    app.m_gameRules.unloadCurrentGameRules();
    

    MinecraftServer::resetFlags();

    
    
    
    pMinecraft->progressRenderer->progressStart(IDS_EXITING_GAME);

    
    
    
    while (g_NetworkManager.IsInSession()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    app.SetChangingSessionType(false);
    app.SetReallyChangingSessionType(false);
    pMinecraft->exitingWorldRightNow = false;
}

int IUIScene_PauseMenu::SaveGameDialogReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    
    if (result == C4JStorage::EMessage_ResultDecline) {
        
        app.SetAction(iPad, eAppAction_SaveGame);
    }
    return 0;
}

int IUIScene_PauseMenu::EnableAutosaveDialogReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    
    if (result == C4JStorage::EMessage_ResultDecline) {
        
        
        app.SetGameHostOption(eGameHostOption_DisableSaving, 0);
    } else {
        
        
        
        app.SetGameHostOption(eGameHostOption_DisableSaving, 1);
    }
    
    StorageManager.SetSaveDisabled(false);

    
    app.SetAction(iPad, eAppAction_SaveGame);
    return 0;
}

int IUIScene_PauseMenu::DisableAutosaveDialogReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    
    if (result == C4JStorage::EMessage_ResultDecline) {
        
        
        app.SetGameHostOption(eGameHostOption_DisableSaving, 1);
        StorageManager.SetSaveDisabled(false);

        
        app.SetAction(iPad, eAppAction_SaveGame);
    }
    return 0;
}