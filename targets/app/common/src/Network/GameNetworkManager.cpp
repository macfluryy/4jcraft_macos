#include "GameNetworkManager.h"

#include <assert.h>

#include <algorithm>
#include <chrono>
#include <compare>
#include <memory>
#include <thread>
#include <vector>

#include "platform/sdl2/Input.h"
#include "platform/sdl2/Profile.h"
#include "platform/sdl2/Render.h"
#include "platform/sdl2/Storage.h"
#include "app/common/App_enums.h"
#include "app/common/Game.h"
#include "app/common/src/GameRules/GameRuleManager.h"
#include "app/common/src/GameRules/LevelGeneration/LevelGenerationOptions.h"
#include "app/common/src/Network/NetworkPlayerInterface.h"
#include "app/common/src/Network/PlatformNetworkManagerStub.h"
#include "app/common/src/UI/All Platforms/UIEnums.h"
#include "app/common/src/UI/All Platforms/UIStructs.h"
#include "app/common/src/UI/Scenes/In-Game Menu Screens/UIScene_PauseMenu.h"
#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"
#include "app/mac/Stubs/winapi_stubs.h"
#include "Socket.h"
#include "XboxStubs.h"
#include "util/StringHelpers.h"
#include "platform/PlatformServices.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/compression.h"
#include "java/File.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/ProgressRenderer.h"
#include "minecraft/client/User.h"
#include "minecraft/client/gui/Gui.h"
#include "minecraft/client/multiplayer/ClientConnection.h"
#include "minecraft/client/multiplayer/MultiPlayerLocalPlayer.h"
#include "minecraft/client/renderer/LevelRenderer.h"
#include "minecraft/client/skins/TexturePack.h"
#include "minecraft/client/skins/TexturePackRepository.h"
#include "minecraft/network/Connection.h"
#include "minecraft/network/packet/DisconnectPacket.h"
#include "minecraft/network/packet/PreLoginPacket.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/server/PlayerList.h"
#include "minecraft/server/level/ServerPlayer.h"
#include "minecraft/server/network/PlayerConnection.h"
#include "minecraft/world/entity/Entity.h"
#include "minecraft/world/item/crafting/FireworksRecipe.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/chunk/storage/OldChunkStorage.h"
#include "minecraft/world/level/tile/Tile.h"
#include "strings.h"

class FriendSessionInfo;
class INVITE_INFO;


CGameNetworkManager g_NetworkManager;
IPlatformNetwork* CGameNetworkManager::s_pPlatformNetworkManager;

int64_t CGameNetworkManager::messageQueue[512];
int64_t CGameNetworkManager::byteQueue[512];
int CGameNetworkManager::messageQueuePos = 0;

CGameNetworkManager::CGameNetworkManager() {
    m_bInitialised = false;
    m_bLastDisconnectWasLostRoomOnly = false;
    m_bFullSessionMessageOnNextSessionChange = false;
}

void CGameNetworkManager::Initialise() {
    ServerStoppedCreate(false);
    ServerReadyCreate(false);
    int flagIndexSize =
        LevelRenderer::getGlobalChunkCount() /
        (Level::maxBuildHeight /
         16);  
    s_pPlatformNetworkManager = new IPlatformNetworkStub();
    s_pPlatformNetworkManager->Initialise(this, flagIndexSize);
    m_bNetworkThreadRunning = false;
    m_bInitialised = true;
}

void CGameNetworkManager::Terminate() {
    if (m_bInitialised) {
        s_pPlatformNetworkManager->Terminate();
    }
}

void CGameNetworkManager::DoWork() { s_pPlatformNetworkManager->DoWork(); }

bool CGameNetworkManager::_RunNetworkGame(void* lpParameter) {
    bool success = true;

    bool isHost = g_NetworkManager.IsHost();
    
    Minecraft* pMinecraft = Minecraft::GetInstance();
    success = StartNetworkGame(pMinecraft, lpParameter);

    if (!success) return false;

    if (isHost) {
        
        

        success = s_pPlatformNetworkManager->_RunNetworkGame();
        if (!success) {
            app.SetAction(ProfileManager.GetPrimaryPad(), eAppAction_ExitWorld,
                          (void*)true);
            return true;
        }
    }

    if (g_NetworkManager.IsLeavingGame()) return false;

    app.SetGameStarted(true);

    

    return success;
}

bool CGameNetworkManager::StartNetworkGame(Minecraft* minecraft,
                                           void* lpParameter) {
    int64_t seed = 0;
    if (lpParameter != nullptr) {
        NetworkGameInitData* param = (NetworkGameInitData*)lpParameter;
        seed = param->seed;

        app.setLevelGenerationOptions(param->levelGen);
        if (param->levelGen != nullptr) {
            if (app.getLevelGenerationOptions() == nullptr) {
                app.DebugPrintf(
                    "Game rule was not loaded, and seed is required. "
                    "Exiting.\n");
                return false;
            } else {
                param->seed = seed =
                    app.getLevelGenerationOptions()->getLevelSeed();

                if (param->levelGen->isTutorial()) {
                    
                    if (param->levelGen->requiresBaseSave() &&
                        !param->levelGen->getBaseSavePath().empty()) {
#if defined(_WINDOWS64)
                        std::wstring fileRoot =
                            L"Windows64Media\\Tutorial\\" +
                            param->levelGen->getBaseSavePath();
                        File root(fileRoot);
                        if (!root.exists())
                            fileRoot = L"Windows64\\Tutorial\\" +
                                       param->levelGen->getBaseSavePath();
#else
                        std::wstring fileRoot =
                            L"Tutorial\\" + param->levelGen->getBaseSavePath();
#endif
                        File grf(fileRoot);
                        if (grf.exists()) {
                            std::size_t dwFileSize =
                                PlatformFileIO.fileSize(grf.getPath());
                            if (dwFileSize > 0) {
                                uint8_t* pbData =
                                    (uint8_t*)new uint8_t[dwFileSize];
                                auto readResult = PlatformFileIO.readFile(
                                    grf.getPath(), pbData, dwFileSize);
                                if (readResult.status !=
                                    IPlatformFileIO::ReadStatus::Ok) {
                                    app.FatalLoadError();
                                }

                                
                                
                                param->levelGen->setBaseSaveData(pbData,
                                                                 dwFileSize);
                            }
                        }
                    }
                }
            }
        }
    }

    static int64_t sseed =
        seed;  
               
    ServerStoppedCreate(false);
    if (g_NetworkManager.IsHost()) {
        ServerStoppedCreate(true);
        ServerReadyCreate(true);
        
        C4JThread* thread =
            new C4JThread(&CGameNetworkManager::ServerThreadProc, lpParameter,
                          "Server", 256 * 1024);

        thread->run();

        app.DebugPrintf("[NET] Waiting for server ready...\n");
        ServerReadyWait();
        ServerReadyDestroy();
        app.DebugPrintf("[NET] Server ready! serverHalted=%d\n",
                        MinecraftServer::serverHalted());

        if (MinecraftServer::serverHalted()) return false;

        
    } else {
        Socket::Initialise(nullptr);
    }

    Minecraft* pMinecraft = Minecraft::GetInstance();
    app.DebugPrintf("[NET] IsReadyToPlayOrIdle=%d  IsInSession=%d\n",
                    IsReadyToPlayOrIdle(), IsInSession());
    
    
    
    bool changedMessage = false;
    while (!IsReadyToPlayOrIdle()) {
        changedMessage = true;
        pMinecraft->progressRenderer->progressStage(
            g_NetworkManager.CorrectErrorIDS(
                IDS_PROGRESS_SAVING_TO_DISC));  
                                                
        pMinecraft->progressRenderer->progressStagePercentage(
            g_NetworkManager.GetJoiningReadyPercentage());
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (changedMessage) {
        pMinecraft->progressRenderer->progressStagePercentage(100);
    }

    
    
    app.DebugPrintf("[NET] Checking IsInSession...=%d\n", IsInSession());
    if (!IsInSession()) {
        app.DebugPrintf("[NET] NOT in session! Halting server.\n");
        MinecraftServer::HaltServer();
        return false;
    }

    app.DebugPrintf("[NET] DLC check: completed=%d pending=%d\n",
                    app.DLCInstallProcessCompleted(), app.DLCInstallPending());
    
    
    
    while (!app.DLCInstallProcessCompleted() && app.DLCInstallPending() &&
           !g_NetworkManager.IsLeavingGame()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (g_NetworkManager.IsLeavingGame()) {
        MinecraftServer::HaltServer();
        return false;
    }

    

    app.DebugPrintf("[NET] Creating ClientConnection (IsHost=%d)...\n",
                    g_NetworkManager.IsHost());
    std::vector<ClientConnection*> createdConnections;
    ClientConnection* connection;

    if (g_NetworkManager.IsHost()) {
        connection = new ClientConnection(minecraft, nullptr);
        app.DebugPrintf("[NET] ClientConnection created, createdOk=%d\n",
                        connection->createdOk);
    } else {
        INetworkPlayer* pNetworkPlayer =
            g_NetworkManager.GetLocalPlayerByUserIndex(
                ProfileManager.GetLockedProfile());
        if (pNetworkPlayer == nullptr) {
            MinecraftServer::HaltServer();
            app.DebugPrintf("%d\n", ProfileManager.GetLockedProfile());
            
            
            return false;
        }

        Socket* socket = pNetworkPlayer->GetSocket();

        
        
        if (socket == nullptr) {
            assert(false);
            MinecraftServer::HaltServer();
            
            
            return false;
        }

        connection = new ClientConnection(minecraft, socket);
    }

    if (!connection->createdOk) {
        assert(false);
        delete connection;
        connection = nullptr;
        MinecraftServer::HaltServer();
        return false;
    }

    app.DebugPrintf("[NET] Sending PreLoginPacket...\n");
    connection->send(std::shared_ptr<PreLoginPacket>(
        new PreLoginPacket(minecraft->user->name)));
    app.DebugPrintf(
        "[NET] PreLoginPacket sent. Entering connection tick loop...\n");

    
    
    
    
    
    if (!g_NetworkManager.IsHost()) {
        Minecraft::GetInstance()->progressRenderer->progressStart(
            IDS_PROGRESS_CONNECTING);
    }

    TexturePack* tPack = Minecraft::GetInstance()->skins->getSelected();
    do {
        connection->tick();

        
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    } while ((IsInSession() && !connection->isStarted() &&
              !connection->isClosed() && !g_NetworkManager.IsLeavingGame()) ||
             tPack->isLoadingData() ||
             (Minecraft::GetInstance()->skins->needsUIUpdate() ||
              ui.IsReloadingSkin()));
    ui.CleanUpSkinReload();

    
    
    
    if (g_NetworkManager.IsLeavingGame() || !IsInSession()) {
        connection->close();
    }

    if (connection->isStarted() && !connection->isClosed()) {
        createdConnections.push_back(connection);

        int primaryPad = ProfileManager.GetPrimaryPad();
        app.SetRichPresenceContext(primaryPad, CONTEXT_GAME_STATE_BLANK);
        if (GetPlayerCount() >
            1)  
        {
            if (IsLocalGame())
                ProfileManager.SetCurrentGameActivity(
                    primaryPad, CONTEXT_PRESENCE_MULTIPLAYEROFFLINE, false);
            else
                ProfileManager.SetCurrentGameActivity(
                    primaryPad, CONTEXT_PRESENCE_MULTIPLAYER, false);
        } else {
            if (IsLocalGame())
                ProfileManager.SetCurrentGameActivity(
                    primaryPad, CONTEXT_PRESENCE_MULTIPLAYER_1POFFLINE, false);
            else
                ProfileManager.SetCurrentGameActivity(
                    primaryPad, CONTEXT_PRESENCE_MULTIPLAYER_1P, false);
        }

        
        for (int idx = 0; idx < XUSER_MAX_COUNT; ++idx) {
            
            if (idx == ProfileManager.GetPrimaryPad()) continue;

            if (GetLocalPlayerByUserIndex(idx) != nullptr &&
                !ProfileManager.IsSignedIn(idx)) {
                INetworkPlayer* pNetworkPlayer =
                    g_NetworkManager.GetLocalPlayerByUserIndex(idx);
                Socket* socket = pNetworkPlayer->GetSocket();
                app.DebugPrintf(
                    "Closing socket due to player %d not being signed in any "
                    "more\n");
                if (!socket->close(false)) socket->close(true);

                continue;
            }

            
            
            
            
            
            
            INetworkPlayer* pNetworkPlayer =
                g_NetworkManager.GetLocalPlayerByUserIndex(idx);
            if (pNetworkPlayer == nullptr) continue;

            ClientConnection* connection;

            Socket* socket = pNetworkPlayer->GetSocket();
            connection = new ClientConnection(minecraft, socket, idx);

            minecraft->addPendingLocalConnection(idx, connection);
            
            

            
            Socket::addIncomingSocket(socket);

            connection->send(std::shared_ptr<PreLoginPacket>(new PreLoginPacket(
                convStringToWstring(ProfileManager.GetGamertag(idx)))));

            createdConnections.push_back(connection);

            
            
            
            
            
            
            do {
                
                
                for (auto it = createdConnections.begin();
                     it < createdConnections.end(); ++it) {
                    (*it)->tick();
                }

                
                
                
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                app.DebugPrintf("<***> %d %d %d %d %d\n", IsInSession(),
                                !connection->isStarted(),
                                !connection->isClosed(),
                                ProfileManager.IsSignedIn(idx),
                                !g_NetworkManager.IsLeavingGame());
                
                
                
            } while (IsInSession() && !connection->isStarted() &&
                     !connection->isClosed() &&
                     !g_NetworkManager.IsLeavingGame());

            
            
            
            
            
            if (g_NetworkManager.IsLeavingGame() || !IsInSession()) break;

            if (ProfileManager.IsSignedIn(idx) && !connection->isClosed()) {
                app.SetRichPresenceContext(idx, CONTEXT_GAME_STATE_BLANK);
                if (IsLocalGame())
                    ProfileManager.SetCurrentGameActivity(
                        idx, CONTEXT_PRESENCE_MULTIPLAYEROFFLINE, false);
                else
                    ProfileManager.SetCurrentGameActivity(
                        idx, CONTEXT_PRESENCE_MULTIPLAYER, false);
            } else {
                connection->close();
                auto it = find(createdConnections.begin(),
                               createdConnections.end(), connection);
                if (it != createdConnections.end())
                    createdConnections.erase(it);
            }
        }

        app.SetGameMode(eMode_Multiplayer);
    } else if (connection->isClosed() || !IsInSession()) {
        
        MinecraftServer::HaltServer();
        return false;
    }

    if (g_NetworkManager.IsLeavingGame() || !IsInSession()) {
        for (auto it = createdConnections.begin();
             it < createdConnections.end(); ++it) {
            (*it)->close();
        }
        
        MinecraftServer::HaltServer();
        return false;
    }

    
    if (MinecraftServer::serverHalted()) return false;

    return true;
}

int CGameNetworkManager::CorrectErrorIDS(int IDS) {
    return s_pPlatformNetworkManager->CorrectErrorIDS(IDS);
}

int CGameNetworkManager::GetLocalPlayerMask(int playerIndex) {
    return s_pPlatformNetworkManager->GetLocalPlayerMask(playerIndex);
}

int CGameNetworkManager::GetPlayerCount() {
    return s_pPlatformNetworkManager->GetPlayerCount();
}

int CGameNetworkManager::GetOnlinePlayerCount() {
    return s_pPlatformNetworkManager->GetOnlinePlayerCount();
}

bool CGameNetworkManager::AddLocalPlayerByUserIndex(int userIndex) {
    return s_pPlatformNetworkManager->AddLocalPlayerByUserIndex(userIndex);
}

bool CGameNetworkManager::RemoveLocalPlayerByUserIndex(int userIndex) {
    return s_pPlatformNetworkManager->RemoveLocalPlayerByUserIndex(userIndex);
}

INetworkPlayer* CGameNetworkManager::GetLocalPlayerByUserIndex(int userIndex) {
    return s_pPlatformNetworkManager->GetLocalPlayerByUserIndex(userIndex);
}

INetworkPlayer* CGameNetworkManager::GetPlayerByIndex(int playerIndex) {
    return s_pPlatformNetworkManager->GetPlayerByIndex(playerIndex);
}

INetworkPlayer* CGameNetworkManager::GetPlayerByXuid(PlayerUID xuid) {
    return s_pPlatformNetworkManager->GetPlayerByXuid(xuid);
}

INetworkPlayer* CGameNetworkManager::GetPlayerBySmallId(unsigned char smallId) {
    return s_pPlatformNetworkManager->GetPlayerBySmallId(smallId);
}

INetworkPlayer* CGameNetworkManager::GetHostPlayer() {
    return s_pPlatformNetworkManager->GetHostPlayer();
}

void CGameNetworkManager::RegisterPlayerChangedCallback(
    int iPad,
    std::function<void(INetworkPlayer* pPlayer, bool leaving)> callback) {
    s_pPlatformNetworkManager->RegisterPlayerChangedCallback(
        iPad, std::move(callback));
}

void CGameNetworkManager::UnRegisterPlayerChangedCallback(int iPad) {
    s_pPlatformNetworkManager->UnRegisterPlayerChangedCallback(iPad);
}

void CGameNetworkManager::HandleSignInChange() {
    s_pPlatformNetworkManager->HandleSignInChange();
}

bool CGameNetworkManager::ShouldMessageForFullSession() {
    return s_pPlatformNetworkManager->ShouldMessageForFullSession();
}

bool CGameNetworkManager::IsInSession() {
    return s_pPlatformNetworkManager->IsInSession();
}

bool CGameNetworkManager::IsInGameplay() {
    return s_pPlatformNetworkManager->IsInGameplay();
}

bool CGameNetworkManager::IsReadyToPlayOrIdle() {
    return s_pPlatformNetworkManager->IsReadyToPlayOrIdle();
}

bool CGameNetworkManager::IsLeavingGame() {
    return s_pPlatformNetworkManager->IsLeavingGame();
}

bool CGameNetworkManager::SetLocalGame(bool isLocal) {
    return s_pPlatformNetworkManager->SetLocalGame(isLocal);
}

bool CGameNetworkManager::IsLocalGame() {
    return s_pPlatformNetworkManager->IsLocalGame();
}

void CGameNetworkManager::SetPrivateGame(bool isPrivate) {
    s_pPlatformNetworkManager->SetPrivateGame(isPrivate);
}

bool CGameNetworkManager::IsPrivateGame() {
    return s_pPlatformNetworkManager->IsPrivateGame();
}

void CGameNetworkManager::HostGame(int localUsersMask, bool bOnlineGame,
                                   bool bIsPrivate, unsigned char publicSlots,
                                   unsigned char privateSlots) {
    
    Minecraft::GetInstance()->clearConnectionFailed();

    s_pPlatformNetworkManager->HostGame(localUsersMask, bOnlineGame, bIsPrivate,
                                        publicSlots, privateSlots);
}

bool CGameNetworkManager::IsHost() {
    return (s_pPlatformNetworkManager->IsHost() == true);
}

bool CGameNetworkManager::IsInStatsEnabledSession() {
    return s_pPlatformNetworkManager->IsInStatsEnabledSession();
}

bool CGameNetworkManager::SessionHasSpace(unsigned int spaceRequired) {
    return s_pPlatformNetworkManager->SessionHasSpace(spaceRequired);
}

std::vector<FriendSessionInfo*>* CGameNetworkManager::GetSessionList(
    int iPad, int localPlayers, bool partyOnly) {
    return s_pPlatformNetworkManager->GetSessionList(iPad, localPlayers,
                                                     partyOnly);
}

bool CGameNetworkManager::GetGameSessionInfo(int iPad, SessionID sessionId,
                                             FriendSessionInfo* foundSession) {
    return s_pPlatformNetworkManager->GetGameSessionInfo(iPad, sessionId,
                                                         foundSession);
}

void CGameNetworkManager::SetSessionsUpdatedCallback(
    std::function<void()> callback) {
    s_pPlatformNetworkManager->SetSessionsUpdatedCallback(std::move(callback));
}

void CGameNetworkManager::GetFullFriendSessionInfo(
    FriendSessionInfo* foundSession,
    std::function<void(bool success)> callback) {
    s_pPlatformNetworkManager->GetFullFriendSessionInfo(
        foundSession, std::move(callback));
}

void CGameNetworkManager::ForceFriendsSessionRefresh() {
    s_pPlatformNetworkManager->ForceFriendsSessionRefresh();
}

bool CGameNetworkManager::JoinGameFromInviteInfo(
    int userIndex, int userMask, const INVITE_INFO* pInviteInfo) {
    return s_pPlatformNetworkManager->JoinGameFromInviteInfo(
        userIndex, userMask, pInviteInfo);
}

CGameNetworkManager::eJoinGameResult CGameNetworkManager::JoinGame(
    FriendSessionInfo* searchResult, int localUsersMask) {
    app.SetTutorialMode(false);
    g_NetworkManager.SetLocalGame(false);

    int primaryUserIndex = ProfileManager.GetLockedProfile();

    
    Minecraft::GetInstance()->clearConnectionFailed();

    
    localUsersMask |= GetLocalPlayerMask(ProfileManager.GetPrimaryPad());

    return (eJoinGameResult)(s_pPlatformNetworkManager->JoinGame(
        searchResult, localUsersMask, primaryUserIndex));
}

void CGameNetworkManager::CancelJoinGame(void* lpParam) {}

bool CGameNetworkManager::LeaveGame(bool bMigrateHost) {
    Minecraft::GetInstance()->gui->clearMessages();
    return s_pPlatformNetworkManager->LeaveGame(bMigrateHost);
}

int CGameNetworkManager::JoinFromInvite_SignInReturned(void* pParam,
                                                       bool bContinue,
                                                       int iPad) {
    INVITE_INFO* pInviteInfo = (INVITE_INFO*)pParam;

    if (bContinue == true) {
        app.DebugPrintf("JoinFromInvite_SignInReturned, iPad %d\n", iPad);
        
        if (ProfileManager.IsSignedIn(iPad) &&
            ProfileManager.IsSignedInLive(iPad)) {
            app.DebugPrintf(
                "JoinFromInvite_SignInReturned, passed sign-in tests\n");
            int localUsersMask = 0;
            int joiningUsers = 0;

            bool noPrivileges = false;
            for (unsigned int index = 0; index < XUSER_MAX_COUNT; ++index) {
                if (ProfileManager.IsSignedIn(index)) {
                    ++joiningUsers;
                    if (!ProfileManager.AllowedToPlayMultiplayer(index))
                        noPrivileges = true;
                    localUsersMask |= GetLocalPlayerMask(index);
                }
            }

            
            
            bool noUGC = false;

            if (noUGC) {
                int messageText =
                    IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_SINGLE_LOCAL;
                if (joiningUsers > 1)
                    messageText =
                        IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_ALL_LOCAL;

                ui.RequestUGCMessageBox(IDS_CONNECTION_FAILED, messageText);
            } else if (noPrivileges) {
                unsigned int uiIDA[1];
                uiIDA[0] = IDS_CONFIRM_OK;
                ui.RequestErrorMessage(IDS_NO_MULTIPLAYER_PRIVILEGE_TITLE,
                                       IDS_NO_MULTIPLAYER_PRIVILEGE_JOIN_TEXT,
                                       uiIDA, 1,
                                       ProfileManager.GetPrimaryPad());
            } else {
                ProfileManager.SetLockedProfile(iPad);
                ProfileManager.SetPrimaryPad(iPad);

                g_NetworkManager.SetLocalGame(false);

                
                
                
                ProfileManager.QuerySigninStatus();

                
                Minecraft::GetInstance()->clearConnectionFailed();

                
                Minecraft::GetInstance()->user->name = convStringToWstring(
                    ProfileManager.GetGamertag(ProfileManager.GetPrimaryPad()));

                bool success = g_NetworkManager.JoinGameFromInviteInfo(
                    iPad,            
                    localUsersMask,  
                    pInviteInfo);    
                if (!success) {
                    app.DebugPrintf("Failed joining game from invite\n");
                }
            }
        } else {
            app.DebugPrintf(
                "JoinFromInvite_SignInReturned, failed sign-in tests :%d %d\n",
                ProfileManager.IsSignedIn(iPad),
                ProfileManager.IsSignedInLive(iPad));
        }
    }
    return 0;
}

void CGameNetworkManager::UpdateAndSetGameSessionData(
    INetworkPlayer* pNetworkPlayerLeaving) {
    Minecraft* pMinecraft = Minecraft::GetInstance();
    TexturePack* tPack = pMinecraft->skins->getSelected();
    s_pPlatformNetworkManager->SetSessionTexturePackParentId(
        tPack->getDLCParentPackId());
    s_pPlatformNetworkManager->SetSessionSubTexturePackId(
        tPack->getDLCSubPackId());

    s_pPlatformNetworkManager->UpdateAndSetGameSessionData(
        pNetworkPlayerLeaving);
}

void CGameNetworkManager::SendInviteGUI(int quadrant) {
    s_pPlatformNetworkManager->SendInviteGUI(quadrant);
}

void CGameNetworkManager::ResetLeavingGame() {
    s_pPlatformNetworkManager->ResetLeavingGame();
}

bool CGameNetworkManager::IsNetworkThreadRunning() {
    return m_bNetworkThreadRunning;
    ;
}

int CGameNetworkManager::RunNetworkGameThreadProc(void* lpParameter) {
    
    
    Compression::UseDefaultThreadStorage();
    Tile::CreateNewThreadStorage();

    g_NetworkManager.m_bNetworkThreadRunning = true;
    bool success = g_NetworkManager._RunNetworkGame(lpParameter);
    g_NetworkManager.m_bNetworkThreadRunning = false;
    if (!success) {
        TexturePack* tPack = Minecraft::GetInstance()->skins->getSelected();
        while (tPack->isLoadingData() ||
               (Minecraft::GetInstance()->skins->needsUIUpdate() ||
                ui.IsReloadingSkin())) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        ui.CleanUpSkinReload();
        if (app.GetDisconnectReason() == DisconnectPacket::eDisconnect_None) {
            app.SetDisconnectReason(
                DisconnectPacket::eDisconnect_ConnectionCreationFailed);
        }
        
        
        if (MinecraftServer::getInstance() == nullptr)
            app.m_gameRules.unloadCurrentGameRules();
        Tile::ReleaseThreadStorage();
        return -1;
    }

    Tile::ReleaseThreadStorage();
    return 0;
}

int CGameNetworkManager::ServerThreadProc(void* lpParameter) {
    int64_t seed = 0;
    if (lpParameter != nullptr) {
        NetworkGameInitData* param = (NetworkGameInitData*)lpParameter;
        seed = param->seed;
        app.SetGameHostOption(eGameHostOption_All, param->settings);

        
        
        if (param->levelGen != nullptr &&
            (param->texturePackId == 0 ||
             param->levelGen->getRequiredTexturePackId() !=
                 param->texturePackId)) {
            while ((Minecraft::GetInstance()->skins->needsUIUpdate() ||
                    ui.IsReloadingSkin())) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            param->levelGen->loadBaseSaveData();
        }
    }

    C4JThread::setThreadName(static_cast<std::uint32_t>(-1), "Minecraft Server thread");
    Compression::UseDefaultThreadStorage();
    OldChunkStorage::UseDefaultThreadStorage();
    Entity::useSmallIds();
    Level::enableLightingCache();
    Tile::CreateNewThreadStorage();
    FireworksRecipe::CreateNewThreadStorage();

    MinecraftServer::main(
        seed,
        lpParameter);  

    Tile::ReleaseThreadStorage();
    Level::destroyLightingCache();

    if (lpParameter != nullptr) delete (NetworkGameInitData*)lpParameter;

    return 0;
}

int CGameNetworkManager::ExitAndJoinFromInviteThreadProc(void* lpParam) {
    
    
    Compression::UseDefaultThreadStorage();

    
    UIScene_PauseMenu::_ExitWorld(nullptr);

    while (g_NetworkManager.IsInSession()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    
    
    JoinFromInviteData* inviteData = (JoinFromInviteData*)lpParam;
    app.SetAction(inviteData->dwUserIndex, eAppAction_JoinFromInvite, lpParam);

    return 0;
}

void CGameNetworkManager::_LeaveGame() {
    s_pPlatformNetworkManager->_LeaveGame(false, true);
}

int CGameNetworkManager::ChangeSessionTypeThreadProc(void* lpParam) {
    
    
    Compression::UseDefaultThreadStorage();

    Minecraft* pMinecraft = Minecraft::GetInstance();
    MinecraftServer* pServer = MinecraftServer::getInstance();

    pMinecraft->progressRenderer->progressStartNoAbort(
        g_NetworkManager.CorrectErrorIDS(IDS_CONNECTION_LOST_LIVE_NO_EXIT));
    pMinecraft->progressRenderer->progressStage(
        IDS_PROGRESS_CONVERTING_TO_OFFLINE_GAME);

    while (app.GetXuiServerAction(ProfileManager.GetPrimaryPad()) !=
               eXuiServerAction_Idle &&
           !MinecraftServer::serverHalted()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),
                           eXuiServerAction_PauseServer, (void*)true);

    
    pServer->m_serverPausedEvent->waitForSignal(C4JThread::kInfiniteTimeout);

    pMinecraft->progressRenderer->progressStartNoAbort(
        g_NetworkManager.CorrectErrorIDS(IDS_CONNECTION_LOST_LIVE_NO_EXIT));
    pMinecraft->progressRenderer->progressStage(
        IDS_PROGRESS_CONVERTING_TO_OFFLINE_GAME);

    pMinecraft->progressRenderer->progressStagePercentage(25);

    
    
    if (pServer != nullptr) {
        PlayerList* players = pServer->getPlayers();
        for (auto it = players->players.begin(); it < players->players.end();
             ++it) {
            std::shared_ptr<ServerPlayer> servPlayer = *it;
            if (servPlayer->connection->isLocal() &&
                !servPlayer->connection->isGuest()) {
                servPlayer->connection->connection->getSocket()->setPlayer(
                    nullptr);
            }
        }
    }

    
    
    
    
    if (g_NetworkManager.m_bLastDisconnectWasLostRoomOnly) {
        s_pPlatformNetworkManager->_LeaveGame(false, false);
    } else {
        s_pPlatformNetworkManager->_LeaveGame(false, true);
    }

    
    while (g_NetworkManager.IsInSession()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    
    
    g_NetworkManager.m_bLastDisconnectWasLostRoomOnly = false;
    g_NetworkManager.m_bFullSessionMessageOnNextSessionChange = false;

    pMinecraft->progressRenderer->progressStagePercentage(50);

    
    g_NetworkManager.SetLocalGame(true);

    
    int localUsersMask = 0;
    char numLocalPlayers = 0;
    for (unsigned int index = 0; index < XUSER_MAX_COUNT; ++index) {
        if (ProfileManager.IsSignedIn(index) &&
            pMinecraft->localplayers[index] != nullptr) {
            numLocalPlayers++;
            localUsersMask |= GetLocalPlayerMask(index);
        }
    }

    s_pPlatformNetworkManager->_HostGame(localUsersMask);

    pMinecraft->progressRenderer->progressStagePercentage(75);

    
    while (g_NetworkManager.GetPlayerCount() < numLocalPlayers) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    
    if (pServer != nullptr) {
        for (unsigned int index = 0; index < XUSER_MAX_COUNT; ++index) {
            if (ProfileManager.IsSignedIn(index) &&
                pMinecraft->localplayers[index] != nullptr) {
                PlayerUID localPlayerXuid =
                    pMinecraft->localplayers[index]->getXuid();

                PlayerList* players = pServer->getPlayers();
                for (auto it = players->players.begin();
                     it < players->players.end(); ++it) {
                    std::shared_ptr<ServerPlayer> servPlayer = *it;
                    if (servPlayer->getXuid() == localPlayerXuid) {
                        servPlayer->connection->connection->getSocket()
                            ->setPlayer(
                                g_NetworkManager.GetLocalPlayerByUserIndex(
                                    index));
                    }
                }

                
                if (pMinecraft->m_pendingLocalConnections[index] != nullptr) {
                    
                    pMinecraft->m_pendingLocalConnections[index]
                        ->getConnection()
                        ->getSocket()
                        ->setPlayer(
                            g_NetworkManager.GetLocalPlayerByUserIndex(index));
                } else if (pMinecraft->m_connectionFailed[index] &&
                           (pMinecraft->m_connectionFailedReason[index] ==
                            DisconnectPacket::
                                eDisconnect_ConnectionCreationFailed)) {
                    pMinecraft->removeLocalPlayerIdx(index);
                }
            }
        }
    }

    pMinecraft->progressRenderer->progressStagePercentage(100);

    
    
    while (!s_pPlatformNetworkManager->IsReadyToPlayOrIdle()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    s_pPlatformNetworkManager->_StartGame();

    
    while (ui.IsSceneInStack(XUSER_INDEX_ANY, eUIScene_MessageBox)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    
    app.SetGameStarted(true);
    app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),
                           eXuiServerAction_PauseServer, (void*)false);
    app.SetChangingSessionType(false);
    app.SetReallyChangingSessionType(false);

    return 0;
}

void CGameNetworkManager::SystemFlagSet(INetworkPlayer* pNetworkPlayer,
                                        int index) {
    s_pPlatformNetworkManager->SystemFlagSet(pNetworkPlayer, index);
}

bool CGameNetworkManager::SystemFlagGet(INetworkPlayer* pNetworkPlayer,
                                        int index) {
    return s_pPlatformNetworkManager->SystemFlagGet(pNetworkPlayer, index);
}

void CGameNetworkManager::SystemFlagClear(INetworkPlayer* pNetworkPlayer,
                                          int index) {
    s_pPlatformNetworkManager->SystemFlagClear(pNetworkPlayer, index);
}

std::wstring CGameNetworkManager::GatherStats() {
    return s_pPlatformNetworkManager->GatherStats();
}

void CGameNetworkManager::renderQueueMeter() {}

std::wstring CGameNetworkManager::GatherRTTStats() {
    return s_pPlatformNetworkManager->GatherRTTStats();
}

void CGameNetworkManager::StateChange_AnyToHosting() {
    app.DebugPrintf("Disabling Guest Signin\n");
    XEnableGuestSignin(false);
    Minecraft::GetInstance()->clearPendingClientTextureRequests();
}

void CGameNetworkManager::StateChange_AnyToJoining() {
    app.DebugPrintf("Disabling Guest Signin\n");
    XEnableGuestSignin(false);
    Minecraft::GetInstance()->clearPendingClientTextureRequests();

    ConnectionProgressParams* param = new ConnectionProgressParams();
    param->iPad = ProfileManager.GetPrimaryPad();
    param->stringId = -1;
    param->showTooltips = false;
    param->setFailTimer = true;
    param->timerTime = CONNECTING_PROGRESS_CHECK_TIME;

    ui.NavigateToScene(ProfileManager.GetPrimaryPad(),
                       eUIScene_ConnectingProgress, param);
}

void CGameNetworkManager::StateChange_JoiningToIdle(
    IPlatformNetwork::eJoinFailedReason reason) {
    DisconnectPacket::eDisconnectReason disconnectReason;
    switch (reason) {
        case IPlatformNetwork::JOIN_FAILED_SERVER_FULL:
            disconnectReason = DisconnectPacket::eDisconnect_ServerFull;
            break;
        case IPlatformNetwork::JOIN_FAILED_INSUFFICIENT_PRIVILEGES:
            disconnectReason =
                DisconnectPacket::eDisconnect_NoMultiplayerPrivilegesJoin;
            app.SetAction(ProfileManager.GetPrimaryPad(),
                          eAppAction_FailedToJoinNoPrivileges);
            break;
        default:
            disconnectReason =
                DisconnectPacket::eDisconnect_ConnectionCreationFailed;
            break;
    };
    Minecraft::GetInstance()->connectionDisconnected(
        ProfileManager.GetPrimaryPad(), disconnectReason);
}

void CGameNetworkManager::StateChange_AnyToStarting() {
    if (!g_NetworkManager.IsHost()) {
        LoadingInputParams* loadingParams = new LoadingInputParams();
        loadingParams->func = &CGameNetworkManager::RunNetworkGameThreadProc;
        loadingParams->lpParam = nullptr;

        UIFullscreenProgressCompletionData* completionData =
            new UIFullscreenProgressCompletionData();
        completionData->bShowBackground = true;
        completionData->bShowLogo = true;
        completionData->type = e_ProgressCompletion_CloseAllPlayersUIScenes;
        completionData->iPad = ProfileManager.GetPrimaryPad();
        loadingParams->completionData = completionData;

        ui.NavigateToScene(ProfileManager.GetPrimaryPad(),
                           eUIScene_FullscreenProgress, loadingParams);
    }
}

void CGameNetworkManager::StateChange_AnyToEnding(bool bStateWasPlaying) {
    
    
    if (bStateWasPlaying && g_NetworkManager.IsLocalGame()) {
        for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i) {
            INetworkPlayer* pNetworkPlayer =
                g_NetworkManager.GetLocalPlayerByUserIndex(i);
            if (pNetworkPlayer != nullptr && ProfileManager.IsSignedIn(i)) {
                app.DebugPrintf(
                    "Stats save for an offline game for the player at index "
                    "%d\n",
                    i);
                Minecraft::GetInstance()->forceStatsSave(
                    pNetworkPlayer->GetUserIndex());
            }
        }
    }

    Minecraft::GetInstance()->gui->clearMessages();

    if (!g_NetworkManager.IsHost() && !g_NetworkManager.IsLeavingGame()) {
        
        
        

        
        if (app.GetDisconnectReason() == DisconnectPacket::eDisconnect_None)
            app.SetDisconnectReason(DisconnectPacket::eDisconnect_Quitting);
        app.SetAction(ProfileManager.GetPrimaryPad(), eAppAction_ExitWorld,
                      (void*)true);
    }
}

void CGameNetworkManager::StateChange_AnyToIdle() {
    app.DebugPrintf("Enabling Guest Signin\n");
    XEnableGuestSignin(true);
    
    
    
    if (!app.GetChangingSessionType()) g_NetworkManager.SetLocalGame(false);
}

void CGameNetworkManager::CreateSocket(INetworkPlayer* pNetworkPlayer,
                                       bool localPlayer) {
    Minecraft* pMinecraft = Minecraft::GetInstance();

    Socket* socket = nullptr;
    std::shared_ptr<MultiplayerLocalPlayer> mpPlayer =
        pMinecraft->localplayers[pNetworkPlayer->GetUserIndex()];
    if (localPlayer && mpPlayer != nullptr && mpPlayer->connection != nullptr) {
        
        
        socket = mpPlayer->connection->getSocket();

        
        pNetworkPlayer->SetSocket(socket);
        if (socket) {
            socket->setPlayer(pNetworkPlayer);
        }
    } else {
        socket = new Socket(pNetworkPlayer, g_NetworkManager.IsHost(),
                            g_NetworkManager.IsHost() && localPlayer);
        pNetworkPlayer->SetSocket(socket);

        
        
        if (g_NetworkManager.IsHost() && g_NetworkManager.IsInGameplay()) {
            Socket::addIncomingSocket(socket);
        }

        
        
        if (localPlayer && g_NetworkManager.IsInGameplay()) {
            int idx = pNetworkPlayer->GetUserIndex();
            app.DebugPrintf("Creating new client connection for idx: %d\n",
                            idx);

            ClientConnection* connection;
            connection = new ClientConnection(pMinecraft, socket, idx);

            if (connection->createdOk) {
                connection->send(std::shared_ptr<PreLoginPacket>(
                    new PreLoginPacket(pNetworkPlayer->GetOnlineName())));
                pMinecraft->addPendingLocalConnection(idx, connection);
            } else {
                pMinecraft->connectionDisconnected(
                    idx,
                    DisconnectPacket::eDisconnect_ConnectionCreationFailed);
                delete connection;
                connection = nullptr;
            }
        }
    }
}

void CGameNetworkManager::CloseConnection(INetworkPlayer* pNetworkPlayer) {
    MinecraftServer* server = MinecraftServer::getInstance();
    if (server != nullptr) {
        PlayerList* players = server->getPlayers();
        if (players != nullptr) {
            players->closePlayerConnectionBySmallId(
                pNetworkPlayer->GetSmallId());
        }
    }
}

void CGameNetworkManager::PlayerJoining(INetworkPlayer* pNetworkPlayer) {
    if (g_NetworkManager
            .IsInGameplay())  
                              
    {
        
        bool multiplayer = g_NetworkManager.GetPlayerCount() > 1,
             localgame = g_NetworkManager.IsLocalGame();
        for (int iPad = 0; iPad < XUSER_MAX_COUNT; ++iPad) {
            INetworkPlayer* pNetworkPlayer =
                g_NetworkManager.GetLocalPlayerByUserIndex(iPad);
            if (pNetworkPlayer == nullptr) continue;

            app.SetRichPresenceContext(iPad, CONTEXT_GAME_STATE_BLANK);
            if (multiplayer) {
                if (localgame)
                    ProfileManager.SetCurrentGameActivity(
                        iPad, CONTEXT_PRESENCE_MULTIPLAYEROFFLINE, false);
                else
                    ProfileManager.SetCurrentGameActivity(
                        iPad, CONTEXT_PRESENCE_MULTIPLAYER, false);
            } else {
                if (localgame)
                    ProfileManager.SetCurrentGameActivity(
                        iPad, CONTEXT_PRESENCE_MULTIPLAYER_1POFFLINE, false);
                else
                    ProfileManager.SetCurrentGameActivity(
                        iPad, CONTEXT_PRESENCE_MULTIPLAYER_1P, false);
            }
        }
    }
}

void CGameNetworkManager::PlayerLeaving(INetworkPlayer* pNetworkPlayer) {
    if (pNetworkPlayer->IsLocal()) {
        ProfileManager.SetCurrentGameActivity(pNetworkPlayer->GetUserIndex(),
                                              CONTEXT_PRESENCE_IDLE, false);
    }
}

void CGameNetworkManager::HostChanged() {
    
    app.SetAction(ProfileManager.GetPrimaryPad(), eAppAction_ExitWorld,
                  (void*)true);
}

void CGameNetworkManager::WriteStats(INetworkPlayer* pNetworkPlayer) {
    Minecraft::GetInstance()->forceStatsSave(pNetworkPlayer->GetUserIndex());
}

void CGameNetworkManager::GameInviteReceived(int userIndex,
                                             const INVITE_INFO* pInviteInfo) {
    int localUsersMask = 0;
    Minecraft* pMinecraft = Minecraft::GetInstance();
    int joiningUsers = 0;

    bool noPrivileges = false;
    for (unsigned int index = 0; index < XUSER_MAX_COUNT; ++index) {
        if (ProfileManager.IsSignedIn(index)) {
            
            
            
            
            if (index == userIndex ||
                pMinecraft->localplayers[index] != nullptr) {
                ++joiningUsers;
                if (!ProfileManager.AllowedToPlayMultiplayer(index))
                    noPrivileges = true;
                localUsersMask |= GetLocalPlayerMask(index);
            }
        }
    }

    
    
    bool noUGC = false;
    bool bContentRestricted = false;
    bool pccAllowed = true;
    bool pccFriendsAllowed = true;
    ProfileManager.AllowedPlayerCreatedContent(
        ProfileManager.GetPrimaryPad(), false, &pccAllowed, &pccFriendsAllowed);
    if (!pccAllowed && !pccFriendsAllowed) noUGC = true;

    if (noUGC) {
        int messageText = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_SINGLE_LOCAL;
        if (joiningUsers > 1)
            messageText = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_ALL_LOCAL;

        ui.RequestUGCMessageBox(IDS_CONNECTION_FAILED, messageText,
                                XUSER_INDEX_ANY);
    } else if (noPrivileges) {
        unsigned int uiIDA[1];
        uiIDA[0] = IDS_CONFIRM_OK;

        
        
        
        
        
        
        ui.RequestErrorMessage(IDS_NO_MULTIPLAYER_PRIVILEGE_TITLE,
                               IDS_NO_MULTIPLAYER_PRIVILEGE_JOIN_TEXT, uiIDA, 1,
                               XUSER_INDEX_ANY);
    } else {
        if (!g_NetworkManager.IsInSession()) {
            HandleInviteWhenInMenus(userIndex, pInviteInfo);
        } else {
            app.DebugPrintf(
                "We are already in a multiplayer game...need to leave it\n");

            
            
            
            
            

            
            {
                app.ProcessInvite(userIndex, localUsersMask, pInviteInfo);
            }
        }
    }
}

volatile bool waitHere = true;

void CGameNetworkManager::HandleInviteWhenInMenus(
    int userIndex, const INVITE_INFO* pInviteInfo) {
    

    {
        ProfileManager.SetPrimaryPad(userIndex);

        
        
        
        
        app.StartInstallDLCProcess(userIndex);

        
        
        
        
        

        if (!app.IsLocalMultiplayerAvailable()) {
            bool noPrivileges =
                !ProfileManager.AllowedToPlayMultiplayer(userIndex);

            if (noPrivileges) {
                unsigned int uiIDA[1];
                uiIDA[0] = IDS_CONFIRM_OK;
                ui.RequestErrorMessage(IDS_NO_MULTIPLAYER_PRIVILEGE_TITLE,
                                       IDS_NO_MULTIPLAYER_PRIVILEGE_JOIN_TEXT,
                                       uiIDA, 1,
                                       ProfileManager.GetPrimaryPad());
            } else {
                ProfileManager.SetLockedProfile(userIndex);
                ProfileManager.SetPrimaryPad(userIndex);

                int localUsersMask = 0;
                localUsersMask |= GetLocalPlayerMask(userIndex);

                
                
                
                ProfileManager.QuerySigninStatus();

                
                Minecraft::GetInstance()->clearConnectionFailed();

                g_NetworkManager.SetLocalGame(false);

                
                Minecraft::GetInstance()->user->name = convStringToWstring(
                    ProfileManager.GetGamertag(ProfileManager.GetPrimaryPad()));

                bool success = g_NetworkManager.JoinGameFromInviteInfo(
                    userIndex, localUsersMask, pInviteInfo);
                if (!success) {
                    app.DebugPrintf("Failed joining game from invite\n");
                }
            }
        } else {
            
            
            SignInInfo info;
            info.Func = [pInviteInfo](bool bContinue, int pad) {
                return JoinFromInvite_SignInReturned(
                    const_cast<INVITE_INFO*>(pInviteInfo), bContinue, pad);
            };
            info.requireOnline = true;
            app.DebugPrintf("Using fullscreen layer\n");
            ui.NavigateToScene(ProfileManager.GetPrimaryPad(),
                               eUIScene_QuadrantSignin, &info, eUILayer_Alert,
                               eUIGroup_Fullscreen);
        }
    }
}

void CGameNetworkManager::AddLocalPlayerFailed(int idx,
                                               bool serverFull ) {
    Minecraft::GetInstance()->connectionDisconnected(
        idx, serverFull
                 ? DisconnectPacket::eDisconnect_ServerFull
                 : DisconnectPacket::eDisconnect_ConnectionCreationFailed);
}

void CGameNetworkManager::HandleDisconnect(bool bLostRoomOnly) {
    int iPrimaryPlayer = g_NetworkManager.GetPrimaryPad();

    if ((g_NetworkManager.GetLockedProfile() != -1) && iPrimaryPlayer != -1 &&
        g_NetworkManager.IsInSession()) {
        m_bLastDisconnectWasLostRoomOnly = bLostRoomOnly;
        app.SetAction(iPrimaryPlayer, eAppAction_EthernetDisconnected);
    } else {
        m_bLastDisconnectWasLostRoomOnly = false;
    }
}

int CGameNetworkManager::GetPrimaryPad() {
    return ProfileManager.GetPrimaryPad();
}

int CGameNetworkManager::GetLockedProfile() {
    return ProfileManager.GetLockedProfile();
}

bool CGameNetworkManager::IsSignedInLive(int playerIdx) {
    return ProfileManager.IsSignedInLive(playerIdx);
}

bool CGameNetworkManager::AllowedToPlayMultiplayer(int playerIdx) {
    return ProfileManager.AllowedToPlayMultiplayer(playerIdx);
}

char* CGameNetworkManager::GetOnlineName(int playerIdx) {
    return ProfileManager.GetGamertag(playerIdx);
}

void CGameNetworkManager::ServerReadyCreate(bool create) {
    m_hServerReadyEvent = (create ? (new C4JThread::Event) : nullptr);
}

void CGameNetworkManager::ServerReady() {
    if (m_hServerReadyEvent != nullptr) {
        m_hServerReadyEvent->set();
    } else {
        app.DebugPrintf(
            "[NET] Warning: ServerReady() called but m_hServerReadyEvent is "
            "nullptr\n");
    }
}

void CGameNetworkManager::ServerReadyWait() {
    if (m_hServerReadyEvent != nullptr) {
        m_hServerReadyEvent->waitForSignal(C4JThread::kInfiniteTimeout);
    } else {
        app.DebugPrintf(
            "[NET] Warning: ServerReadyWait() called but m_hServerReadyEvent "
            "is nullptr\n");
    }
}

void CGameNetworkManager::ServerReadyDestroy() {
    delete m_hServerReadyEvent;
    m_hServerReadyEvent = nullptr;
}

bool CGameNetworkManager::ServerReadyValid() {
    return (m_hServerReadyEvent != nullptr);
}

void CGameNetworkManager::ServerStoppedCreate(bool create) {
    m_hServerStoppedEvent = (create ? (new C4JThread::Event) : nullptr);
}

void CGameNetworkManager::ServerStopped() {
    if (m_hServerStoppedEvent != nullptr) {
        m_hServerStoppedEvent->set();
    } else {
        app.DebugPrintf(
            "[NET] Warning: ServerStopped() called but m_hServerStoppedEvent "
            "is nullptr\n");
    }
}

void CGameNetworkManager::ServerStoppedWait() {
    
    
    
    
    
    if (C4JThread::isMainThread()) {
        int result = C4JThread::WaitResult::Timeout;
        do {
            RenderManager.StartFrame();
            result = m_hServerStoppedEvent->waitForSignal(20);
            
            ProfileManager.Tick();
            StorageManager.Tick();
            InputManager.Tick();
            RenderManager.Tick();
            ui.tick();
            ui.render();
            RenderManager.Present();
        } while (result == C4JThread::WaitResult::Timeout);
    } else {
        if (m_hServerStoppedEvent != nullptr) {
            m_hServerStoppedEvent->waitForSignal(C4JThread::kInfiniteTimeout);
        } else {
            app.DebugPrintf(
                "[NET] Warning: ServerStoppedWait() called but "
                "m_hServerStoppedEvent is nullptr\n");
        }
    }
}

void CGameNetworkManager::ServerStoppedDestroy() {
    delete m_hServerStoppedEvent;
    m_hServerStoppedEvent = nullptr;
}

bool CGameNetworkManager::ServerStoppedValid() {
    return (m_hServerStoppedEvent != nullptr);
}

int CGameNetworkManager::GetJoiningReadyPercentage() {
    return s_pPlatformNetworkManager->GetJoiningReadyPercentage();
}

void CGameNetworkManager::FakeLocalPlayerJoined() {
    s_pPlatformNetworkManager->FakeLocalPlayerJoined();
}
