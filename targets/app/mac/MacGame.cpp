#include "MacGame.h"

#include <assert.h>
#include <stdio.h>
#include <string>
#include <ctime>
#include <unistd.h>
#include <wchar.h>

#include "platform/sdl2/Profile.h"
#include "platform/sdl2/Render.h"
#include "platform/sdl2/Storage.h"

#include "app/common/App_enums.h"
#include "app/common/Game.h"
#include "app/common/src/Network/GameNetworkManager.h"
#include "app/common/src/Network/NetworkPlayerInterface.h"
#include "app/common/src/Network/PlatformNetworkManagerStub.h"
#include "app/common/src/Network/RemoteNetworkPlayer.h"
#include "app/common/src/Network/Socket.h"
#include "app/common/src/UI/All Platforms/UIStructs.h"
#include "platform/C4JThread.h"

#include "minecraft/client/Minecraft.h"
#include "minecraft/client/Options.h"
#include "minecraft/client/User.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/world/level/LevelSettings.h"

extern bool _bQNetStubIsHost;
extern bool _bQNetStubGameRunning;

MacGame app;

#define CONTEXT_GAME_STATE 0

MacGame::MacGame() : Game() {}

void MacGame::SetRichPresenceContext(int iPad, int contextId) {}

void MacGame::StoreLaunchData() {}

void MacGame::ExitGame() {
    app.DebugPrintf("macOS MacGame::ExitGame AFTER START\n");
    RenderManager.Close();
}

void MacGame::FatalLoadError() {
    app.DebugPrintf("MacGame::FatalLoadError - asserting 0 and dying...\n");
    assert(0);
}

void MacGame::CaptureSaveThumbnail() {}

void MacGame::GetSaveThumbnail(std::uint8_t** thumbnailData,
                                unsigned int*  thumbnailSize) {}

void MacGame::ReleaseSaveThumbnail() {}

void MacGame::GetScreenshot(int iPad, std::uint8_t** screenshotData,
                             unsigned int* screenshotSize) {}

void MacGame::TemporaryCreateGameStart() {
    app.setLevelGenerationOptions(nullptr);
    Minecraft* pMinecraft = Minecraft::GetInstance();
    app.ReleaseSaveThumbnail();
    ProfileManager.SetLockedProfile(0);
    {
        std::wstring nick = pMinecraft->options
                                ? pMinecraft->options->lastMpNickname
                                : std::wstring();
        if (nick.empty()) nick = L"Player";
        pMinecraft->user->name = nick;
    }
    app.ApplyGameSettingsChanged(0);
    MinecraftServer::resetFlags();
    app.SetTutorialMode(false);
    app.SetCorruptSaveDeleted(false);
    app.ClearTerrainFeaturePosition();

    std::wstring wWorldName = L"TestWorld";
    StorageManager.ResetSaveData();
    StorageManager.SetSaveTitle(wWorldName.c_str());

    bool    isFlat    = false;
    int64_t seedValue = 0;

    NetworkGameInitData* param = new NetworkGameInitData();
    param->seed                = seedValue;
    param->saveData            = nullptr;

    app.SetGameHostOption(eGameHostOption_Difficulty,         0);
    app.SetGameHostOption(eGameHostOption_FriendsOfFriends,   0);
    app.SetGameHostOption(eGameHostOption_Gamertags,          1);
    app.SetGameHostOption(eGameHostOption_BedrockFog,         1);
    app.SetGameHostOption(eGameHostOption_GameType,
                          GameType::CREATIVE->getId());
    app.SetGameHostOption(eGameHostOption_LevelType,          0);
    app.SetGameHostOption(eGameHostOption_Structures,         1);
    app.SetGameHostOption(eGameHostOption_BonusChest,         0);
    app.SetGameHostOption(eGameHostOption_PvP,                1);
    app.SetGameHostOption(eGameHostOption_TrustPlayers,       1);
    app.SetGameHostOption(eGameHostOption_FireSpreads,        1);
    app.SetGameHostOption(eGameHostOption_TNT,                1);
    app.SetGameHostOption(eGameHostOption_HostCanFly,         1);
    app.SetGameHostOption(eGameHostOption_HostCanChangeHunger,1);
    app.SetGameHostOption(eGameHostOption_HostCanBeInvisible, 1);
    param->settings = app.GetGameHostOption(eGameHostOption_All);

    g_NetworkManager.FakeLocalPlayerJoined();

    LoadingInputParams* loadingParams = new LoadingInputParams();
    loadingParams->func               = &CGameNetworkManager::RunNetworkGameThreadProc;
    loadingParams->lpParam            = param;

    app.SetAutosaveTimerTime();

    C4JThread* thread = new C4JThread(loadingParams->func,
                                      loadingParams->lpParam, "RunNetworkGame");
    thread->run();
}

bool MacGame::TemporaryDirectConnectStart(const char* host, int port) {
    return TemporaryDirectConnectStartEx(host, port,  true,
                                          nullptr);
}

bool MacGame::TemporaryDirectConnectStartEx(const char* host, int port,
                                            bool spawnOwnThread,
                                            const wchar_t* nickname) {
    app.setLevelGenerationOptions(nullptr);

    Minecraft* pMinecraft = Minecraft::GetInstance();
    app.ReleaseSaveThumbnail();
    ProfileManager.SetLockedProfile(0);
    OverrideXuidBaseForDirectConnect();
    if (nickname != nullptr && nickname[0] != L'\0') {
        pMinecraft->user->name = std::wstring(nickname);
    } else {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        unsigned int suffix =
            (unsigned int)((ts.tv_nsec ^ (ts.tv_sec << 4) ^ getpid()) % 10000);
        wchar_t buf[32];
        swprintf(buf, sizeof(buf) / sizeof(buf[0]), L"Player%u", suffix);
        pMinecraft->user->name = std::wstring(buf);
    }
    app.ApplyGameSettingsChanged(0);

    MinecraftServer::resetFlags();

    app.SetTutorialMode(false);
    app.SetCorruptSaveDeleted(false);
    app.ClearTerrainFeaturePosition();
    app.SetGameHostOption(eGameHostOption_GameType,
                          GameType::CREATIVE->getId());
    _bQNetStubIsHost = false;
    _bQNetStubGameRunning = true;
    g_NetworkManager.SetLocalGame(false);
    g_NetworkManager.FakeLocalPlayerJoined();
    RemoteNetworkPlayer* remoteHost =
        RemoteNetworkPlayer::CreateForOutgoing(host ? host : "host");
    IPlatformNetworkStub::s_pRemoteHostOverride = remoteHost;

    fprintf(stderr, "[TCP] Direct-connect -> %s:%d\n", host, port);
    Socket* tcpSock = Socket::ConnectTcp(std::string(host), port, remoteHost);
    if (tcpSock == nullptr) {
        fprintf(stderr, "[TCP] Direct-connect FAILED, aborting client start.\n");
        IPlatformNetworkStub::s_pRemoteHostOverride = nullptr;
        delete remoteHost;
        _bQNetStubIsHost = true;
        _bQNetStubGameRunning = false;
        return false;
    }
    remoteHost->SetSocket(tcpSock);

    INetworkPlayer* localPlayer = g_NetworkManager.GetLocalPlayerByUserIndex(0);
    if (localPlayer == nullptr) {
        fprintf(stderr,
                "[TCP] No local player available - direct-connect aborted.\n");
        delete tcpSock;
        IPlatformNetworkStub::s_pRemoteHostOverride = nullptr;
        delete remoteHost;
        _bQNetStubIsHost = true;
        _bQNetStubGameRunning = false;
        return false;
    }
    localPlayer->SetSocket(tcpSock);

    NetworkGameInitData* param = new NetworkGameInitData();
    param->seed = 0;
    param->saveData = nullptr;
    param->settings = app.GetGameHostOption(eGameHostOption_All);

    app.SetAutosaveTimerTime();

    if (spawnOwnThread) {
        LoadingInputParams* loadingParams = new LoadingInputParams();
        loadingParams->func =
            &CGameNetworkManager::RunNetworkGameThreadProc;
        loadingParams->lpParam = param;
        C4JThread* thread = new C4JThread(loadingParams->func,
                                          loadingParams->lpParam,
                                          "RunNetworkGame");
        thread->run();
    } else {
        m_pendingDirectConnectParam = param;
    }
    return true;
}

int MacGame::GetLocalTMSFileIndex(wchar_t* wchTMSFile,
                                   bool     bFilenameIncludesExtension,
                                   eFileExtensionType eEXT) {
    return -1;
}

int MacGame::LoadLocalTMSFile(wchar_t* wchTMSFile) { return -1; }

int MacGame::LoadLocalTMSFile(wchar_t* wchTMSFile, eFileExtensionType eExt) {
    return -1;
}

void MacGame::FreeLocalTMSFiles(eTMSFileType eType) {}