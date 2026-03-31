#include "Minecraft.World/Header Files/stdafx.h"
#include "../Common/Consoles_App.h"
#include "../net/minecraft/client/User.h"
#include "../net/minecraft/client/Minecraft.h"
#include "../net/minecraft/server/MinecraftServer.h"
#include "../net/minecraft/server/PlayerList.h"
#include "../net/minecraft/server/level/ServerPlayer.h"
#include "Minecraft.World/net/minecraft/world/level/Level.h"
#include "Minecraft.World/net/minecraft/world/level/LevelSettings.h"
#include "Minecraft.World/net/minecraft/world/level/biome/BiomeSource.h"
#include "Minecraft.World/net/minecraft/world/level/LevelType.h"
#include "Linux_App.h"

CConsoleMinecraftApp app;

#define CONTEXT_GAME_STATE 0

CConsoleMinecraftApp::CConsoleMinecraftApp() : CMinecraftApp() {}

void CConsoleMinecraftApp::SetRichPresenceContext(int iPad, int contextId) {
}

void CConsoleMinecraftApp::StoreLaunchData() {}
void CConsoleMinecraftApp::ExitGame() {
    app.DebugPrintf("Linux_App CConsoleMinecraftApp::ExitGame AFTER START\n");
    RenderManager.Close();
}
void CConsoleMinecraftApp::FatalLoadError() {
    app.DebugPrintf(
        "CConsoleMinecraftApp::FatalLoadError - asserting 0 and dying...\n");
    assert(0);
}

void CConsoleMinecraftApp::CaptureSaveThumbnail() {}
void CConsoleMinecraftApp::GetSaveThumbnail(std::uint8_t** thumbnailData,
                                            unsigned int* thumbnailSize) {}
void CConsoleMinecraftApp::ReleaseSaveThumbnail() {}

void CConsoleMinecraftApp::GetScreenshot(int iPad,
                                         std::uint8_t** screenshotData,
                                         unsigned int* screenshotSize) {}

void CConsoleMinecraftApp::TemporaryCreateGameStart() {
    //////////////////////////////////////////////////////////////////////////////////////////////
    /// From CScene_Main::OnInit

    app.setLevelGenerationOptions(nullptr);

    // From CScene_Main::RunPlayGame
    Minecraft* pMinecraft = Minecraft::GetInstance();
    app.ReleaseSaveThumbnail();
    ProfileManager.SetLockedProfile(0);
    pMinecraft->user->name = L"Windows";
    app.ApplyGameSettingsChanged(0);

    //////////////////////////////////////////////////////////////////////////////////////////////
    /// From CScene_MultiGameJoinLoad::OnInit
    MinecraftServer::resetFlags();

    // From CScene_MultiGameJoinLoad::OnNotifyPressEx
    app.SetTutorialMode(false);
    app.SetCorruptSaveDeleted(false);

    //////////////////////////////////////////////////////////////////////////////////////////////
    /// From CScene_MultiGameCreate::CreateGame

    app.ClearTerrainFeaturePosition();
    std::wstring wWorldName = L"TestWorld";

    StorageManager.ResetSaveData();
    StorageManager.SetSaveTitle(wWorldName.c_str());

    bool isFlat = false;
    int64_t seedValue =
        0;  // BiomeSource::findSeed(isFlat?LevelType::lvl_flat:LevelType::lvl_normal);
            // // 4J - was (new Random())->nextLong() - now trying to actually
            // find a seed to suit our requirements

    NetworkGameInitData* param = new NetworkGameInitData();
    param->seed = seedValue;
    param->saveData = nullptr;

    app.SetGameHostOption(eGameHostOption_Difficulty, 0);
    app.SetGameHostOption(eGameHostOption_FriendsOfFriends, 0);
    app.SetGameHostOption(eGameHostOption_Gamertags, 1);
    app.SetGameHostOption(eGameHostOption_BedrockFog, 1);

    app.SetGameHostOption(
        eGameHostOption_GameType,
        GameType::CREATIVE->getId());  // LevelSettings::GAMETYPE_SURVIVAL
    app.SetGameHostOption(eGameHostOption_LevelType, 0);
    app.SetGameHostOption(eGameHostOption_Structures, 1);
    app.SetGameHostOption(eGameHostOption_BonusChest, 0);

    app.SetGameHostOption(eGameHostOption_PvP, 1);
    app.SetGameHostOption(eGameHostOption_TrustPlayers, 1);
    app.SetGameHostOption(eGameHostOption_FireSpreads, 1);
    app.SetGameHostOption(eGameHostOption_TNT, 1);
    app.SetGameHostOption(eGameHostOption_HostCanFly, 1);
    app.SetGameHostOption(eGameHostOption_HostCanChangeHunger, 1);
    app.SetGameHostOption(eGameHostOption_HostCanBeInvisible, 1);

    param->settings = app.GetGameHostOption(eGameHostOption_All);

    g_NetworkManager.FakeLocalPlayerJoined();

    LoadingInputParams* loadingParams = new LoadingInputParams();
    loadingParams->func = &CGameNetworkManager::RunNetworkGameThreadProc;
    loadingParams->lpParam = param;

    // Reset the autosave time
    app.SetAutosaveTimerTime();

    C4JThread* thread = new C4JThread(loadingParams->func,
                                      loadingParams->lpParam, "RunNetworkGame");
    thread->run();
}

int CConsoleMinecraftApp::GetLocalTMSFileIndex(wchar_t* wchTMSFile,
                                               bool bFilenameIncludesExtension,
                                               eFileExtensionType eEXT) {
    return -1;
}

int CConsoleMinecraftApp::LoadLocalTMSFile(wchar_t* wchTMSFile) { return -1; }

int CConsoleMinecraftApp::LoadLocalTMSFile(wchar_t* wchTMSFile,
                                           eFileExtensionType eExt) {
    return -1;
}

void CConsoleMinecraftApp::FreeLocalTMSFiles(eTMSFileType eType) {}
