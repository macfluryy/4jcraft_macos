
#include "../Minecraft.World/Platform/stdafx.h"

#include "../Minecraft.World/Recipes/Recipy.h"
#include "../Minecraft.Client/GameState/Options.h"
#include "../Minecraft.World/Util/AABB.h"
#include "../Minecraft.World/Util/Vec3.h"
#include "../Minecraft.Client/MinecraftServer.h"
#include "../Minecraft.Client/Level/MultiPlayerLevel.h"
#include "../Minecraft.Client/Rendering/GameRenderer.h"
#include "../Minecraft.Client/Rendering/EntityRenderers/ProgressRenderer.h"
#include "../Minecraft.Client/Rendering/LevelRenderer.h"
#include "../Minecraft.Client/Textures/MobSkinMemTextureProcessor.h"
#include "../Minecraft.Client/Minecraft.h"
#include "../Minecraft.Client/Network/ClientConnection.h"
#include "../Minecraft.Client/Player/MultiPlayerLocalPlayer.h"
#include "../Minecraft.Client/Player/LocalPlayer.h"
#include "../Minecraft.World/Player/Player.h"
#include "../Minecraft.World/Containers/Inventory.h"
#include "../Minecraft.World/Level/Level.h"
#include "../Minecraft.World/Blocks/TileEntities/FurnaceTileEntity.h"
#include "../Minecraft.World/Containers/Container.h"
#include "../Minecraft.World/Blocks/TileEntities/DispenserTileEntity.h"
#include "../Minecraft.World/Blocks/TileEntities/SignTileEntity.h"
#include "../Minecraft.World/Blocks/TileEntities/HopperTileEntity.h"
#include "../Minecraft.World/Util/PathHelper.h"
#include "../Minecraft.Client/GameState/StatsCounter.h"
#include "../Minecraft.Client/GameState/GameMode.h"
#include "../Minecraft.Client/Platform/Windows64/Social/SocialManager.h"
#include "Tutorial/TutorialMode.h"
#if defined(_WINDOWS64)
#include "../Minecraft.Client/Platform/Common/XML/ATGXmlParser.h"
#include "../Minecraft.Client/Platform/Common/XML/xmlFilesCallback.h"
#endif
#include "Minecraft_Macros.h"
#include "../Minecraft.Client/Network/PlayerList.h"
#include "../Minecraft.Client/Player/ServerPlayer.h"
#include "GameRules/ConsoleGameRules.h"
#include "GameRules/ConsoleSchematicFile.h"
#include "../Minecraft.World/IO/Streams/InputOutputStream.h"
#include "../Minecraft.World/Level/Storage/LevelSettings.h"
#include "../Minecraft.Client/Player/User.h"
#include <cstring>
#include "../Minecraft.World/Level/LevelData.h"
#include "../Minecraft.World/Headers/net.minecraft.world.entity.player.h"
#include "../Minecraft.Client/Rendering/EntityRenderers/EntityRenderDispatcher.h"
#include "../Minecraft.World/IO/Streams/Compression.h"
#include "../Minecraft.Client/Textures/Packs/TexturePackRepository.h"
#include "../Minecraft.Client/Textures/Packs/DLCTexturePack.h"
#include "DLC/DLCPack.h"
#include "../Minecraft.Client/Utils/StringTable.h"
#include "../Minecraft.Client/Utils/ArchiveFile.h"
#include "../Minecraft.Client/Minecraft.h"
#if defined(__linux__)
#include <unistd.h>
#include <climits>
#endif
#include "UI/UI.h"
#include "UI/UIScene_PauseMenu.h"

#include <thread>
#include <chrono>

#include "Leaderboards/LeaderboardManager.h"

// CMinecraftApp app;
unsigned int CMinecraftApp::m_uiLastSignInData = 0;

const float CMinecraftApp::fSafeZoneX = 64.0f;  // 5% of 1280
const float CMinecraftApp::fSafeZoneY = 36.0f;  // 5% of 720

int CMinecraftApp::s_iHTMLFontSizesA[eHTMLSize_COUNT] = {
    // 20,15,20,24
    20, 13, 20, 26};

CMinecraftApp::CMinecraftApp() {
    if (GAME_SETTINGS_PROFILE_DATA_BYTES != sizeof(GAME_SETTINGS)) {
        // 4J Stu - See comment for GAME_SETTINGS_PROFILE_DATA_BYTES in
        // Xbox_App.h
        DebugPrintf(
            "WARNING: The size of the profile GAME_SETTINGS struct has "
            "changed, so all stat data is likely incorrect. Is: %d, Should be: "
            "%d\n",
            sizeof(GAME_SETTINGS), GAME_SETTINGS_PROFILE_DATA_BYTES);
#if !defined(_CONTENT_PACKAGE)
        __debugbreak();
#endif
    }

    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
        m_eTMSAction[i] = eTMSAction_Idle;
        m_eXuiAction[i] = eAppAction_Idle;
        m_eXuiActionParam[i] = nullptr;
        // m_dwAdditionalModelParts[i] = 0;

        if (FAILED(XUserGetSigninInfo(i,
                                      XUSER_GET_SIGNIN_INFO_OFFLINE_XUID_ONLY,
                                      &m_currentSigninInfo[i]))) {
            m_currentSigninInfo[i].xuid = INVALID_XUID;
            m_currentSigninInfo[i].dwGuestNumber = 0;
        }
        DebugPrintf("Player at index %d has guest number %d\n", i,
                    m_currentSigninInfo[i].dwGuestNumber);

        m_bRead_BannedListA[i] = false;
        SetBanListCheck(i, false);

        m_uiOpacityCountDown[i] = 0;
    }
    m_eGlobalXuiAction = eAppAction_Idle;
    m_eGlobalXuiServerAction = eXuiServerAction_Idle;

    m_bResourcesLoaded = false;
    m_bGameStarted = false;
    m_bIsAppPaused = false;
    // m_bSplitScreenEnabled = false;

    m_bIntroRunning = false;
    m_eGameMode = eMode_Singleplayer;
    m_bLoadSavesFromFolderEnabled = false;
    m_bWriteSavesToFolderEnabled = false;
    // m_bInterfaceRenderingOff = false;
    // m_bHandRenderingOff = false;
    m_bTutorialMode = false;
    m_disconnectReason = DisconnectPacket::eDisconnect_None;
    m_bLiveLinkRequired = false;
    m_bChangingSessionType = false;
    m_bReallyChangingSessionType = false;

#if defined(_DEBUG_MENUS_ENABLED)

#if defined(_CONTENT_PACKAGE)
    m_bDebugOptions =
        false;  // make them off by default in a content package build
#else
    m_bDebugOptions = true;
#endif
#else
    m_bDebugOptions = false;
#endif

    // ZeroMemory(m_PreviewBuffer,sizeof(XSOCIAL_PREVIEWIMAGE)*XUSER_MAX_COUNT);

    m_xuidNotch = INVALID_XUID;

    ZeroMemory(&m_InviteData, sizeof(JoinFromInviteData));

    // 	m_bRead_TMS_XUIDS_XML=false;
    // 	m_bRead_TMS_DLCINFO_XML=false;

    m_pDLCFileBuffer = nullptr;
    m_dwDLCFileSize = 0;
    m_pBannedListFileBuffer = nullptr;
    m_dwBannedListFileSize = 0;

    m_bDefaultCapeInstallAttempted = false;
    m_bDLCInstallProcessCompleted = false;
    m_bDLCInstallPending = false;
    m_iTotalDLC = 0;
    m_iTotalDLCInstalled = 0;
    mfTrialPausedTime = 0.0f;
    m_uiAutosaveTimer = 0;
    ZeroMemory(m_pszUniqueMapName, 14);

    m_bNewDLCAvailable = false;
    m_bSeenNewDLCTip = false;

    m_uiGameHostSettings = 0;

#if defined(_LARGE_WORLDS)
    m_GameNewWorldSize = 0;
    m_bGameNewWorldSizeUseMoat = false;
    m_GameNewHellScale = 0;
#endif

    ZeroMemory(m_playerColours, MINECRAFT_NET_MAX_PLAYERS);

    m_iDLCOfferC = 0;
    m_bAllDLCContentRetrieved = true;
    m_bAllTMSContentRetrieved = true;
    m_bTickTMSDLCFiles = true;
    m_saveNotificationDepth = 0;

    m_dwRequiredTexturePackID = 0;

    m_bResetNether = false;

#if defined(_CONTENT_PACAKGE)
    m_bUseDPadForDebug = false;
#else
    m_bUseDPadForDebug = true;
#endif

    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
        m_vBannedListA[i] = new std::vector<PBANNEDLISTDATA>;
    }

    LocaleAndLanguageInit();
}

void CMinecraftApp::DebugPrintf(const char* szFormat, ...) {
#if !defined(_FINAL_BUILD)
    char buf[1024];
    va_list ap;
    va_start(ap, szFormat);
    vsnprintf(buf, sizeof(buf), szFormat, ap);
    va_end(ap);
    OutputDebugStringA(buf);
#endif
}

void CMinecraftApp::DebugPrintf(int user, const char* szFormat, ...) {
#if !defined(_FINAL_BUILD)
    if (user == USER_NONE) return;
    char buf[1024];
    va_list ap;
    va_start(ap, szFormat);
    vsnprintf(buf, sizeof(buf), szFormat, ap);
    va_end(ap);
    OutputDebugStringA(buf);
    if (user == USER_UI) {
        ui.logDebugString(buf);
    }
#endif
}

const wchar_t* CMinecraftApp::GetString(int iID) {
    // return L"Değişiklikler ve Yenilikler";
    // return L"ÕÕÕÕÖÖÖÖ";
    return app.m_stringTable->getString(iID);
}

void CMinecraftApp::SetAction(int iPad, eXuiAction action, void* param) {
    if ((m_eXuiAction[iPad] == eAppAction_ReloadTexturePack) &&
        (action == eAppAction_EthernetDisconnected)) {
        app.DebugPrintf(
            "Invalid change of App action for pad %d from %d to %d, ignoring\n",
            iPad, m_eXuiAction[iPad], action);
    } else if ((m_eXuiAction[iPad] == eAppAction_ReloadTexturePack) &&
               (action == eAppAction_ExitWorld)) {
        app.DebugPrintf(
            "Invalid change of App action for pad %d from %d to %d, ignoring\n",
            iPad, m_eXuiAction[iPad], action);
    } else if (m_eXuiAction[iPad] == eAppAction_ExitWorldCapturedThumbnail &&
               action != eAppAction_Idle) {
        app.DebugPrintf(
            "Invalid change of App action for pad %d from %d to %d, ignoring\n",
            iPad, m_eXuiAction[iPad], action);
    } else {
        app.DebugPrintf("Changing App action for pad %d from %d to %d\n", iPad,
                        m_eXuiAction[iPad], action);
        m_eXuiAction[iPad] = action;
        m_eXuiActionParam[iPad] = param;
    }
}

bool CMinecraftApp::IsAppPaused() { return m_bIsAppPaused; }

void CMinecraftApp::SetAppPaused(bool val) { m_bIsAppPaused = val; }

void CMinecraftApp::HandleButtonPresses() {
    for (int i = 0; i < 4; i++) {
        HandleButtonPresses(i);
    }
}

void CMinecraftApp::HandleButtonPresses(int iPad) {
    // 		// test an update of the profile data
    // 		void *pData=ProfileManager.GetGameDefinedProfileData(iPad);
    //
    // 		unsigned char *pchData= (unsigned char *)pData;
    // 		int iCount=0;
    // 		for(int i=0;i<GAME_DEFINED_PROFILE_DATA_BYTES;i++)
    // 		{
    // 			pchData[i]=0xBC;
    // 			//if(iCount==255) iCount = 0;
    // 		}
    //  		ProfileManager.WriteToProfile(iPad,true);
}

bool CMinecraftApp::LoadInventoryMenu(int iPad,
                                      std::shared_ptr<LocalPlayer> player,
                                      bool bNavigateBack) {
    bool success = true;

    InventoryScreenInput* initData = new InventoryScreenInput();
    initData->player = player;
    initData->bNavigateBack = bNavigateBack;
    initData->iPad = iPad;

    if (app.GetLocalPlayerCount() > 1) {
        initData->bSplitscreen = true;
        success = ui.NavigateToScene(iPad, eUIScene_InventoryMenu, initData);
    } else {
        initData->bSplitscreen = false;
        success = ui.NavigateToScene(iPad, eUIScene_InventoryMenu, initData);
    }

    return success;
}

bool CMinecraftApp::LoadCreativeMenu(int iPad,
                                     std::shared_ptr<LocalPlayer> player,
                                     bool bNavigateBack) {
    bool success = true;

    InventoryScreenInput* initData = new InventoryScreenInput();
    initData->player = player;
    initData->bNavigateBack = bNavigateBack;
    initData->iPad = iPad;

    if (app.GetLocalPlayerCount() > 1) {
        initData->bSplitscreen = true;
        success = ui.NavigateToScene(iPad, eUIScene_CreativeMenu, initData);
    } else {
        initData->bSplitscreen = false;
        success = ui.NavigateToScene(iPad, eUIScene_CreativeMenu, initData);
    }

    return success;
}

bool CMinecraftApp::LoadCrafting2x2Menu(int iPad,
                                        std::shared_ptr<LocalPlayer> player) {
    bool success = true;

    CraftingPanelScreenInput* initData = new CraftingPanelScreenInput();
    initData->player = player;
    initData->iContainerType = RECIPE_TYPE_2x2;
    initData->iPad = iPad;
    initData->x = 0;
    initData->y = 0;
    initData->z = 0;

    if (app.GetLocalPlayerCount() > 1) {
        initData->bSplitscreen = true;
        success = ui.NavigateToScene(iPad, eUIScene_Crafting2x2Menu, initData);
    } else {
        initData->bSplitscreen = false;
        success = ui.NavigateToScene(iPad, eUIScene_Crafting2x2Menu, initData);
    }

    return success;
}

bool CMinecraftApp::LoadCrafting3x3Menu(int iPad,
                                        std::shared_ptr<LocalPlayer> player,
                                        int x, int y, int z) {
    bool success = true;

    CraftingPanelScreenInput* initData = new CraftingPanelScreenInput();
    initData->player = player;
    initData->iContainerType = RECIPE_TYPE_3x3;
    initData->iPad = iPad;
    initData->x = x;
    initData->y = y;
    initData->z = z;

    if (app.GetLocalPlayerCount() > 1) {
        initData->bSplitscreen = true;
        success = ui.NavigateToScene(iPad, eUIScene_Crafting3x3Menu, initData);
    } else {
        initData->bSplitscreen = false;
        success = ui.NavigateToScene(iPad, eUIScene_Crafting3x3Menu, initData);
    }

    return success;
}

bool CMinecraftApp::LoadFireworksMenu(int iPad,
                                      std::shared_ptr<LocalPlayer> player,
                                      int x, int y, int z) {
    bool success = true;

    FireworksScreenInput* initData = new FireworksScreenInput();
    initData->player = player;
    initData->iPad = iPad;
    initData->x = x;
    initData->y = y;
    initData->z = z;

    if (app.GetLocalPlayerCount() > 1) {
        initData->bSplitscreen = true;
        success = ui.NavigateToScene(iPad, eUIScene_FireworksMenu, initData);
    } else {
        initData->bSplitscreen = false;
        success = ui.NavigateToScene(iPad, eUIScene_FireworksMenu, initData);
    }

    return success;
}

bool CMinecraftApp::LoadEnchantingMenu(int iPad,
                                       std::shared_ptr<Inventory> inventory,
                                       int x, int y, int z, Level* level,
                                       const std::wstring& name) {
    bool success = true;

    EnchantingScreenInput* initData = new EnchantingScreenInput();
    initData->inventory = inventory;
    initData->level = level;
    initData->x = x;
    initData->y = y;
    initData->z = z;
    initData->iPad = iPad;
    initData->name = name;

    if (app.GetLocalPlayerCount() > 1) {
        initData->bSplitscreen = true;
        success = ui.NavigateToScene(iPad, eUIScene_EnchantingMenu, initData);
    } else {
        initData->bSplitscreen = false;
        success = ui.NavigateToScene(iPad, eUIScene_EnchantingMenu, initData);
    }

    return success;
}

bool CMinecraftApp::LoadFurnaceMenu(
    int iPad, std::shared_ptr<Inventory> inventory,
    std::shared_ptr<FurnaceTileEntity> furnace) {
    bool success = true;

    FurnaceScreenInput* initData = new FurnaceScreenInput();

    initData->furnace = furnace;
    initData->inventory = inventory;
    initData->iPad = iPad;

    // Load the scene.
    if (app.GetLocalPlayerCount() > 1) {
        initData->bSplitscreen = true;
        success = ui.NavigateToScene(iPad, eUIScene_FurnaceMenu, initData);
    } else {
        initData->bSplitscreen = false;
        success = ui.NavigateToScene(iPad, eUIScene_FurnaceMenu, initData);
    }

    return success;
}

bool CMinecraftApp::LoadBrewingStandMenu(
    int iPad, std::shared_ptr<Inventory> inventory,
    std::shared_ptr<BrewingStandTileEntity> brewingStand) {
    bool success = true;

    BrewingScreenInput* initData = new BrewingScreenInput();

    initData->brewingStand = brewingStand;
    initData->inventory = inventory;
    initData->iPad = iPad;

    // Load the scene.
    if (app.GetLocalPlayerCount() > 1) {
        initData->bSplitscreen = true;
        success = ui.NavigateToScene(iPad, eUIScene_BrewingStandMenu, initData);
    } else {
        initData->bSplitscreen = false;
        success = ui.NavigateToScene(iPad, eUIScene_BrewingStandMenu, initData);
    }

    return success;
}

bool CMinecraftApp::LoadContainerMenu(int iPad,
                                      std::shared_ptr<Container> inventory,
                                      std::shared_ptr<Container> container) {
    bool success = true;

    ContainerScreenInput* initData = new ContainerScreenInput();

    initData->inventory = inventory;
    initData->container = container;
    initData->iPad = iPad;

    // Load the scene.
    if (app.GetLocalPlayerCount() > 1) {
        initData->bSplitscreen = true;

        bool bLargeChest =
            (initData->container->getContainerSize() > 3 * 9) ? true : false;
        if (bLargeChest) {
            success =
                ui.NavigateToScene(iPad, eUIScene_LargeContainerMenu, initData);
        } else {
            success =
                ui.NavigateToScene(iPad, eUIScene_ContainerMenu, initData);
        }
    } else {
        initData->bSplitscreen = false;
        success = ui.NavigateToScene(iPad, eUIScene_ContainerMenu, initData);
    }

    return success;
}

bool CMinecraftApp::LoadTrapMenu(int iPad, std::shared_ptr<Container> inventory,
                                 std::shared_ptr<DispenserTileEntity> trap) {
    bool success = true;

    TrapScreenInput* initData = new TrapScreenInput();

    initData->inventory = inventory;
    initData->trap = trap;
    initData->iPad = iPad;

    // Load the scene.
    if (app.GetLocalPlayerCount() > 1) {
        initData->bSplitscreen = true;
        success = ui.NavigateToScene(iPad, eUIScene_DispenserMenu, initData);
    } else {
        initData->bSplitscreen = false;
        success = ui.NavigateToScene(iPad, eUIScene_DispenserMenu, initData);
    }

    return success;
}

bool CMinecraftApp::LoadSignEntryMenu(int iPad,
                                      std::shared_ptr<SignTileEntity> sign) {
    bool success = true;

    SignEntryScreenInput* initData = new SignEntryScreenInput();

    initData->sign = sign;
    initData->iPad = iPad;

    success = ui.NavigateToScene(iPad, eUIScene_SignEntryMenu, initData);

    delete initData;

    return success;
}

bool CMinecraftApp::LoadRepairingMenu(int iPad,
                                      std::shared_ptr<Inventory> inventory,
                                      Level* level, int x, int y, int z) {
    bool success = true;

    AnvilScreenInput* initData = new AnvilScreenInput();
    initData->inventory = inventory;
    initData->level = level;
    initData->x = x;
    initData->y = y;
    initData->z = z;
    initData->iPad = iPad;
    if (app.GetLocalPlayerCount() > 1)
        initData->bSplitscreen = true;
    else
        initData->bSplitscreen = false;

    success = ui.NavigateToScene(iPad, eUIScene_AnvilMenu, initData);

    return success;
}

bool CMinecraftApp::LoadTradingMenu(int iPad,
                                    std::shared_ptr<Inventory> inventory,
                                    std::shared_ptr<Merchant> trader,
                                    Level* level, const std::wstring& name) {
    bool success = true;

    TradingScreenInput* initData = new TradingScreenInput();
    initData->inventory = inventory;
    initData->trader = trader;
    initData->level = level;
    initData->iPad = iPad;
    if (app.GetLocalPlayerCount() > 1)
        initData->bSplitscreen = true;
    else
        initData->bSplitscreen = false;

    success = ui.NavigateToScene(iPad, eUIScene_TradingMenu, initData);

    return success;
}

bool CMinecraftApp::LoadHopperMenu(int iPad,
                                   std::shared_ptr<Inventory> inventory,
                                   std::shared_ptr<HopperTileEntity> hopper) {
    bool success = true;

    HopperScreenInput* initData = new HopperScreenInput();
    initData->inventory = inventory;
    initData->hopper = hopper;
    initData->iPad = iPad;
    if (app.GetLocalPlayerCount() > 1)
        initData->bSplitscreen = true;
    else
        initData->bSplitscreen = false;

    success = ui.NavigateToScene(iPad, eUIScene_HopperMenu, initData);

    return success;
}

bool CMinecraftApp::LoadHopperMenu(int iPad,
                                   std::shared_ptr<Inventory> inventory,
                                   std::shared_ptr<MinecartHopper> hopper) {
    bool success = true;

    HopperScreenInput* initData = new HopperScreenInput();
    initData->inventory = inventory;
    initData->hopper = std::dynamic_pointer_cast<Container>(hopper);
    initData->iPad = iPad;
    if (app.GetLocalPlayerCount() > 1)
        initData->bSplitscreen = true;
    else
        initData->bSplitscreen = false;

    success = ui.NavigateToScene(iPad, eUIScene_HopperMenu, initData);

    return success;
}

bool CMinecraftApp::LoadHorseMenu(int iPad,
                                  std::shared_ptr<Inventory> inventory,
                                  std::shared_ptr<Container> container,
                                  std::shared_ptr<EntityHorse> horse) {
    bool success = true;

    HorseScreenInput* initData = new HorseScreenInput();
    initData->inventory = inventory;
    initData->container = container;
    initData->horse = horse;
    initData->iPad = iPad;
    if (app.GetLocalPlayerCount() > 1)
        initData->bSplitscreen = true;
    else
        initData->bSplitscreen = false;

    success = ui.NavigateToScene(iPad, eUIScene_HorseMenu, initData);

    return success;
}

bool CMinecraftApp::LoadBeaconMenu(int iPad,
                                   std::shared_ptr<Inventory> inventory,
                                   std::shared_ptr<BeaconTileEntity> beacon) {
    bool success = true;

    BeaconScreenInput* initData = new BeaconScreenInput();
    initData->inventory = inventory;
    initData->beacon = beacon;
    initData->iPad = iPad;
    if (app.GetLocalPlayerCount() > 1)
        initData->bSplitscreen = true;
    else
        initData->bSplitscreen = false;

    success = ui.NavigateToScene(iPad, eUIScene_BeaconMenu, initData);

    return success;
}

//////////////////////////////////////////////
// GAME SETTINGS
//////////////////////////////////////////////
void CMinecraftApp::InitGameSettings() {
    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
        GameSettingsA[i] =
            (GAME_SETTINGS*)ProfileManager.GetGameDefinedProfileData(i);
        // clear the flag to say the settings have changed
        GameSettingsA[i]->bSettingsChanged = false;

        // SetDefaultGameSettings(i); - done on a callback from the profile
        // manager

        // 4J-PB - adding in for Windows & PS3 to set the defaults for the
        // joypad
#if defined(_WINDOWS64)
        C_4JProfile::PROFILESETTINGS* pProfileSettings =
            ProfileManager.GetDashboardProfileSettings(i);
        // clear this for now - it will come from reading the system values
        memset(pProfileSettings, 0, sizeof(C_4JProfile::PROFILESETTINGS));
        SetDefaultOptions(pProfileSettings, i);
#else
        // 4jcrqaft: Linux (and any other platform): profile data is
        // zero-initialised, so explicitly apply defaults
        C_4JProfile::PROFILESETTINGS* pProfileSettings =
            ProfileManager.GetDashboardProfileSettings(i);
        memset(pProfileSettings, 0, sizeof(C_4JProfile::PROFILESETTINGS));
        SetDefaultOptions(pProfileSettings, i);
#endif
    }
}

int CMinecraftApp::SetDefaultOptions(C_4JProfile::PROFILESETTINGS* pSettings,
                                     const int iPad) {
    SetGameSettings(iPad, eGameSetting_MusicVolume, DEFAULT_VOLUME_LEVEL);
    SetGameSettings(iPad, eGameSetting_SoundFXVolume, DEFAULT_VOLUME_LEVEL);
    SetGameSettings(iPad, eGameSetting_Gamma, 50);

    // 4J-PB - Don't reset the difficult level if we're in-game
    if (Minecraft::GetInstance()->level == nullptr) {
        app.DebugPrintf("SetDefaultOptions - Difficulty = 1\n");
        SetGameSettings(iPad, eGameSetting_Difficulty, 1);
    }
    SetGameSettings(iPad, eGameSetting_Sensitivity_InGame, 100);
    SetGameSettings(iPad, eGameSetting_ViewBob, 1);
    SetGameSettings(iPad, eGameSetting_ControlScheme, 0);
    SetGameSettings(iPad, eGameSetting_ControlInvertLook,
                    (pSettings->iYAxisInversion != 0) ? 1 : 0);
    SetGameSettings(iPad, eGameSetting_ControlSouthPaw,
                    pSettings->bSwapSticks ? 1 : 0);
    SetGameSettings(iPad, eGameSetting_SplitScreenVertical, 0);
    SetGameSettings(iPad, eGameSetting_GamertagsVisible, 1);

    // Interim TU 1.6.6
    SetGameSettings(iPad, eGameSetting_Sensitivity_InMenu, 100);
    SetGameSettings(iPad, eGameSetting_DisplaySplitscreenGamertags, 1);
    SetGameSettings(iPad, eGameSetting_Hints, 1);
    SetGameSettings(iPad, eGameSetting_Autosave, 2);
    SetGameSettings(iPad, eGameSetting_Tooltips, 1);
    SetGameSettings(iPad, eGameSetting_InterfaceOpacity, 80);

    // TU 5
    SetGameSettings(iPad, eGameSetting_Clouds, 1);
    SetGameSettings(iPad, eGameSetting_Online, 1);
    SetGameSettings(iPad, eGameSetting_InviteOnly, 0);
    SetGameSettings(iPad, eGameSetting_FriendsOfFriends, 1);

    // default the update changes message to zero
    // 4J-PB - We'll only display the message if the profile is pre-TU5
    // SetGameSettings(iPad,eGameSetting_DisplayUpdateMessage,0);

    // TU 6
    SetGameSettings(iPad, eGameSetting_BedrockFog, 0);
    SetGameSettings(iPad, eGameSetting_DisplayHUD, 1);
    SetGameSettings(iPad, eGameSetting_DisplayHand, 1);

    // TU 7
    SetGameSettings(iPad, eGameSetting_CustomSkinAnim, 1);

    // TU 9
    SetGameSettings(iPad, eGameSetting_DeathMessages, 1);
    SetGameSettings(iPad, eGameSetting_UISize, 1);
    SetGameSettings(iPad, eGameSetting_UISizeSplitscreen, 2);
    SetGameSettings(iPad, eGameSetting_AnimatedCharacter, 1);

    // TU 12
    GameSettingsA[iPad]->ucCurrentFavoriteSkinPos = 0;
    for (int i = 0; i < MAX_FAVORITE_SKINS; i++) {
        GameSettingsA[iPad]->uiFavoriteSkinA[i] = 0xFFFFFFFF;
    }

    // TU 13
    GameSettingsA[iPad]->uiMashUpPackWorldsDisplay = 0xFFFFFFFF;

    // 1.6.4
    app.SetGameHostOption(eGameHostOption_MobGriefing, 1);
    app.SetGameHostOption(eGameHostOption_KeepInventory, 0);
    app.SetGameHostOption(eGameHostOption_DoMobSpawning, 1);
    app.SetGameHostOption(eGameHostOption_DoMobLoot, 1);
    app.SetGameHostOption(eGameHostOption_DoTileDrops, 1);
    app.SetGameHostOption(eGameHostOption_NaturalRegeneration, 1);
    app.SetGameHostOption(eGameHostOption_DoDaylightCycle, 1);

    // 4J-PB - leave these in, or remove from everywhere they are referenced!
    // Although probably best to leave in unless we split the profile settings
    // into platform specific classes - having different meaning per platform
    // for the same bitmask could get confusing
    // #ifdef 0
    // PS3DEC13
    SetGameSettings(iPad, eGameSetting_PS3_EULA_Read, 0);  // EULA not read

    // PS3 1.05 - added Greek

    // 4J-JEV: We cannot change these in-game, as they could affect localised
    // strings and font. XB1: Fix for #172947 - Content: Gameplay: While playing
    // in language different form system default one and resetting options to
    // their defaults in active gameplay causes in-game language to change and
    // HUD to disappear
    if (!app.GetGameStarted()) {
        GameSettingsA[iPad]->ucLanguage =
            MINECRAFT_LANGUAGE_DEFAULT;  // use the system language
        GameSettingsA[iPad]->ucLocale =
            MINECRAFT_LANGUAGE_DEFAULT;  // use the system locale
    }

    // #endif

    return 0;
}

int CMinecraftApp::DefaultOptionsCallback(
    void* pParam, C_4JProfile::PROFILESETTINGS* pSettings, const int iPad) {
    CMinecraftApp* pApp = (CMinecraftApp*)pParam;

    // flag the default options to be set

    pApp->DebugPrintf("Setting default options for player %d", iPad);
    pApp->SetAction(iPad, eAppAction_SetDefaultOptions, (void*)pSettings);
    // pApp->SetDefaultOptions(pSettings,iPad);

    // if the profile data has been changed, then force a profile write
    // It seems we're allowed to break the 5 minute rule if it's the result of a
    // user action
    // pApp->CheckGameSettingsChanged();

    return 0;
}

int CMinecraftApp::OldProfileVersionCallback(void* pParam,
                                             unsigned char* pucData,
                                             const unsigned short usVersion,
                                             const int iPad) {
    // check what needs to be done with this version to update to the current
    // one

    switch (usVersion) {
        case PROFILE_VERSION_8: {
            GAME_SETTINGS* pGameSettings = (GAME_SETTINGS*)pucData;
            // reset the display new message counter
            pGameSettings->uiBitmaskValues |=
                GAMESETTING_DISPLAYUPDATEMSG;  // eGameSetting_DisplayUpdateMessage
                                               // (counter)

            // Added a bitmask in TU13 to enable/disable display of the Mash-up
            // pack worlds in the saves list
            pGameSettings->uiMashUpPackWorldsDisplay = 0xFFFFFFFF;

            // PS3DEC13
            pGameSettings->uiBitmaskValues &=
                ~GAMESETTING_PS3EULAREAD;  // eGameSetting_PS3_EULA_Read - off

            // PS3 1.05 - added Greek
            pGameSettings->ucLanguage =
                MINECRAFT_LANGUAGE_DEFAULT;  // use the system language

        } break;
        case PROFILE_VERSION_9:
            // PS3DEC13
            {
                GAME_SETTINGS* pGameSettings = (GAME_SETTINGS*)pucData;
                pGameSettings->uiBitmaskValues |=
                    GAMESETTING_DISPLAYUPDATEMSG;  // eGameSetting_DisplayUpdateMessage
                                                   // (counter)
                pGameSettings->uiBitmaskValues &=
                    ~GAMESETTING_PS3EULAREAD;  // eGameSetting_PS3_EULA_Read -
                                               // off

                // PS3 1.05 - added Greek
                pGameSettings->ucLanguage =
                    MINECRAFT_LANGUAGE_DEFAULT;  // use the system language
            }
            break;
        case PROFILE_VERSION_10: {
            GAME_SETTINGS* pGameSettings = (GAME_SETTINGS*)pucData;
            pGameSettings->uiBitmaskValues |=
                GAMESETTING_DISPLAYUPDATEMSG;  // eGameSetting_DisplayUpdateMessage
                                               // (counter)
            pGameSettings->ucLanguage =
                MINECRAFT_LANGUAGE_DEFAULT;  // use the system language
        } break;
        case PROFILE_VERSION_11: {
            GAME_SETTINGS* pGameSettings = (GAME_SETTINGS*)pucData;
            pGameSettings->uiBitmaskValues |=
                GAMESETTING_DISPLAYUPDATEMSG;  // eGameSetting_DisplayUpdateMessage
                                               // (counter)
        } break;
        case PROFILE_VERSION_12: {
            GAME_SETTINGS* pGameSettings = (GAME_SETTINGS*)pucData;
            pGameSettings->uiBitmaskValues |=
                GAMESETTING_DISPLAYUPDATEMSG;  // eGameSetting_DisplayUpdateMessage
                                               // (counter)
        } break;
        default: {
            // This might be from a version during testing of new profile
            // updates
            app.DebugPrintf(
                "Don't know what to do with this profile version!\n");
#if !defined(_CONTENT_PACKAGE)
            //		__debugbreak();
#endif

            GAME_SETTINGS* pGameSettings = (GAME_SETTINGS*)pucData;
            pGameSettings->ucMenuSensitivity =
                100;  // eGameSetting_Sensitivity_InMenu
            pGameSettings->ucInterfaceOpacity =
                80;  // eGameSetting_Sensitivity_InMenu
            pGameSettings->usBitmaskValues |=
                0x0200;  // eGameSetting_DisplaySplitscreenGamertags - on
            pGameSettings->usBitmaskValues |= 0x0400;  // eGameSetting_Hints -
                                                       // on
            pGameSettings->usBitmaskValues |=
                0x1000;  // eGameSetting_Autosave - 2
            pGameSettings->usBitmaskValues |=
                0x8000;  // eGameSetting_Tooltips - on

            pGameSettings->uiBitmaskValues = 0L;  // reset
            pGameSettings->uiBitmaskValues |=
                GAMESETTING_CLOUDS;  // eGameSetting_Clouds - on
            pGameSettings->uiBitmaskValues |=
                GAMESETTING_ONLINE;  // eGameSetting_GameSetting_Online - on
            // eGameSetting_GameSetting_Invite - off
            pGameSettings->uiBitmaskValues |=
                GAMESETTING_FRIENDSOFFRIENDS;  // eGameSetting_GameSetting_FriendsOfFriends
                                               // - on
            pGameSettings->uiBitmaskValues |=
                GAMESETTING_DISPLAYUPDATEMSG;  // eGameSetting_DisplayUpdateMessage
                                               // (counter)
            pGameSettings->uiBitmaskValues &=
                ~GAMESETTING_BEDROCKFOG;  // eGameSetting_BedrockFog - off
            pGameSettings->uiBitmaskValues |=
                GAMESETTING_DISPLAYHUD;  // eGameSetting_DisplayHUD - on
            pGameSettings->uiBitmaskValues |=
                GAMESETTING_DISPLAYHAND;  // eGameSetting_DisplayHand - on
            pGameSettings->uiBitmaskValues |=
                GAMESETTING_CUSTOMSKINANIM;  // eGameSetting_CustomSkinAnim - on
            pGameSettings->uiBitmaskValues |=
                GAMESETTING_DEATHMESSAGES;  // eGameSetting_DeathMessages - on
            pGameSettings->uiBitmaskValues |=
                (GAMESETTING_UISIZE & 0x00000800);  // uisize 2
            pGameSettings->uiBitmaskValues |=
                (GAMESETTING_UISIZE_SPLITSCREEN &
                 0x00004000);  // splitscreen ui size 3
            pGameSettings->uiBitmaskValues |=
                GAMESETTING_ANIMATEDCHARACTER;  // eGameSetting_AnimatedCharacter
                                                // - on
            // TU12
            // favorite skins added, but only set in TU12 - set to FFs
            for (int i = 0; i < MAX_FAVORITE_SKINS; i++) {
                pGameSettings->uiFavoriteSkinA[i] = 0xFFFFFFFF;
            }
            pGameSettings->ucCurrentFavoriteSkinPos = 0;
            // Added a bitmask in TU13 to enable/disable display of the Mash-up
            // pack worlds in the saves list
            pGameSettings->uiMashUpPackWorldsDisplay = 0xFFFFFFFF;

            // PS3DEC13
            pGameSettings->uiBitmaskValues &=
                ~GAMESETTING_PS3EULAREAD;  // eGameSetting_PS3_EULA_Read - off

            // PS3 1.05 - added Greek
            pGameSettings->ucLanguage =
                MINECRAFT_LANGUAGE_DEFAULT;  // use the system language

        } break;
    }

    return 0;
}

void CMinecraftApp::ApplyGameSettingsChanged(int iPad) {
    ActionGameSettings(iPad, eGameSetting_MusicVolume);
    ActionGameSettings(iPad, eGameSetting_SoundFXVolume);
    ActionGameSettings(iPad, eGameSetting_Gamma);
    ActionGameSettings(iPad, eGameSetting_Difficulty);
    ActionGameSettings(iPad, eGameSetting_Sensitivity_InGame);
    ActionGameSettings(iPad, eGameSetting_ViewBob);
    ActionGameSettings(iPad, eGameSetting_ControlScheme);
    ActionGameSettings(iPad, eGameSetting_ControlInvertLook);
    ActionGameSettings(iPad, eGameSetting_ControlSouthPaw);
    ActionGameSettings(iPad, eGameSetting_SplitScreenVertical);
    ActionGameSettings(iPad, eGameSetting_GamertagsVisible);

    // Interim TU 1.6.6
    ActionGameSettings(iPad, eGameSetting_Sensitivity_InMenu);
    ActionGameSettings(iPad, eGameSetting_DisplaySplitscreenGamertags);
    ActionGameSettings(iPad, eGameSetting_Hints);
    ActionGameSettings(iPad, eGameSetting_InterfaceOpacity);
    ActionGameSettings(iPad, eGameSetting_Tooltips);

    ActionGameSettings(iPad, eGameSetting_Clouds);
    ActionGameSettings(iPad, eGameSetting_BedrockFog);
    ActionGameSettings(iPad, eGameSetting_DisplayHUD);
    ActionGameSettings(iPad, eGameSetting_DisplayHand);
    ActionGameSettings(iPad, eGameSetting_CustomSkinAnim);
    ActionGameSettings(iPad, eGameSetting_DeathMessages);
    ActionGameSettings(iPad, eGameSetting_UISize);
    ActionGameSettings(iPad, eGameSetting_UISizeSplitscreen);
    ActionGameSettings(iPad, eGameSetting_AnimatedCharacter);

    ActionGameSettings(iPad, eGameSetting_PS3_EULA_Read);
}

void CMinecraftApp::ActionGameSettings(int iPad, eGameSetting eVal) {
    Minecraft* pMinecraft = Minecraft::GetInstance();
    switch (eVal) {
        case eGameSetting_MusicVolume:
            if (iPad == ProfileManager.GetPrimaryPad()) {
                pMinecraft->options->set(
                    Options::Option::MUSIC,
                    ((float)GameSettingsA[iPad]->ucMusicVolume) / 100.0f);
            }
            break;
        case eGameSetting_SoundFXVolume:
            if (iPad == ProfileManager.GetPrimaryPad()) {
                pMinecraft->options->set(
                    Options::Option::SOUND,
                    ((float)GameSettingsA[iPad]->ucSoundFXVolume) / 100.0f);
            }
            break;
        case eGameSetting_Gamma:
            if (iPad == ProfileManager.GetPrimaryPad()) {
                // ucGamma range is 0-100, UpdateGamma is 0 - 32768
                float fVal = ((float)GameSettingsA[iPad]->ucGamma) * 327.68f;
                RenderManager.UpdateGamma((unsigned short)fVal);
            }

            break;
        case eGameSetting_Difficulty:
            if (iPad == ProfileManager.GetPrimaryPad()) {
                pMinecraft->options->toggle(
                    Options::Option::DIFFICULTY,
                    GameSettingsA[iPad]->usBitmaskValues & 0x03);
                app.DebugPrintf("Difficulty toggle to %d\n",
                                GameSettingsA[iPad]->usBitmaskValues & 0x03);

                // Update the Game Host setting
                app.SetGameHostOption(eGameHostOption_Difficulty,
                                      pMinecraft->options->difficulty);

                // send this to the other players if we are in-game
                bool bInGame = pMinecraft->level != nullptr;

                // Game Host only (and for now we can't change the diff while in
                // game, so this shouldn't happen)
                if (bInGame && g_NetworkManager.IsHost() &&
                    (iPad == ProfileManager.GetPrimaryPad())) {
                    app.SetXuiServerAction(
                        iPad, eXuiServerAction_ServerSettingChanged_Difficulty);
                }
            } else {
                app.DebugPrintf(
                    "NOT ACTIONING DIFFICULTY - Primary pad is %d, This pad is "
                    "%d\n",
                    ProfileManager.GetPrimaryPad(), iPad);
            }

            break;
        case eGameSetting_Sensitivity_InGame:
            // 4J-PB - we don't use the options value
            // tell the input that we've changed the sensitivity - range of the
            // slider is 0 to 200, default is 100
            pMinecraft->options->set(
                Options::Option::SENSITIVITY,
                ((float)GameSettingsA[iPad]->ucSensitivity) / 100.0f);
            // InputManager.SetJoypadSensitivity(iPad,((float)GameSettingsA[iPad]->ucSensitivity)/100.0f);

            break;
        case eGameSetting_ViewBob:
            // 4J-PB - not handled here any more - it's read from the
            // gamesettings per player
            // pMinecraft->options->toggle(Options::Option::VIEW_BOBBING,GameSettingsA[iPad]->usBitmaskValues&0x04);
            break;
        case eGameSetting_ControlScheme:
            InputManager.SetJoypadMapVal(
                iPad, (GameSettingsA[iPad]->usBitmaskValues & 0x30) >> 4);
            break;

        case eGameSetting_ControlInvertLook:
            // Nothing specific to do for this setting.
            break;

        case eGameSetting_ControlSouthPaw:
            // What is the setting?
            if (GameSettingsA[iPad]->usBitmaskValues & 0x80) {
                // Southpaw.
                InputManager.SetJoypadStickAxisMap(iPad, AXIS_MAP_LX,
                                                   AXIS_MAP_RX);
                InputManager.SetJoypadStickAxisMap(iPad, AXIS_MAP_LY,
                                                   AXIS_MAP_RY);
                InputManager.SetJoypadStickAxisMap(iPad, AXIS_MAP_RX,
                                                   AXIS_MAP_LX);
                InputManager.SetJoypadStickAxisMap(iPad, AXIS_MAP_RY,
                                                   AXIS_MAP_LY);
                InputManager.SetJoypadStickTriggerMap(iPad, TRIGGER_MAP_0,
                                                      TRIGGER_MAP_1);
                InputManager.SetJoypadStickTriggerMap(iPad, TRIGGER_MAP_1,
                                                      TRIGGER_MAP_0);
            } else {
                // Right handed.
                InputManager.SetJoypadStickAxisMap(iPad, AXIS_MAP_LX,
                                                   AXIS_MAP_LX);
                InputManager.SetJoypadStickAxisMap(iPad, AXIS_MAP_LY,
                                                   AXIS_MAP_LY);
                InputManager.SetJoypadStickAxisMap(iPad, AXIS_MAP_RX,
                                                   AXIS_MAP_RX);
                InputManager.SetJoypadStickAxisMap(iPad, AXIS_MAP_RY,
                                                   AXIS_MAP_RY);
                InputManager.SetJoypadStickTriggerMap(iPad, TRIGGER_MAP_0,
                                                      TRIGGER_MAP_0);
                InputManager.SetJoypadStickTriggerMap(iPad, TRIGGER_MAP_1,
                                                      TRIGGER_MAP_1);
            }
            break;
        case eGameSetting_SplitScreenVertical:
            if (iPad == ProfileManager.GetPrimaryPad()) {
                pMinecraft->updatePlayerViewportAssignments();
            }
            break;
        case eGameSetting_GamertagsVisible: {
            bool bInGame = pMinecraft->level != nullptr;

            // Game Host only
            if (bInGame && g_NetworkManager.IsHost() &&
                (iPad == ProfileManager.GetPrimaryPad())) {
                // Update the Game Host setting if you are the host and you are
                // in-game
                app.SetGameHostOption(
                    eGameHostOption_Gamertags,
                    ((GameSettingsA[iPad]->usBitmaskValues & 0x0008) != 0) ? 1
                                                                           : 0);
                app.SetXuiServerAction(
                    iPad, eXuiServerAction_ServerSettingChanged_Gamertags);

                PlayerList* players =
                    MinecraftServer::getInstance()->getPlayerList();
                for (auto it3 = players->players.begin();
                     it3 != players->players.end(); ++it3) {
                    std::shared_ptr<ServerPlayer> decorationPlayer = *it3;
                    decorationPlayer->setShowOnMaps(
                        (app.GetGameHostOption(eGameHostOption_Gamertags) != 0)
                            ? true
                            : false);
                }
            }
        } break;
            // Interim TU 1.6.6
        case eGameSetting_Sensitivity_InMenu:
            // 4J-PB - we don't use the options value
            // tell the input that we've changed the sensitivity - range of the
            // slider is 0 to 200, default is 100
            // pMinecraft->options->set(Options::Option::SENSITIVITY,((float)GameSettingsA[iPad]->ucSensitivity)/100.0f);
            // InputManager.SetJoypadSensitivity(iPad,((float)GameSettingsA[iPad]->ucSensitivity)/100.0f);

            break;

        case eGameSetting_DisplaySplitscreenGamertags:
            for (std::uint8_t idx = 0; idx < XUSER_MAX_COUNT; ++idx) {
                if (pMinecraft->localplayers[idx] != nullptr) {
                    if (pMinecraft->localplayers[idx]->m_iScreenSection ==
                        C4JRender::VIEWPORT_TYPE_FULLSCREEN) {
                        ui.DisplayGamertag(idx, false);
                    } else {
                        ui.DisplayGamertag(idx, true);
                    }
                }
            }

            break;
        case eGameSetting_InterfaceOpacity:
            // update the tooltips display
            ui.RefreshTooltips(iPad);

            break;
        case eGameSetting_Hints:
            // nothing to do here
            break;
        case eGameSetting_Tooltips:
            if ((GameSettingsA[iPad]->usBitmaskValues & 0x8000) != 0) {
                ui.SetEnableTooltips(iPad, true);
            } else {
                ui.SetEnableTooltips(iPad, false);
            }
            break;
        case eGameSetting_Clouds:
            // nothing to do here
            break;
        case eGameSetting_Online:
            // nothing to do here
            break;
        case eGameSetting_InviteOnly:
            // nothing to do here
            break;
        case eGameSetting_FriendsOfFriends:
            // nothing to do here
            break;
        case eGameSetting_BedrockFog: {
            bool bInGame = pMinecraft->level != nullptr;

            // Game Host only
            if (bInGame && g_NetworkManager.IsHost() &&
                (iPad == ProfileManager.GetPrimaryPad())) {
                // Update the Game Host setting if you are the host and you are
                // in-game
                app.SetGameHostOption(
                    eGameHostOption_BedrockFog,
                    GetGameSettings(iPad, eGameSetting_BedrockFog) ? 1 : 0);
                app.SetXuiServerAction(
                    iPad, eXuiServerAction_ServerSettingChanged_BedrockFog);
            }
        } break;
        case eGameSetting_DisplayHUD:
            // nothing to do here
            break;
        case eGameSetting_DisplayHand:
            // nothing to do here
            break;
        case eGameSetting_CustomSkinAnim:
            // nothing to do here
            break;
        case eGameSetting_DeathMessages:
            // nothing to do here
            break;
        case eGameSetting_UISize:
            // nothing to do here
            break;
        case eGameSetting_UISizeSplitscreen:
            // nothing to do here
            break;
        case eGameSetting_AnimatedCharacter:
            // nothing to do here
            break;
        case eGameSetting_PS3_EULA_Read:
            // nothing to do here
            break;
        case eGameSetting_PSVita_NetworkModeAdhoc:
            // nothing to do here
            break;
        default:
            break;
    }
}

void CMinecraftApp::SetPlayerSkin(int iPad, const std::wstring& name) {
    std::uint32_t skinId = app.getSkinIdFromPath(name);

    SetPlayerSkin(iPad, skinId);
}

void CMinecraftApp::SetPlayerSkin(int iPad, std::uint32_t dwSkinId) {
    DebugPrintf("Setting skin for %d to %08X\n", iPad, dwSkinId);

    GameSettingsA[iPad]->dwSelectedSkin = dwSkinId;
    GameSettingsA[iPad]->bSettingsChanged = true;

    TelemetryManager->RecordSkinChanged(iPad,
                                        GameSettingsA[iPad]->dwSelectedSkin);

    if (Minecraft::GetInstance()->localplayers[iPad] != nullptr)
        Minecraft::GetInstance()->localplayers[iPad]->setAndBroadcastCustomSkin(
            dwSkinId);
}

std::wstring CMinecraftApp::GetPlayerSkinName(int iPad) {
    return app.getSkinPathFromId(GameSettingsA[iPad]->dwSelectedSkin);
}

std::uint32_t CMinecraftApp::GetPlayerSkinId(int iPad) {
    // 4J-PB -check the user has rights to use this skin - they may have had at
    // some point but the entitlement has been removed.
    DLCPack* Pack = nullptr;
    DLCSkinFile* skinFile = nullptr;
    std::uint32_t dwSkin = GameSettingsA[iPad]->dwSelectedSkin;
    wchar_t chars[256];

    if (GET_IS_DLC_SKIN_FROM_BITMASK(dwSkin)) {
        // 4J Stu - DLC skins are numbered using decimal rather than hex to make
        // it easier to number manually
        swprintf(chars, 256, L"dlcskin%08d.png",
                 GET_DLC_SKIN_ID_FROM_BITMASK(dwSkin));

        Pack = app.m_dlcManager.getPackContainingSkin(chars);

        if (Pack) {
            skinFile = Pack->getSkinFile(chars);

            bool bSkinIsFree =
                skinFile->getParameterAsBool(DLCManager::e_DLCParamType_Free);
            bool bLicensed = Pack->hasPurchasedFile(DLCManager::e_DLCType_Skin,
                                                    skinFile->getPath());

            if (bSkinIsFree || bLicensed) {
                return dwSkin;
            } else {
                return 0;
            }
        }
    }

    return dwSkin;
}

std::uint32_t CMinecraftApp::GetAdditionalModelParts(int iPad) {
    return m_dwAdditionalModelParts[iPad];
}

void CMinecraftApp::SetPlayerCape(int iPad, const std::wstring& name) {
    std::uint32_t capeId = Player::getCapeIdFromPath(name);

    SetPlayerCape(iPad, capeId);
}

void CMinecraftApp::SetPlayerCape(int iPad, std::uint32_t dwCapeId) {
    DebugPrintf("Setting cape for %d to %08X\n", iPad, dwCapeId);

    GameSettingsA[iPad]->dwSelectedCape = dwCapeId;
    GameSettingsA[iPad]->bSettingsChanged = true;

    // SentientManager.RecordSkinChanged(iPad,
    // GameSettingsA[iPad]->dwSelectedSkin);

    if (Minecraft::GetInstance()->localplayers[iPad] != nullptr)
        Minecraft::GetInstance()->localplayers[iPad]->setAndBroadcastCustomCape(
            dwCapeId);
}

std::wstring CMinecraftApp::GetPlayerCapeName(int iPad) {
    return Player::getCapePathFromId(GameSettingsA[iPad]->dwSelectedCape);
}

std::uint32_t CMinecraftApp::GetPlayerCapeId(int iPad) {
    return GameSettingsA[iPad]->dwSelectedCape;
}

void CMinecraftApp::SetPlayerFavoriteSkin(int iPad, int iIndex,
                                          unsigned int uiSkinID) {
    DebugPrintf("Setting favorite skin for %d to %08X\n", iPad, uiSkinID);

    GameSettingsA[iPad]->uiFavoriteSkinA[iIndex] = uiSkinID;
    GameSettingsA[iPad]->bSettingsChanged = true;
}

unsigned int CMinecraftApp::GetPlayerFavoriteSkin(int iPad, int iIndex) {
    return GameSettingsA[iPad]->uiFavoriteSkinA[iIndex];
}

unsigned char CMinecraftApp::GetPlayerFavoriteSkinsPos(int iPad) {
    return GameSettingsA[iPad]->ucCurrentFavoriteSkinPos;
}

void CMinecraftApp::SetPlayerFavoriteSkinsPos(int iPad, int iPos) {
    GameSettingsA[iPad]->ucCurrentFavoriteSkinPos = (unsigned char)iPos;
    GameSettingsA[iPad]->bSettingsChanged = true;
}

unsigned int CMinecraftApp::GetPlayerFavoriteSkinsCount(int iPad) {
    unsigned int uiCount = 0;
    for (int i = 0; i < MAX_FAVORITE_SKINS; i++) {
        if (GameSettingsA[iPad]->uiFavoriteSkinA[i] != 0xFFFFFFFF) {
            uiCount++;
        } else {
            break;
        }
    }
    return uiCount;
}

void CMinecraftApp::ValidateFavoriteSkins(int iPad) {
    unsigned int uiCount = GetPlayerFavoriteSkinsCount(iPad);

    // remove invalid skins
    unsigned int uiValidSkin = 0;
    wchar_t chars[256];

    for (unsigned int i = 0; i < uiCount; i++) {
        // get the pack number from the skin id
        swprintf(chars, 256, L"dlcskin%08d.png",
                 app.GetPlayerFavoriteSkin(iPad, i));

        // Also check they haven't reverted to a trial pack
        DLCPack* pDLCPack = app.m_dlcManager.getPackContainingSkin(chars);

        if (pDLCPack != nullptr) {
            // 4J-PB - We should let players add the free skins to their
            // favourites as well!
            // DLCFile
            // *pDLCFile=pDLCPack->getFile(DLCManager::e_DLCType_Skin,chars);
            DLCSkinFile* pSkinFile = pDLCPack->getSkinFile(chars);

            if (pDLCPack->hasPurchasedFile(DLCManager::e_DLCType_Skin, L"") ||
                (pSkinFile && pSkinFile->isFree())) {
                GameSettingsA[iPad]->uiFavoriteSkinA[uiValidSkin++] =
                    GameSettingsA[iPad]->uiFavoriteSkinA[i];
            }
        }
    }

    for (unsigned int i = uiValidSkin; i < MAX_FAVORITE_SKINS; i++) {
        GameSettingsA[iPad]->uiFavoriteSkinA[i] = 0xFFFFFFFF;
    }
}

// Mash-up pack worlds
void CMinecraftApp::HideMashupPackWorld(int iPad, unsigned int iMashupPackID) {
    unsigned int uiPackID = iMashupPackID - 1024;  // mash-up ids start at 1024
    GameSettingsA[iPad]->uiMashUpPackWorldsDisplay &= ~(1 << uiPackID);
    GameSettingsA[iPad]->bSettingsChanged = true;
}

void CMinecraftApp::EnableMashupPackWorlds(int iPad) {
    GameSettingsA[iPad]->uiMashUpPackWorldsDisplay = 0xFFFFFFFF;
    GameSettingsA[iPad]->bSettingsChanged = true;
}

unsigned int CMinecraftApp::GetMashupPackWorlds(int iPad) {
    return GameSettingsA[iPad]->uiMashUpPackWorldsDisplay;
}

void CMinecraftApp::SetMinecraftLanguage(int iPad, unsigned char ucLanguage) {
    GameSettingsA[iPad]->ucLanguage = ucLanguage;
    GameSettingsA[iPad]->bSettingsChanged = true;
}

unsigned char CMinecraftApp::GetMinecraftLanguage(int iPad) {
    // if there are no game settings read yet, return the default language
    if (GameSettingsA[iPad] == nullptr) {
        return 0;
    } else {
        return GameSettingsA[iPad]->ucLanguage;
    }
}

void CMinecraftApp::SetMinecraftLocale(int iPad, unsigned char ucLocale) {
    GameSettingsA[iPad]->ucLocale = ucLocale;
    GameSettingsA[iPad]->bSettingsChanged = true;
}

unsigned char CMinecraftApp::GetMinecraftLocale(int iPad) {
    // if there are no game settings read yet, return the default language
    if (GameSettingsA[iPad] == nullptr) {
        return 0;
    } else {
        return GameSettingsA[iPad]->ucLocale;
    }
}

void CMinecraftApp::SetGameSettings(int iPad, eGameSetting eVal,
                                    unsigned char ucVal) {
    // Minecraft *pMinecraft=Minecraft::GetInstance();

    switch (eVal) {
        case eGameSetting_MusicVolume:
            if (GameSettingsA[iPad]->ucMusicVolume != ucVal) {
                GameSettingsA[iPad]->ucMusicVolume = ucVal;
                if (iPad == ProfileManager.GetPrimaryPad()) {
                    ActionGameSettings(iPad, eVal);
                }
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;
        case eGameSetting_SoundFXVolume:
            if (GameSettingsA[iPad]->ucSoundFXVolume != ucVal) {
                GameSettingsA[iPad]->ucSoundFXVolume = ucVal;
                if (iPad == ProfileManager.GetPrimaryPad()) {
                    ActionGameSettings(iPad, eVal);
                }
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;
        case eGameSetting_Gamma:
            if (GameSettingsA[iPad]->ucGamma != ucVal) {
                GameSettingsA[iPad]->ucGamma = ucVal;
                if (iPad == ProfileManager.GetPrimaryPad()) {
                    ActionGameSettings(iPad, eVal);
                }
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;
        case eGameSetting_Difficulty:
            if ((GameSettingsA[iPad]->usBitmaskValues & 0x03) !=
                (ucVal & 0x03)) {
                GameSettingsA[iPad]->usBitmaskValues &= ~0x03;
                GameSettingsA[iPad]->usBitmaskValues |= ucVal & 0x03;
                if (iPad == ProfileManager.GetPrimaryPad()) {
                    ActionGameSettings(iPad, eVal);
                }
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;
        case eGameSetting_Sensitivity_InGame:
            if (GameSettingsA[iPad]->ucSensitivity != ucVal) {
                GameSettingsA[iPad]->ucSensitivity = ucVal;
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;
        case eGameSetting_ViewBob:
            if ((GameSettingsA[iPad]->usBitmaskValues & 0x0004) !=
                ((ucVal & 0x01) << 2)) {
                if (ucVal != 0) {
                    GameSettingsA[iPad]->usBitmaskValues |= 0x0004;
                } else {
                    GameSettingsA[iPad]->usBitmaskValues &= ~0x0004;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;
        case eGameSetting_ControlScheme:  // bits 5 and 6
            if ((GameSettingsA[iPad]->usBitmaskValues & 0x30) !=
                ((ucVal & 0x03) << 4)) {
                GameSettingsA[iPad]->usBitmaskValues &= ~0x0030;
                if (ucVal != 0) {
                    GameSettingsA[iPad]->usBitmaskValues |= (ucVal & 0x03) << 4;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;

        case eGameSetting_ControlInvertLook:
            if ((GameSettingsA[iPad]->usBitmaskValues & 0x0040) !=
                ((ucVal & 0x01) << 6)) {
                if (ucVal != 0) {
                    GameSettingsA[iPad]->usBitmaskValues |= 0x0040;
                } else {
                    GameSettingsA[iPad]->usBitmaskValues &= ~0x0040;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;

        case eGameSetting_ControlSouthPaw:
            if ((GameSettingsA[iPad]->usBitmaskValues & 0x0080) !=
                ((ucVal & 0x01) << 7)) {
                if (ucVal != 0) {
                    GameSettingsA[iPad]->usBitmaskValues |= 0x0080;
                } else {
                    GameSettingsA[iPad]->usBitmaskValues &= ~0x0080;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;
        case eGameSetting_SplitScreenVertical:
            if ((GameSettingsA[iPad]->usBitmaskValues & 0x0100) !=
                ((ucVal & 0x01) << 8)) {
                if (ucVal != 0) {
                    GameSettingsA[iPad]->usBitmaskValues |= 0x0100;
                } else {
                    GameSettingsA[iPad]->usBitmaskValues &= ~0x0100;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;
        case eGameSetting_GamertagsVisible:
            if ((GameSettingsA[iPad]->usBitmaskValues & 0x0008) !=
                ((ucVal & 0x01) << 3)) {
                if (ucVal != 0) {
                    GameSettingsA[iPad]->usBitmaskValues |= 0x0008;
                } else {
                    GameSettingsA[iPad]->usBitmaskValues &= ~0x0008;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;

            // 4J-PB - Added for Interim TU for 1.6.6
        case eGameSetting_Sensitivity_InMenu:
            if (GameSettingsA[iPad]->ucMenuSensitivity != ucVal) {
                GameSettingsA[iPad]->ucMenuSensitivity = ucVal;
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;
        case eGameSetting_DisplaySplitscreenGamertags:
            if ((GameSettingsA[iPad]->usBitmaskValues & 0x0200) !=
                ((ucVal & 0x01) << 9)) {
                if (ucVal != 0) {
                    GameSettingsA[iPad]->usBitmaskValues |= 0x0200;
                } else {
                    GameSettingsA[iPad]->usBitmaskValues &= ~0x0200;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;
        case eGameSetting_Hints:
            if ((GameSettingsA[iPad]->usBitmaskValues & 0x0400) !=
                ((ucVal & 0x01) << 10)) {
                if (ucVal != 0) {
                    GameSettingsA[iPad]->usBitmaskValues |= 0x0400;
                } else {
                    GameSettingsA[iPad]->usBitmaskValues &= ~0x0400;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;
        case eGameSetting_Autosave:
            if ((GameSettingsA[iPad]->usBitmaskValues & 0x7800) !=
                ((ucVal & 0x0F) << 11)) {
                GameSettingsA[iPad]->usBitmaskValues &= ~0x7800;
                if (ucVal != 0) {
                    GameSettingsA[iPad]->usBitmaskValues |= (ucVal & 0x0F)
                                                            << 11;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;

        case eGameSetting_Tooltips:
            if ((GameSettingsA[iPad]->usBitmaskValues & 0x8000) !=
                ((ucVal & 0x01) << 15)) {
                if (ucVal != 0) {
                    GameSettingsA[iPad]->usBitmaskValues |= 0x8000;
                } else {
                    GameSettingsA[iPad]->usBitmaskValues &= ~0x8000;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;
        case eGameSetting_InterfaceOpacity:
            if (GameSettingsA[iPad]->ucInterfaceOpacity != ucVal) {
                GameSettingsA[iPad]->ucInterfaceOpacity = ucVal;
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }

            break;
        case eGameSetting_Clouds:
            if ((GameSettingsA[iPad]->uiBitmaskValues & GAMESETTING_CLOUDS) !=
                (ucVal & 0x01)) {
                if (ucVal == 1) {
                    GameSettingsA[iPad]->uiBitmaskValues |= GAMESETTING_CLOUDS;
                } else {
                    GameSettingsA[iPad]->uiBitmaskValues &= ~GAMESETTING_CLOUDS;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }

            break;

        case eGameSetting_Online:
            if ((GameSettingsA[iPad]->uiBitmaskValues & GAMESETTING_ONLINE) !=
                (ucVal & 0x01) << 1) {
                if (ucVal == 1) {
                    GameSettingsA[iPad]->uiBitmaskValues |= GAMESETTING_ONLINE;
                } else {
                    GameSettingsA[iPad]->uiBitmaskValues &= ~GAMESETTING_ONLINE;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }

            break;
        case eGameSetting_InviteOnly:
            if ((GameSettingsA[iPad]->uiBitmaskValues &
                 GAMESETTING_INVITEONLY) != (ucVal & 0x01) << 2) {
                if (ucVal == 1) {
                    GameSettingsA[iPad]->uiBitmaskValues |=
                        GAMESETTING_INVITEONLY;
                } else {
                    GameSettingsA[iPad]->uiBitmaskValues &=
                        ~GAMESETTING_INVITEONLY;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }

            break;
        case eGameSetting_FriendsOfFriends:
            if ((GameSettingsA[iPad]->uiBitmaskValues &
                 GAMESETTING_FRIENDSOFFRIENDS) != (ucVal & 0x01) << 3) {
                if (ucVal == 1) {
                    GameSettingsA[iPad]->uiBitmaskValues |=
                        GAMESETTING_FRIENDSOFFRIENDS;
                } else {
                    GameSettingsA[iPad]->uiBitmaskValues &=
                        ~GAMESETTING_FRIENDSOFFRIENDS;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }

            break;
        case eGameSetting_DisplayUpdateMessage:
            if ((GameSettingsA[iPad]->uiBitmaskValues &
                 GAMESETTING_DISPLAYUPDATEMSG) != (ucVal & 0x03) << 4) {
                GameSettingsA[iPad]->uiBitmaskValues &=
                    ~GAMESETTING_DISPLAYUPDATEMSG;
                if (ucVal > 0) {
                    GameSettingsA[iPad]->uiBitmaskValues |= (ucVal & 0x03) << 4;
                }

                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }

            break;

        case eGameSetting_BedrockFog:
            if ((GameSettingsA[iPad]->uiBitmaskValues &
                 GAMESETTING_BEDROCKFOG) != (ucVal & 0x01) << 6) {
                if (ucVal == 1) {
                    GameSettingsA[iPad]->uiBitmaskValues |=
                        GAMESETTING_BEDROCKFOG;
                } else {
                    GameSettingsA[iPad]->uiBitmaskValues &=
                        ~GAMESETTING_BEDROCKFOG;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }

            break;
        case eGameSetting_DisplayHUD:
            if ((GameSettingsA[iPad]->uiBitmaskValues &
                 GAMESETTING_DISPLAYHUD) != (ucVal & 0x01) << 7) {
                if (ucVal == 1) {
                    GameSettingsA[iPad]->uiBitmaskValues |=
                        GAMESETTING_DISPLAYHUD;
                } else {
                    GameSettingsA[iPad]->uiBitmaskValues &=
                        ~GAMESETTING_DISPLAYHUD;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }

            break;
        case eGameSetting_DisplayHand:
            if ((GameSettingsA[iPad]->uiBitmaskValues &
                 GAMESETTING_DISPLAYHAND) != (ucVal & 0x01) << 8) {
                if (ucVal == 1) {
                    GameSettingsA[iPad]->uiBitmaskValues |=
                        GAMESETTING_DISPLAYHAND;
                } else {
                    GameSettingsA[iPad]->uiBitmaskValues &=
                        ~GAMESETTING_DISPLAYHAND;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }

            break;

        case eGameSetting_CustomSkinAnim:
            if ((GameSettingsA[iPad]->uiBitmaskValues &
                 GAMESETTING_CUSTOMSKINANIM) != (ucVal & 0x01) << 9) {
                if (ucVal == 1) {
                    GameSettingsA[iPad]->uiBitmaskValues |=
                        GAMESETTING_CUSTOMSKINANIM;
                } else {
                    GameSettingsA[iPad]->uiBitmaskValues &=
                        ~GAMESETTING_CUSTOMSKINANIM;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }

            break;
            // TU9
        case eGameSetting_DeathMessages:
            if ((GameSettingsA[iPad]->uiBitmaskValues &
                 GAMESETTING_DEATHMESSAGES) != (ucVal & 0x01) << 10) {
                if (ucVal == 1) {
                    GameSettingsA[iPad]->uiBitmaskValues |=
                        GAMESETTING_DEATHMESSAGES;
                } else {
                    GameSettingsA[iPad]->uiBitmaskValues &=
                        ~GAMESETTING_DEATHMESSAGES;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;
        case eGameSetting_UISize:
            if ((GameSettingsA[iPad]->uiBitmaskValues & GAMESETTING_UISIZE) !=
                ((ucVal & 0x03) << 11)) {
                GameSettingsA[iPad]->uiBitmaskValues &= ~GAMESETTING_UISIZE;
                if (ucVal != 0) {
                    GameSettingsA[iPad]->uiBitmaskValues |= (ucVal & 0x03)
                                                            << 11;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;
        case eGameSetting_UISizeSplitscreen:
            if ((GameSettingsA[iPad]->uiBitmaskValues &
                 GAMESETTING_UISIZE_SPLITSCREEN) != ((ucVal & 0x03) << 13)) {
                GameSettingsA[iPad]->uiBitmaskValues &=
                    ~GAMESETTING_UISIZE_SPLITSCREEN;
                if (ucVal != 0) {
                    GameSettingsA[iPad]->uiBitmaskValues |= (ucVal & 0x03)
                                                            << 13;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;
        case eGameSetting_AnimatedCharacter:
            if ((GameSettingsA[iPad]->uiBitmaskValues &
                 GAMESETTING_ANIMATEDCHARACTER) != (ucVal & 0x01) << 15) {
                if (ucVal == 1) {
                    GameSettingsA[iPad]->uiBitmaskValues |=
                        GAMESETTING_ANIMATEDCHARACTER;
                } else {
                    GameSettingsA[iPad]->uiBitmaskValues &=
                        ~GAMESETTING_ANIMATEDCHARACTER;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;
        case eGameSetting_PS3_EULA_Read:
            if ((GameSettingsA[iPad]->uiBitmaskValues &
                 GAMESETTING_PS3EULAREAD) != (ucVal & 0x01) << 16) {
                if (ucVal == 1) {
                    GameSettingsA[iPad]->uiBitmaskValues |=
                        GAMESETTING_PS3EULAREAD;
                } else {
                    GameSettingsA[iPad]->uiBitmaskValues &=
                        ~GAMESETTING_PS3EULAREAD;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;
        case eGameSetting_PSVita_NetworkModeAdhoc:
            if ((GameSettingsA[iPad]->uiBitmaskValues &
                 GAMESETTING_PSVITANETWORKMODEADHOC) != (ucVal & 0x01) << 17) {
                if (ucVal == 1) {
                    GameSettingsA[iPad]->uiBitmaskValues |=
                        GAMESETTING_PSVITANETWORKMODEADHOC;
                } else {
                    GameSettingsA[iPad]->uiBitmaskValues &=
                        ~GAMESETTING_PSVITANETWORKMODEADHOC;
                }
                ActionGameSettings(iPad, eVal);
                GameSettingsA[iPad]->bSettingsChanged = true;
            }
            break;
    }
}

unsigned char CMinecraftApp::GetGameSettings(eGameSetting eVal) {
    int iPad = ProfileManager.GetPrimaryPad();

    return GetGameSettings(iPad, eVal);
}

unsigned char CMinecraftApp::GetGameSettings(int iPad, eGameSetting eVal) {
    switch (eVal) {
        case eGameSetting_MusicVolume:
            return GameSettingsA[iPad]->ucMusicVolume;
            break;
        case eGameSetting_SoundFXVolume:
            return GameSettingsA[iPad]->ucSoundFXVolume;
            break;
        case eGameSetting_Gamma:
            return GameSettingsA[iPad]->ucGamma;
            break;
        case eGameSetting_Difficulty:
            return GameSettingsA[iPad]->usBitmaskValues & 0x0003;
            break;
        case eGameSetting_Sensitivity_InGame:
            return GameSettingsA[iPad]->ucSensitivity;
            break;
        case eGameSetting_ViewBob:
            return ((GameSettingsA[iPad]->usBitmaskValues & 0x0004) >> 2);
            break;
        case eGameSetting_GamertagsVisible:
            return ((GameSettingsA[iPad]->usBitmaskValues & 0x0008) >> 3);
            break;
        case eGameSetting_ControlScheme:
            return ((GameSettingsA[iPad]->usBitmaskValues & 0x0030) >>
                    4);  // 2 bits
            break;
        case eGameSetting_ControlInvertLook:
            return ((GameSettingsA[iPad]->usBitmaskValues & 0x0040) >> 6);
            break;
        case eGameSetting_ControlSouthPaw:
            return ((GameSettingsA[iPad]->usBitmaskValues & 0x0080) >> 7);
            break;
        case eGameSetting_SplitScreenVertical:
            return ((GameSettingsA[iPad]->usBitmaskValues & 0x0100) >> 8);
            break;
            // 4J-PB - Added for Interim TU for 1.6.6
        case eGameSetting_Sensitivity_InMenu:
            return GameSettingsA[iPad]->ucMenuSensitivity;
            break;

        case eGameSetting_DisplaySplitscreenGamertags:
            return ((GameSettingsA[iPad]->usBitmaskValues & 0x0200) >> 9);
            break;

        case eGameSetting_Hints:
            return ((GameSettingsA[iPad]->usBitmaskValues & 0x0400) >> 10);
            break;
        case eGameSetting_Autosave: {
            unsigned char ucVal =
                (GameSettingsA[iPad]->usBitmaskValues & 0x7800) >> 11;
            return ucVal;
        } break;
        case eGameSetting_Tooltips:
            return ((GameSettingsA[iPad]->usBitmaskValues & 0x8000) >> 15);
            break;

        case eGameSetting_InterfaceOpacity:
            return GameSettingsA[iPad]->ucInterfaceOpacity;
            break;

        case eGameSetting_Clouds:
            return (GameSettingsA[iPad]->uiBitmaskValues & GAMESETTING_CLOUDS);
            break;
        case eGameSetting_Online:
            return (GameSettingsA[iPad]->uiBitmaskValues &
                    GAMESETTING_ONLINE) >>
                   1;
            break;
        case eGameSetting_InviteOnly:
            return (GameSettingsA[iPad]->uiBitmaskValues &
                    GAMESETTING_INVITEONLY) >>
                   2;
            break;
        case eGameSetting_FriendsOfFriends:
            return (GameSettingsA[iPad]->uiBitmaskValues &
                    GAMESETTING_FRIENDSOFFRIENDS) >>
                   3;
            break;
        case eGameSetting_DisplayUpdateMessage:
            return (GameSettingsA[iPad]->uiBitmaskValues &
                    GAMESETTING_DISPLAYUPDATEMSG) >>
                   4;
            break;
        case eGameSetting_BedrockFog:
            return (GameSettingsA[iPad]->uiBitmaskValues &
                    GAMESETTING_BEDROCKFOG) >>
                   6;
            break;
        case eGameSetting_DisplayHUD:
            return (GameSettingsA[iPad]->uiBitmaskValues &
                    GAMESETTING_DISPLAYHUD) >>
                   7;
            break;
        case eGameSetting_DisplayHand:
            return (GameSettingsA[iPad]->uiBitmaskValues &
                    GAMESETTING_DISPLAYHAND) >>
                   8;
            break;
        case eGameSetting_CustomSkinAnim:
            return (GameSettingsA[iPad]->uiBitmaskValues &
                    GAMESETTING_CUSTOMSKINANIM) >>
                   9;
            break;
            // TU9
        case eGameSetting_DeathMessages:
            return (GameSettingsA[iPad]->uiBitmaskValues &
                    GAMESETTING_DEATHMESSAGES) >>
                   10;
            break;
        case eGameSetting_UISize: {
            unsigned char ucVal =
                (GameSettingsA[iPad]->uiBitmaskValues & GAMESETTING_UISIZE) >>
                11;
            return ucVal;
        } break;
        case eGameSetting_UISizeSplitscreen: {
            unsigned char ucVal = (GameSettingsA[iPad]->uiBitmaskValues &
                                   GAMESETTING_UISIZE_SPLITSCREEN) >>
                                  13;
            return ucVal;
        } break;
        case eGameSetting_AnimatedCharacter:
            return (GameSettingsA[iPad]->uiBitmaskValues &
                    GAMESETTING_ANIMATEDCHARACTER) >>
                   15;

        case eGameSetting_PS3_EULA_Read:
            return (GameSettingsA[iPad]->uiBitmaskValues &
                    GAMESETTING_PS3EULAREAD) >>
                   16;

        case eGameSetting_PSVita_NetworkModeAdhoc:
            return (GameSettingsA[iPad]->uiBitmaskValues &
                    GAMESETTING_PSVITANETWORKMODEADHOC) >>
                   17;
    }
    return 0;
}

void CMinecraftApp::CheckGameSettingsChanged(bool bOverride5MinuteTimer,
                                             int iPad) {
    // If the settings have changed, write them to the profile

    if (iPad == XUSER_INDEX_ANY) {
        for (int i = 0; i < XUSER_MAX_COUNT; i++) {
            if (GameSettingsA[i]->bSettingsChanged) {
                ProfileManager.WriteToProfile(i, true, bOverride5MinuteTimer);
                GameSettingsA[i]->bSettingsChanged = false;
            }
        }
    } else {
        if (GameSettingsA[iPad]->bSettingsChanged) {
            ProfileManager.WriteToProfile(iPad, true, bOverride5MinuteTimer);
            GameSettingsA[iPad]->bSettingsChanged = false;
        }
    }
}

void CMinecraftApp::ClearGameSettingsChangedFlag(int iPad) {
    GameSettingsA[iPad]->bSettingsChanged = false;
}

///////////////////////////
//
// Remove the debug settings in the content package build
//
////////////////////////////
#if !defined(_DEBUG_MENUS_ENABLED)
unsigned int CMinecraftApp::GetGameSettingsDebugMask(
    int iPad, bool bOverridePlayer)  // bOverridePlayer is to force the send for
                                     // the server to get the read options
{
    return 0;
}

void CMinecraftApp::SetGameSettingsDebugMask(int iPad, unsigned int uiVal) {}

void CMinecraftApp::ActionDebugMask(int iPad, bool bSetAllClear) {}

#else

unsigned int CMinecraftApp::GetGameSettingsDebugMask(
    int iPad, bool bOverridePlayer)  // bOverridePlayer is to force the send for
                                     // the server to get the read options
{
    if (iPad == -1) {
        iPad = ProfileManager.GetPrimaryPad();
    }
    if (iPad < 0) iPad = 0;

    std::shared_ptr<Player> player =
        Minecraft::GetInstance()->localplayers[iPad];

    if (bOverridePlayer || player == nullptr) {
        return GameSettingsA[iPad]->uiDebugBitmask;
    } else {
        return player->GetDebugOptions();
    }
}

void CMinecraftApp::SetGameSettingsDebugMask(int iPad, unsigned int uiVal) {
#if !defined(_CONTENT_PACKAGE)
    GameSettingsA[iPad]->bSettingsChanged = true;
    GameSettingsA[iPad]->uiDebugBitmask = uiVal;

    // update the value so the network server can use it
    std::shared_ptr<Player> player =
        Minecraft::GetInstance()->localplayers[iPad];

    if (player) {
        Minecraft::GetInstance()->localgameModes[iPad]->handleDebugOptions(
            uiVal, player);
    }
#endif
}

void CMinecraftApp::ActionDebugMask(int iPad, bool bSetAllClear) {
    unsigned int ulBitmask = app.GetGameSettingsDebugMask(iPad);

    if (bSetAllClear) ulBitmask = 0L;

    // these settings should only be actioned for the primary player
    if (ProfileManager.GetPrimaryPad() != iPad) return;

    for (int i = 0; i < eDebugSetting_Max; i++) {
        switch (i) {
            case eDebugSetting_LoadSavesFromDisk:
                if (ulBitmask & (1 << i)) {
                    app.SetLoadSavesFromFolderEnabled(true);
                } else {
                    app.SetLoadSavesFromFolderEnabled(false);
                }
                break;

            case eDebugSetting_WriteSavesToDisk:
                if (ulBitmask & (1 << i)) {
                    app.SetWriteSavesToFolderEnabled(true);
                } else {
                    app.SetWriteSavesToFolderEnabled(false);
                }
                break;

            case eDebugSetting_FreezePlayers:  // eDebugSetting_InterfaceOff:
                if (ulBitmask & (1 << i)) {
                    app.SetFreezePlayers(true);

                    // Turn off interface rendering.
                    // app.SetInterfaceRenderingOff( true );
                } else {
                    app.SetFreezePlayers(false);

                    // Turn on interface rendering.
                    // app.SetInterfaceRenderingOff( false );
                }
                break;
            case eDebugSetting_Safearea:
                if (ulBitmask & (1 << i)) {
                    app.ShowSafeArea(true);
                } else {
                    app.ShowSafeArea(false);
                }
                break;

                // case eDebugSetting_HandRenderingOff:
                //	if(ulBitmask&(1<<i))
                //	{
                //		// Turn off hand rendering.
                //		//app.SetHandRenderingOff( true );
                //	}
                //	else
                //	{
                //		// Turn on hand rendering.
                //		//app.SetHandRenderingOff( false );
                //	}
                //	break;

            case eDebugSetting_ShowUIConsole:
                if (ulBitmask & (1 << i)) {
                    ui.ShowUIDebugConsole(true);
                } else {
                    ui.ShowUIDebugConsole(false);
                }
                break;

            case eDebugSetting_ShowUIMarketingGuide:
                if (ulBitmask & (1 << i)) {
                    ui.ShowUIDebugMarketingGuide(true);
                } else {
                    ui.ShowUIDebugMarketingGuide(false);
                }
                break;

            case eDebugSetting_MobsDontAttack:
                if (ulBitmask & (1 << i)) {
                    app.SetMobsDontAttackEnabled(true);
                } else {
                    app.SetMobsDontAttackEnabled(false);
                }
                break;

            case eDebugSetting_UseDpadForDebug:
                if (ulBitmask & (1 << i)) {
                    app.SetUseDPadForDebug(true);
                } else {
                    app.SetUseDPadForDebug(false);
                }
                break;
            case eDebugSetting_MobsDontTick:
                if (ulBitmask & (1 << i)) {
                    app.SetMobsDontTickEnabled(true);
                } else {
                    app.SetMobsDontTickEnabled(false);
                }
                break;
        }
    }
}
#endif

int CMinecraftApp::DisplaySavingMessage(void* pParam,
                                        C4JStorage::ESavingMessage eVal,
                                        int iPad) {
    // CMinecraftApp* pClass = (CMinecraftApp*)pParam;

    ui.ShowSavingMessage(iPad, eVal);

    return 0;
}

void CMinecraftApp::SetActionConfirmed(void* param) {
    XuiActionParam* actionInfo = (XuiActionParam*)param;
    app.SetAction(actionInfo->iPad, actionInfo->action);
}

void CMinecraftApp::HandleXuiActions(void) {
    eXuiAction eAction;
    eTMSAction eTMS;
    void* param;
    Minecraft* pMinecraft = Minecraft::GetInstance();
    std::shared_ptr<MultiplayerLocalPlayer> player;

    // are there any global actions to deal with?
    eAction = app.GetGlobalXuiAction();
    if (eAction != eAppAction_Idle) {
        switch (eAction) {
            case eAppAction_DisplayLavaMessage:
                // Display a warning about placing lava in the spawn area
                {
                    unsigned int uiIDA[1];
                    uiIDA[0] = IDS_CONFIRM_OK;
                    C4JStorage::EMessageResult result =
                        ui.RequestErrorMessage(IDS_CANT_PLACE_NEAR_SPAWN_TITLE,
                                               IDS_CANT_PLACE_NEAR_SPAWN_TEXT,
                                               uiIDA, 1, XUSER_INDEX_ANY);
                    if (result != C4JStorage::EMessage_Busy)
                        SetGlobalXuiAction(eAppAction_Idle);
                }
                break;
            default:
                break;
        }
    }

    // are there any app actions to deal with?
    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
        eAction = app.GetXuiAction(i);
        param = m_eXuiActionParam[i];

        if (eAction != eAppAction_Idle) {
            switch (eAction) {
                    // the renderer will capture a screenshot
                case eAppAction_SocialPost:
                    if (ProfileManager.IsFullVersion()) {
                        // Facebook Share
                        if (CSocialManager::Instance()
                                ->IsTitleAllowedToPostImages() &&
                            CSocialManager::Instance()
                                ->AreAllUsersAllowedToPostImages()) {
                            // disable character name tags for the shot
                            // m_bwasHidingGui = pMinecraft->options->hideGui;
                            // // 4J Stu - Removed 1.8.2 bug fix (TU6) as don't
                            // need this
                            pMinecraft->options->hideGui = true;

                            SetAction(i, eAppAction_SocialPostScreenshot);
                        } else {
                            SetAction(i, eAppAction_Idle);
                        }
                    } else {
                        SetAction(i, eAppAction_Idle);
                    }
                    break;
                case eAppAction_SocialPostScreenshot: {
                    SetAction(i, eAppAction_Idle);
                    bool bKeepHiding = false;
                    for (int j = 0; j < XUSER_MAX_COUNT; ++j) {
                        if (app.GetXuiAction(j) ==
                            eAppAction_SocialPostScreenshot) {
                            bKeepHiding = true;
                            break;
                        }
                    }
                    pMinecraft->options->hideGui = bKeepHiding;

                    // Facebook Share

                    if (app.GetLocalPlayerCount() > 1) {
                        ui.NavigateToScene(i, eUIScene_SocialPost);
                    } else {
                        ui.NavigateToScene(i, eUIScene_SocialPost);
                    }
                } break;
                case eAppAction_SaveGame:
                    SetAction(i, eAppAction_Idle);
                    if (!GetChangingSessionType()) {
                        // If this is the trial game, do an upsell
                        if (ProfileManager.IsFullVersion()) {
                            // flag the render to capture the screenshot for the
                            // save
                            SetAction(i, eAppAction_SaveGameCapturedThumbnail);
                        } else {
                            // ask the player if they would like to upgrade, or
                            // they'll lose the level

                            unsigned int uiIDA[2];
                            uiIDA[0] = IDS_CONFIRM_OK;
                            uiIDA[1] = IDS_CONFIRM_CANCEL;
                            ui.RequestErrorMessage(
                                IDS_UNLOCK_TITLE, IDS_UNLOCK_TOSAVE_TEXT, uiIDA,
                                2, i, &CMinecraftApp::UnlockFullSaveReturned,
                                this);
                        }
                    }

                    break;
                case eAppAction_AutosaveSaveGame: {
                    // Need to run a check to see if the save exists in order to
                    // stop the dialog asking if we want to overwrite it coming
                    // up on an autosave
                    bool bSaveExists;
                    StorageManager.DoesSaveExist(&bSaveExists);

                    SetAction(i, eAppAction_Idle);
                    if (!GetChangingSessionType()) {
                        // flag the render to capture the screenshot for the
                        // save
                        SetAction(i,
                                  eAppAction_AutosaveSaveGameCapturedThumbnail);
                    }
                }

                break;

                case eAppAction_SaveGameCapturedThumbnail:
                    // reset the autosave timer
                    app.SetAutosaveTimerTime();
                    SetAction(i, eAppAction_Idle);
                    // Check that there is a name for the save - if we're saving
                    // from the tutorial and this is the first save from the
                    // tutorial, we'll not have a name
                    /*if(StorageManager.GetSaveName()==nullptr)
                    {
                    app.NavigateToScene(i,eUIScene_SaveWorld);
                    }
                    else*/
                    {
                        // turn off the gamertags in splitscreen for the primary
                        // player, since they are about to be made fullscreen
                        ui.HideAllGameUIElements();

                        // Hide the other players scenes
                        ui.ShowOtherPlayersBaseScene(
                            ProfileManager.GetPrimaryPad(), false);

                        // int saveOrCheckpointId = 0;
                        // bool validSave =
                        // StorageManager.GetSaveUniqueNumber(&saveOrCheckpointId);
                        // SentientManager.RecordLevelSaveOrCheckpoint(ProfileManager.GetPrimaryPad(),
                        // saveOrCheckpointId);

                        LoadingInputParams* loadingParams =
                            new LoadingInputParams();
                        loadingParams->func =
                            &UIScene_PauseMenu::SaveWorldThreadProc;
                        loadingParams->lpParam = (void*)false;

                        // 4J-JEV - PS4: Fix for #5708 - [ONLINE] - If the user
                        // pulls their network cable out while saving the title
                        // will hang.
                        loadingParams->waitForThreadToDelete = true;

                        UIFullscreenProgressCompletionData* completionData =
                            new UIFullscreenProgressCompletionData();
                        completionData->bShowBackground = true;
                        completionData->bShowLogo = true;
                        completionData->type =
                            e_ProgressCompletion_NavigateBackToScene;
                        completionData->iPad = ProfileManager.GetPrimaryPad();

                        if (ui.IsSceneInStack(ProfileManager.GetPrimaryPad(),
                                              eUIScene_EndPoem)) {
                            completionData->scene = eUIScene_EndPoem;
                        } else {
                            completionData->scene = eUIScene_PauseMenu;
                        }

                        loadingParams->completionData = completionData;

                        // 4J Stu - Xbox only

                        ui.NavigateToScene(ProfileManager.GetPrimaryPad(),
                                           eUIScene_FullscreenProgress,
                                           loadingParams, eUILayer_Fullscreen,
                                           eUIGroup_Fullscreen);
                    }
                    break;
                case eAppAction_AutosaveSaveGameCapturedThumbnail:

                {
                    app.SetAutosaveTimerTime();
                    SetAction(i, eAppAction_Idle);

                    // turn off the gamertags in splitscreen for the primary
                    // player, since they are about to be made fullscreen
                    ui.HideAllGameUIElements();

                    // app.CloseAllPlayersXuiScenes();
                    //  Hide the other players scenes
                    ui.ShowOtherPlayersBaseScene(ProfileManager.GetPrimaryPad(),
                                                 false);

                    // This just allows it to be shown
                    if (pMinecraft
                            ->localgameModes[ProfileManager.GetPrimaryPad()] !=
                        nullptr)
                        pMinecraft
                            ->localgameModes[ProfileManager.GetPrimaryPad()]
                            ->getTutorial()
                            ->showTutorialPopup(false);

                    // int saveOrCheckpointId = 0;
                    // bool validSave =
                    // StorageManager.GetSaveUniqueNumber(&saveOrCheckpointId);
                    // SentientManager.RecordLevelSaveOrCheckpoint(ProfileManager.GetPrimaryPad(),
                    // saveOrCheckpointId);

                    LoadingInputParams* loadingParams =
                        new LoadingInputParams();
                    loadingParams->func =
                        &UIScene_PauseMenu::SaveWorldThreadProc;

                    loadingParams->lpParam = (void*)true;

                    UIFullscreenProgressCompletionData* completionData =
                        new UIFullscreenProgressCompletionData();
                    completionData->bShowBackground = true;
                    completionData->bShowLogo = true;
                    completionData->type =
                        e_ProgressCompletion_AutosaveNavigateBack;
                    completionData->iPad = ProfileManager.GetPrimaryPad();
                    // completionData->bAutosaveWasMenuDisplayed=ui.GetMenuDisplayed(ProfileManager.GetPrimaryPad());
                    loadingParams->completionData = completionData;

                    // 4J Stu - Xbox only

                    ui.NavigateToScene(ProfileManager.GetPrimaryPad(),
                                       eUIScene_FullscreenProgress,
                                       loadingParams, eUILayer_Fullscreen,
                                       eUIGroup_Fullscreen);
                } break;
                case eAppAction_ExitPlayer:
                    // a secondary player has chosen to quit
                    {
                        int iPlayerC = g_NetworkManager.GetPlayerCount();

                        // Since the player is exiting, let's flush any profile
                        // writes for them, and hope we're not breaking TCR
                        // 136...
                        ProfileManager.ForceQueuedProfileWrites(i);

                        // not required - it's done within the
                        // removeLocalPlayerIdx
                        // 				if(pMinecraft->level->isClientSide)
                        // 				{
                        // 					// we need to
                        // remove the qnetplayer, or this player won't be able
                        // to get back into the game until qnet times out and
                        // removes them
                        // 					g_NetworkManager.NotifyPlayerLeaving(g_NetworkManager.GetLocalPlayerByUserIndex(i));
                        // 				}

                        // if there are any tips showing, we need to close them

                        pMinecraft->gui->clearMessages(i);

                        // Make sure we've not got this player selected as
                        // current - this shouldn't be the case anyway
                        pMinecraft->setLocalPlayerIdx(
                            ProfileManager.GetPrimaryPad());
                        pMinecraft->removeLocalPlayerIdx(i);

                        // Wipe out the tooltips
                        ui.SetTooltips(i, -1);

                        // Change the presence info
                        // Are we offline or online, and how many players are
                        // there
                        if (iPlayerC > 2)  // one player is about to leave here
                                           // - they'll be set to idle in the
                                           // qnet manager player leave
                        {
                            for (int iPlayer = 0; iPlayer < XUSER_MAX_COUNT;
                                 iPlayer++) {
                                if ((iPlayer != i) &&
                                    pMinecraft->localplayers[iPlayer]) {
                                    if (g_NetworkManager.IsLocalGame()) {
                                        ProfileManager.SetCurrentGameActivity(
                                            iPlayer,
                                            CONTEXT_PRESENCE_MULTIPLAYEROFFLINE,
                                            false);
                                    } else {
                                        ProfileManager.SetCurrentGameActivity(
                                            iPlayer,
                                            CONTEXT_PRESENCE_MULTIPLAYER,
                                            false);
                                    }
                                }
                            }
                        } else {
                            for (int iPlayer = 0; iPlayer < XUSER_MAX_COUNT;
                                 iPlayer++) {
                                if ((iPlayer != i) &&
                                    pMinecraft->localplayers[iPlayer]) {
                                    if (g_NetworkManager.IsLocalGame()) {
                                        ProfileManager.SetCurrentGameActivity(
                                            iPlayer,
                                            CONTEXT_PRESENCE_MULTIPLAYER_1POFFLINE,
                                            false);
                                    } else {
                                        ProfileManager.SetCurrentGameActivity(
                                            iPlayer,
                                            CONTEXT_PRESENCE_MULTIPLAYER_1P,
                                            false);
                                    }
                                }
                            }
                        }

                        SetAction(i, eAppAction_Idle);
                    }
                    break;
                case eAppAction_ExitPlayerPreLogin: {
                    int iPlayerC = g_NetworkManager.GetPlayerCount();
                    // Since the player is exiting, let's flush any profile
                    // writes for them, and hope we're not breaking TCR 136...
                    ProfileManager.ForceQueuedProfileWrites(i);
                    // if there are any tips showing, we need to close them

                    pMinecraft->gui->clearMessages(i);

                    // Make sure we've not got this player selected as current -
                    // this shouldn't be the case anyway
                    pMinecraft->setLocalPlayerIdx(
                        ProfileManager.GetPrimaryPad());
                    pMinecraft->removeLocalPlayerIdx(i);

                    // Wipe out the tooltips
                    ui.SetTooltips(i, -1);

                    // Change the presence info
                    // Are we offline or online, and how many players are there
                    if (iPlayerC >
                        2)  // one player is about to leave here - they'll be
                            // set to idle in the qnet manager player leave
                    {
                        for (int iPlayer = 0; iPlayer < XUSER_MAX_COUNT;
                             iPlayer++) {
                            if ((iPlayer != i) &&
                                pMinecraft->localplayers[iPlayer]) {
                                if (g_NetworkManager.IsLocalGame()) {
                                    ProfileManager.SetCurrentGameActivity(
                                        iPlayer,
                                        CONTEXT_PRESENCE_MULTIPLAYEROFFLINE,
                                        false);
                                } else {
                                    ProfileManager.SetCurrentGameActivity(
                                        iPlayer, CONTEXT_PRESENCE_MULTIPLAYER,
                                        false);
                                }
                            }
                        }
                    } else {
                        for (int iPlayer = 0; iPlayer < XUSER_MAX_COUNT;
                             iPlayer++) {
                            if ((iPlayer != i) &&
                                pMinecraft->localplayers[iPlayer]) {
                                if (g_NetworkManager.IsLocalGame()) {
                                    ProfileManager.SetCurrentGameActivity(
                                        iPlayer,
                                        CONTEXT_PRESENCE_MULTIPLAYER_1POFFLINE,
                                        false);
                                } else {
                                    ProfileManager.SetCurrentGameActivity(
                                        iPlayer,
                                        CONTEXT_PRESENCE_MULTIPLAYER_1P, false);
                                }
                            }
                        }
                    }
                    SetAction(i, eAppAction_Idle);
                } break;

                case eAppAction_ExitWorld:
                    pMinecraft->exitingWorldRightNow = true;

                    SetAction(i, eAppAction_Idle);

                    // If we're already leaving don't exit
                    if (g_NetworkManager.IsLeavingGame()) {
                        break;
                    }

                    pMinecraft->gui->clearMessages();

                    // turn off the gamertags in splitscreen for the primary
                    // player, since they are about to be made fullscreen
                    ui.HideAllGameUIElements();

                    // reset the flag stopping new dlc message being shown if
                    // you've seen the message before
                    DisplayNewDLCTipAgain();

                    // clear the autosave timer that might be on screen
                    ui.ShowAutosaveCountdownTimer(false);

                    // Hide the selected item text
                    ui.HideAllGameUIElements();

                    // Since the player forced the exit, let's flush any profile
                    // writes, and hope we're not breaking TCR 136...

                    // 4J-PB - cancel any possible std::string verifications
                    // queued with LIVE
                    // InputManager.CancelAllVerifyInProgress();

                    if (ProfileManager.IsFullVersion()) {
                        // In a split screen, only the primary player actually
                        // quits the game, others just remove their players
                        if (i != ProfileManager.GetPrimaryPad()) {
                            // Make sure we've not got this player selected as
                            // current - this shouldn't be the case anyway
                            pMinecraft->setLocalPlayerIdx(
                                ProfileManager.GetPrimaryPad());
                            pMinecraft->removeLocalPlayerIdx(i);

                            SetAction(i, eAppAction_Idle);
                            return;
                        }
                        // flag to capture the save thumbnail
                        SetAction(i, eAppAction_ExitWorldCapturedThumbnail,
                                  param);
                    } else {
                        // ask the player if they would like to upgrade, or
                        // they'll lose the level
                        unsigned int uiIDA[2];
                        uiIDA[0] = IDS_CONFIRM_OK;
                        uiIDA[1] = IDS_CONFIRM_CANCEL;
                        ui.RequestErrorMessage(
                            IDS_UNLOCK_TITLE, IDS_UNLOCK_TOSAVE_TEXT, uiIDA, 2,
                            i, &CMinecraftApp::UnlockFullExitReturned, this);
                    }

                    // Change the presence info
                    // Are we offline or online, and how many players are there

                    if (g_NetworkManager.GetPlayerCount() > 1) {
                        for (int j = 0; j < XUSER_MAX_COUNT; j++) {
                            if (pMinecraft->localplayers[j]) {
                                if (g_NetworkManager.IsLocalGame()) {
                                    app.SetRichPresenceContext(
                                        j, CONTEXT_GAME_STATE_BLANK);
                                    ProfileManager.SetCurrentGameActivity(
                                        j, CONTEXT_PRESENCE_MULTIPLAYEROFFLINE,
                                        false);
                                } else {
                                    app.SetRichPresenceContext(
                                        j, CONTEXT_GAME_STATE_BLANK);
                                    ProfileManager.SetCurrentGameActivity(
                                        j, CONTEXT_PRESENCE_MULTIPLAYER, false);
                                }
                                TelemetryManager->RecordLevelExit(
                                    j, eSen_LevelExitStatus_Exited);
                            }
                        }
                    } else {
                        app.SetRichPresenceContext(i, CONTEXT_GAME_STATE_BLANK);
                        if (g_NetworkManager.IsLocalGame()) {
                            ProfileManager.SetCurrentGameActivity(
                                i, CONTEXT_PRESENCE_MULTIPLAYER_1POFFLINE,
                                false);
                        } else {
                            ProfileManager.SetCurrentGameActivity(
                                i, CONTEXT_PRESENCE_MULTIPLAYER_1P, false);
                        }
                        TelemetryManager->RecordLevelExit(
                            i, eSen_LevelExitStatus_Exited);
                    }
                    break;
                case eAppAction_ExitWorldCapturedThumbnail: {
                    SetAction(i, eAppAction_Idle);
                    // Stop app running
                    SetGameStarted(false);
                    SetChangingSessionType(
                        true);  // Added to stop handling ethernet disconnects

                    ui.CloseAllPlayersScenes();

                    // turn off the gamertags in splitscreen for the primary
                    // player, since they are about to be made fullscreen
                    ui.HideAllGameUIElements();

                    // 4J Stu - Fix for #12368 - Crash: Game crashes when saving
                    // then exiting and selecting to save
                    for (unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx) {
                        // 4J Stu - Fix for #13257 - CRASH: Gameplay: Title
                        // crashed after exiting the tutorial It doesn't matter
                        // if they were in the tutorial already
                        pMinecraft->playerLeftTutorial(idx);
                    }

                    LoadingInputParams* loadingParams =
                        new LoadingInputParams();
                    loadingParams->func =
                        &UIScene_PauseMenu::ExitWorldThreadProc;
                    loadingParams->lpParam = param;

                    UIFullscreenProgressCompletionData* completionData =
                        new UIFullscreenProgressCompletionData();
                    // If param is non-null then this is a forced exit by the
                    // server, so make sure the player knows why 4J Stu -
                    // Changed - Don't use the FullScreenProgressScreen for
                    // action, use a dialog instead
                    completionData->bRequiresUserAction =
                        false;  //(param != nullptr) ? true : false;
                    completionData->bShowTips =
                        (param != nullptr) ? false : true;
                    completionData->bShowBackground = true;
                    completionData->bShowLogo = true;
                    completionData->type =
                        e_ProgressCompletion_NavigateToHomeMenu;
                    completionData->iPad = DEFAULT_XUI_MENU_USER;
                    loadingParams->completionData = completionData;

                    ui.NavigateToScene(ProfileManager.GetPrimaryPad(),
                                       eUIScene_FullscreenProgress,
                                       loadingParams);
                } break;
                case eAppAction_ExitWorldTrial: {
                    SetAction(i, eAppAction_Idle);

                    pMinecraft->gui->clearMessages();

                    // turn off the gamertags in splitscreen for the primary
                    // player, since they are about to be made fullscreen
                    ui.HideAllGameUIElements();

                    // Stop app running
                    SetGameStarted(false);

                    ui.CloseAllPlayersScenes();

                    // 4J Stu - Fix for #12368 - Crash: Game crashes when saving
                    // then exiting and selecting to save
                    for (unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx) {
                        // 4J Stu - Fix for #13257 - CRASH: Gameplay: Title
                        // crashed after exiting the tutorial It doesn't matter
                        // if they were in the tutorial already
                        pMinecraft->playerLeftTutorial(idx);
                    }

                    LoadingInputParams* loadingParams =
                        new LoadingInputParams();
                    loadingParams->func =
                        &UIScene_PauseMenu::ExitWorldThreadProc;
                    loadingParams->lpParam = param;

                    UIFullscreenProgressCompletionData* completionData =
                        new UIFullscreenProgressCompletionData();
                    completionData->bShowBackground = true;
                    completionData->bShowLogo = true;
                    completionData->type =
                        e_ProgressCompletion_NavigateToHomeMenu;
                    completionData->iPad = DEFAULT_XUI_MENU_USER;
                    loadingParams->completionData = completionData;

                    ui.NavigateToScene(ProfileManager.GetPrimaryPad(),
                                       eUIScene_FullscreenProgress,
                                       loadingParams);
                }

                break;
                case eAppAction_ExitTrial:
                    // XLaunchNewImage(XLAUNCH_KEYWORD_DASH_ARCADE, 0);
                    ExitGame();
                    break;

                case eAppAction_Respawn: {
                    ConnectionProgressParams* param =
                        new ConnectionProgressParams();
                    param->iPad = i;
                    param->stringId = IDS_PROGRESS_RESPAWNING;
                    param->showTooltips = false;
                    param->setFailTimer = false;
                    ui.NavigateToScene(i, eUIScene_ConnectingProgress, param);

                    // Need to reset this incase the player has already died and
                    // respawned
                    pMinecraft->localplayers[i]->SetPlayerRespawned(false);

                    SetAction(i, eAppAction_WaitForRespawnComplete);
                    if (app.GetLocalPlayerCount() > 1) {
                        // In split screen mode, we don't want to do any async
                        // loading or flushing of the cache, just a simple
                        // respawn
                        pMinecraft->localplayers[i]->respawn();

                        // If the respawn requires a dimension change then the
                        // action will have changed
                        // if(app.GetXuiAction(i) == eAppAction_Respawn)
                        //{
                        //	SetAction(i,eAppAction_Idle);
                        //	CloseXuiScenes(i);
                        //}
                    } else {
                        // SetAction(i,eAppAction_WaitForRespawnComplete);

                        // LoadingInputParams *loadingParams = new
                        // LoadingInputParams(); loadingParams->func =
                        // &CScene_Death::RespawnThreadProc;
                        // loadingParams->lpParam = (void*)i;

                        // Disable game & update thread whilst we do any of this
                        // app.SetGameStarted(false);
                        pMinecraft->gameRenderer->DisableUpdateThread();

                        // 4J Stu - We don't need this on a thread in
                        // multiplayer as respawning is asynchronous.
                        pMinecraft->localplayers[i]->respawn();

                        // app.SetGameStarted(true);
                        pMinecraft->gameRenderer->EnableUpdateThread();

                        // UIFullscreenProgressCompletionData *completionData =
                        // new UIFullscreenProgressCompletionData();
                        // completionData->bShowBackground=true;
                        // completionData->bShowLogo=true;
                        // completionData->type =
                        // e_ProgressCompletion_CloseUIScenes;
                        // completionData->iPad = i;
                        // loadingParams->completionData = completionData;

                        // app.NavigateToScene(i,eUIScene_FullscreenProgress,
                        // loadingParams, true);
                    }
                } break;
                case eAppAction_WaitForRespawnComplete:
                    player = pMinecraft->localplayers[i];
                    if (player != nullptr && player->GetPlayerRespawned()) {
                        SetAction(i, eAppAction_Idle);

                        if (ui.IsSceneInStack(i, eUIScene_EndPoem)) {
                            ui.NavigateBack(i, false, eUIScene_EndPoem);
                        } else {
                            ui.CloseUIScenes(i);
                        }

                        // clear the progress messages

                        // 					pMinecraft->progressRenderer->progressStart(-1);
                        // 					pMinecraft->progressRenderer->progressStage(-1);
                    } else if (!g_NetworkManager.IsInGameplay()) {
                        SetAction(i, eAppAction_Idle);
                    }
                    break;
                case eAppAction_WaitForDimensionChangeComplete:
                    player = pMinecraft->localplayers[i];
                    if (player != nullptr && player->connection &&
                        player->connection->isStarted()) {
                        SetAction(i, eAppAction_Idle);
                        ui.CloseUIScenes(i);
                    } else if (!g_NetworkManager.IsInGameplay()) {
                        SetAction(i, eAppAction_Idle);
                    }
                    break;
                case eAppAction_PrimaryPlayerSignedOut: {
                    // SetAction(i,eAppAction_Idle);

                    // clear the autosavetimer that might be displayed
                    ui.ShowAutosaveCountdownTimer(false);

                    // If the player signs out before the game started the
                    // server can be killed a bit earlier to stop the loading or
                    // saving of a new game continuing running while the
                    // UI/Guide is up
                    if (!app.GetGameStarted())
                        MinecraftServer::HaltServer(true);

                    // inform the player they are being returned to the menus
                    // because they signed out
                    StorageManager.SetSaveDeviceSelected(i, false);
                    // need to clear the player stats - can't assume it'll be
                    // done in setlevel - we may not be in the game
                    StatsCounter* pStats = Minecraft::GetInstance()->stats[i];
                    pStats->clear();

                    // 4J-PB - the libs will display the Returned to Title
                    // screen 					unsigned int
                    // uiIDA[1]; uiIDA[0]=IDS_CONFIRM_OK;
                    //
                    // 					ui.RequestMessageBox(IDS_RETURNEDTOMENU_TITLE,
                    // IDS_RETURNEDTOTITLESCREEN_TEXT, uiIDA, 1,
                    // i,&CMinecraftApp::PrimaryPlayerSignedOutReturned,this,app.GetStringTable());
                    if (g_NetworkManager.IsInSession()) {
                        app.SetAction(
                            i, eAppAction_PrimaryPlayerSignedOutReturned);
                    } else {
                        app.SetAction(
                            i, eAppAction_PrimaryPlayerSignedOutReturned_Menus);
                        MinecraftServer::resetFlags();
                    }
                } break;
                case eAppAction_EthernetDisconnected: {
                    app.DebugPrintf(
                        "Handling eAppAction_EthernetDisconnected\n");
                    SetAction(i, eAppAction_Idle);

                    // 4J Stu - Fix for #12530 -TCR 001 BAS Game Stability:
                    // Title will crash if the player disconnects while starting
                    // a new world and then opts to play the tutorial once they
                    // have been returned to the Main Menu.
                    if (!g_NetworkManager.IsLeavingGame()) {
                        app.DebugPrintf(
                            "Handling eAppAction_EthernetDisconnected - Not "
                            "leaving game\n");
                        // 4J-PB - not the same as a signout. We should only
                        // leave the game if this machine is not the host. We
                        // shouldn't get rid of the save device either.
                        if (g_NetworkManager.IsHost()) {
                            app.DebugPrintf(
                                "Handling eAppAction_EthernetDisconnected - Is "
                                "Host\n");
                            // If it's already a local game, then an ethernet
                            // disconnect should have no effect
                            if (!g_NetworkManager.IsLocalGame() &&
                                g_NetworkManager.IsInGameplay()) {
                                // Change the session to an offline session
                                SetAction(i, eAppAction_ChangeSessionType);
                            } else if (!g_NetworkManager.IsLocalGame() &&
                                       !g_NetworkManager.IsInGameplay()) {
                                // There are two cases here, either:
                                //	 1. We're early enough in the
                                // create/load game that we can do a really
                                // minimal shutdown or
                                //   2. We're far enough in (game has started
                                //   but the actual game started flag hasn't
                                //   been set) that we should just wait until
                                //   we're in the game and switch to offline
                                //   mode

                                // If there's a non-null level then, for our
                                // purposes, the game has started
                                bool gameStarted = false;
                                for (int j = 0; j < pMinecraft->levels.length;
                                     j++) {
                                    if (pMinecraft->levels.data[j] != nullptr) {
                                        gameStarted = true;
                                        break;
                                    }
                                }

                                if (!gameStarted) {
                                    // 1. Exit
                                    MinecraftServer::HaltServer();

                                    // Fix for #12530 - TCR 001 BAS Game
                                    // Stability: Title will crash if the player
                                    // disconnects while starting a new world
                                    // and then opts to play the tutorial once
                                    // they have been returned to the Main Menu.
                                    // 4J Stu - Leave the session
                                    g_NetworkManager.LeaveGame(false);

                                    // need to clear the player stats - can't
                                    // assume it'll be done in setlevel - we may
                                    // not be in the game
                                    StatsCounter* pStats =
                                        Minecraft::GetInstance()->stats[i];
                                    pStats->clear();
                                    unsigned int uiIDA[1];
                                    uiIDA[0] = IDS_CONFIRM_OK;

                                    ui.RequestErrorMessage(
                                        g_NetworkManager.CorrectErrorIDS(
                                            IDS_CONNECTION_LOST),
                                        g_NetworkManager.CorrectErrorIDS(
                                            IDS_CONNECTION_LOST_LIVE),
                                        uiIDA, 1, i,
                                        &CMinecraftApp::
                                            EthernetDisconnectReturned,
                                        this);
                                } else {
                                    // 2. Switch to offline
                                    SetAction(i, eAppAction_ChangeSessionType);
                                }
                            }
                        } else {
                            {
                                app.DebugPrintf(
                                    "Handling eAppAction_EthernetDisconnected "
                                    "- Not host\n");
                                // need to clear the player stats - can't assume
                                // it'll be done in setlevel - we may not be in
                                // the game
                                StatsCounter* pStats =
                                    Minecraft::GetInstance()->stats[i];
                                pStats->clear();
                                unsigned int uiIDA[1];
                                uiIDA[0] = IDS_CONFIRM_OK;

                                ui.RequestErrorMessage(
                                    g_NetworkManager.CorrectErrorIDS(
                                        IDS_CONNECTION_LOST),
                                    g_NetworkManager.CorrectErrorIDS(
                                        IDS_CONNECTION_LOST_LIVE),
                                    uiIDA, 1, i,
                                    &CMinecraftApp::EthernetDisconnectReturned,
                                    this);
                            }
                        }
                    }
                } break;
                    // We currently handle both these returns the same way.
                case eAppAction_EthernetDisconnectedReturned:
                case eAppAction_PrimaryPlayerSignedOutReturned: {
                    SetAction(i, eAppAction_Idle);

                    pMinecraft->gui->clearMessages();

                    // turn off the gamertags in splitscreen for the primary
                    // player, since they are about to be made fullscreen
                    ui.HideAllGameUIElements();

                    // set the state back to pre-game
                    ProfileManager.ResetProfileProcessState();

                    if (g_NetworkManager.IsLeavingGame()) {
                        // 4J Stu - If we are already leaving the game, then we
                        // just need to signal that the player signed out to
                        // stop saves
                        pMinecraft->progressRenderer->progressStartNoAbort(
                            IDS_EXITING_GAME);
                        pMinecraft->progressRenderer->progressStage(-1);
                        // This has no effect on client machines
                        MinecraftServer::HaltServer(true);
                    } else {
                        // Stop app running
                        SetGameStarted(false);

                        // turn off the gamertags in splitscreen for the primary
                        // player, since they are about to be made fullscreen
                        ui.HideAllGameUIElements();

                        ui.CloseAllPlayersScenes();

                        // 4J Stu - Fix for #12368 - Crash: Game crashes when
                        // saving then exiting and selecting to save
                        for (unsigned int idx = 0; idx < XUSER_MAX_COUNT;
                             ++idx) {
                            // 4J Stu - Fix for #13257 - CRASH: Gameplay: Title
                            // crashed after exiting the tutorial It doesn't
                            // matter if they were in the tutorial already
                            pMinecraft->playerLeftTutorial(idx);
                        }

                        LoadingInputParams* loadingParams =
                            new LoadingInputParams();
                        loadingParams->func =
                            &CMinecraftApp::SignoutExitWorldThreadProc;

                        UIFullscreenProgressCompletionData* completionData =
                            new UIFullscreenProgressCompletionData();
                        completionData->bShowBackground = true;
                        completionData->bShowLogo = true;
                        completionData->iPad = DEFAULT_XUI_MENU_USER;
                        completionData->type =
                            e_ProgressCompletion_NavigateToHomeMenu;
                        loadingParams->completionData = completionData;

                        ui.NavigateToScene(ProfileManager.GetPrimaryPad(),
                                           eUIScene_FullscreenProgress,
                                           loadingParams);
                    }
                } break;
                case eAppAction_PrimaryPlayerSignedOutReturned_Menus:
                    SetAction(i, eAppAction_Idle);
                    // set the state back to pre-game
                    ProfileManager.ResetProfileProcessState();
                    // clear the save device
                    StorageManager.SetSaveDeviceSelected(i, false);

                    ui.UpdatePlayerBasePositions();
                    // there are multiple layers in the help menu, so a navigate
                    // back isn't enough
                    ui.NavigateToHomeMenu();

                    break;
                case eAppAction_EthernetDisconnectedReturned_Menus:
                    SetAction(i, eAppAction_Idle);
                    // set the state back to pre-game
                    ProfileManager.ResetProfileProcessState();

                    ui.UpdatePlayerBasePositions();

                    // there are multiple layers in the help menu, so a navigate
                    // back isn't enough
                    ui.NavigateToHomeMenu();

                    break;

                case eAppAction_TrialOver: {
                    SetAction(i, eAppAction_Idle);
                    unsigned int uiIDA[2];
                    uiIDA[0] = IDS_UNLOCK_TITLE;
                    uiIDA[1] = IDS_EXIT_GAME;

                    ui.RequestErrorMessage(
                        IDS_TRIALOVER_TITLE, IDS_TRIALOVER_TEXT, uiIDA, 2, i,
                        &CMinecraftApp::TrialOverReturned, this);
                } break;

                    // INVITES
                case eAppAction_DashboardTrialJoinFromInvite: {
                    TelemetryManager->RecordUpsellPresented(
                        i, eSen_UpsellID_Full_Version_Of_Game, app.m_dwOfferID);

                    SetAction(i, eAppAction_Idle);
                    unsigned int uiIDA[2];
                    uiIDA[0] = IDS_CONFIRM_OK;
                    uiIDA[1] = IDS_CONFIRM_CANCEL;

                    ui.RequestErrorMessage(
                        IDS_UNLOCK_TITLE, IDS_UNLOCK_ACCEPT_INVITE, uiIDA, 2, i,
                        &CMinecraftApp::UnlockFullInviteReturned, this);
                } break;
                case eAppAction_ExitAndJoinFromInvite: {
                    unsigned int uiIDA[3];

                    SetAction(i, eAppAction_Idle);
                    // Check the player really wants to do this

                    if (ProfileManager.IsFullVersion() &&
                        !StorageManager.GetSaveDisabled() &&
                        i == ProfileManager.GetPrimaryPad() &&
                        g_NetworkManager.IsHost() && GetGameStarted()) {
                        uiIDA[0] = IDS_CONFIRM_CANCEL;
                        uiIDA[1] = IDS_EXIT_GAME_SAVE;
                        uiIDA[2] = IDS_EXIT_GAME_NO_SAVE;

                        ui.RequestAlertMessage(
                            IDS_EXIT_GAME, IDS_CONFIRM_LEAVE_VIA_INVITE, uiIDA,
                            3, i,
                            &CMinecraftApp::
                                ExitAndJoinFromInviteSaveDialogReturned,
                            this);
                    } else {
                        if (!ProfileManager.IsFullVersion()) {
                            TelemetryManager->RecordUpsellPresented(
                                i, eSen_UpsellID_Full_Version_Of_Game,
                                app.m_dwOfferID);

                            // upsell
                            uiIDA[0] = IDS_CONFIRM_OK;
                            uiIDA[1] = IDS_CONFIRM_CANCEL;
                            ui.RequestErrorMessage(
                                IDS_UNLOCK_TITLE, IDS_UNLOCK_ACCEPT_INVITE,
                                uiIDA, 2, i,
                                &CMinecraftApp::UnlockFullInviteReturned, this);
                        } else {
                            uiIDA[0] = IDS_CONFIRM_CANCEL;
                            uiIDA[1] = IDS_CONFIRM_OK;
                            ui.RequestAlertMessage(
                                IDS_EXIT_GAME, IDS_CONFIRM_LEAVE_VIA_INVITE,
                                uiIDA, 2, i,
                                &CMinecraftApp::ExitAndJoinFromInvite, this);
                        }
                    }
                } break;
                case eAppAction_ExitAndJoinFromInviteConfirmed: {
                    SetAction(i, eAppAction_Idle);

                    pMinecraft->gui->clearMessages();

                    // turn off the gamertags in splitscreen for the primary
                    // player, since they are about to be made fullscreen
                    ui.HideAllGameUIElements();

                    // Stop app running
                    SetGameStarted(false);

                    ui.CloseAllPlayersScenes();

                    // 4J Stu - Fix for #12368 - Crash: Game crashes when saving
                    // then exiting and selecting to save
                    for (unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx) {
                        // 4J Stu - Fix for #13257 - CRASH: Gameplay: Title
                        // crashed after exiting the tutorial It doesn't matter
                        // if they were in the tutorial already
                        pMinecraft->playerLeftTutorial(idx);
                    }

                    // 4J-PB - may have been using a texture pack with audio ,
                    // so clean up anything texture pack related here

                    // unload any texture pack audio
                    // if there is audio in use, clear out the audio, and
                    // unmount the pack
                    TexturePack* pTexPack =
                        Minecraft::GetInstance()->skins->getSelected();
                    DLCTexturePack* pDLCTexPack = nullptr;

                    if (pTexPack->hasAudio()) {
                        // get the dlc texture pack, and store it
                        pDLCTexPack = (DLCTexturePack*)pTexPack;
                    }

                    // change to the default texture pack
                    pMinecraft->skins->selectTexturePackById(
                        TexturePackRepository::DEFAULT_TEXTURE_PACK_ID);

                    if (pTexPack->hasAudio()) {
                        // need to stop the streaming audio - by playing
                        // streaming audio from the default texture pack now
                        // reset the streaming sounds back to the normal ones
                        pMinecraft->soundEngine->SetStreamingSounds(
                            eStream_Overworld_Calm1, eStream_Overworld_piano3,
                            eStream_Nether1, eStream_Nether4,
                            eStream_end_dragon, eStream_end_end, eStream_CD_1);
                        pMinecraft->soundEngine->playStreaming(L"", 0, 0, 0, 1,
                                                               1);

                        const unsigned int result =
                            StorageManager.UnmountInstalledDLC("TPACK");
                        app.DebugPrintf("Unmount result is %d\n", result);
                    }

                    LoadingInputParams* loadingParams =
                        new LoadingInputParams();
                    loadingParams->func =
                        &CGameNetworkManager::ExitAndJoinFromInviteThreadProc;
                    loadingParams->lpParam = (void*)&m_InviteData;

                    UIFullscreenProgressCompletionData* completionData =
                        new UIFullscreenProgressCompletionData();
                    completionData->bShowBackground = true;
                    completionData->bShowLogo = true;
                    completionData->iPad = DEFAULT_XUI_MENU_USER;
                    completionData->type = e_ProgressCompletion_NoAction;
                    loadingParams->completionData = completionData;

                    ui.NavigateToScene(ProfileManager.GetPrimaryPad(),
                                       eUIScene_FullscreenProgress,
                                       loadingParams);
                }

                break;
                case eAppAction_JoinFromInvite: {
                    SetAction(i, eAppAction_Idle);

                    // 4J Stu - Move this state block from
                    // CPlatformNetworkManager::ExitAndJoinFromInviteThreadProc,
                    // as g_NetworkManager.JoinGameFromInviteInfo ultimately can
                    // call NavigateToScene,
                    /// and we should only be calling that from the main thread
                    app.SetTutorialMode(false);

                    g_NetworkManager.SetLocalGame(false);

                    JoinFromInviteData* inviteData = (JoinFromInviteData*)param;
                    // 4J-PB - clear any previous connection errors
                    Minecraft::GetInstance()->clearConnectionFailed();

                    app.DebugPrintf(
                        "Changing Primary Pad on an invite accept - pad was "
                        "%d, and is now %d\n",
                        ProfileManager.GetPrimaryPad(),
                        inviteData->dwUserIndex);
                    ProfileManager.SetLockedProfile(inviteData->dwUserIndex);
                    ProfileManager.SetPrimaryPad(inviteData->dwUserIndex);

                    // change the minecraft player name
                    Minecraft::GetInstance()->user->name =
                        convStringToWstring(ProfileManager.GetGamertag(
                            ProfileManager.GetPrimaryPad()));

                    bool success = g_NetworkManager.JoinGameFromInviteInfo(
                        inviteData->dwUserIndex,       // dwUserIndex
                        inviteData->dwLocalUsersMask,  // dwUserMask
                        inviteData->pInviteInfo);      // pInviteInfo

                    if (!success) {
                        app.DebugPrintf("Failed joining game from invite\n");
                        // return hr;

                        // 4J Stu - Copied this from XUI_FullScreenProgress to
                        // properly handle the fail case, as the thread will no
                        // longer be failing
                        unsigned int uiIDA[1];
                        uiIDA[0] = IDS_CONFIRM_OK;
                        ui.RequestErrorMessage(
                            IDS_CONNECTION_FAILED, IDS_CONNECTION_LOST_SERVER,
                            uiIDA, 1, ProfileManager.GetPrimaryPad());

                        ui.NavigateToHomeMenu();
                        ui.UpdatePlayerBasePositions();
                    }
                } break;
                case eAppAction_ChangeSessionType: {
                    // If we are not in gameplay yet, then wait until the server
                    // is setup before changing the session type
                    if (g_NetworkManager.IsInGameplay()) {
                        // This kicks off a thread that waits for the server to
                        // end, then closes the current session, starts a new
                        // one and joins the local players into it

                        SetAction(i, eAppAction_Idle);

                        if (!GetChangingSessionType() &&
                            !g_NetworkManager.IsLocalGame()) {
                            SetGameStarted(false);
                            SetChangingSessionType(true);
                            SetReallyChangingSessionType(true);

                            // turn off the gamertags in splitscreen for the
                            // primary player, since they are about to be made
                            // fullscreen
                            ui.HideAllGameUIElements();

                            if (!ui.IsSceneInStack(
                                    ProfileManager.GetPrimaryPad(),
                                    eUIScene_EndPoem)) {
                                ui.CloseAllPlayersScenes();
                            }
                            ui.ShowOtherPlayersBaseScene(
                                ProfileManager.GetPrimaryPad(), true);

                            // Remove this line to fix:
                            // #49084 - TU5: Code: Gameplay: The title crashes
                            // every time client navigates to 'Play game' menu
                            // and loads/creates new game after a "Connection to
                            // Xbox LIVE was lost" message has appeared.
                            // app.NavigateToScene(0,eUIScene_Main);

                            LoadingInputParams* loadingParams =
                                new LoadingInputParams();
                            loadingParams->func =
                                &CGameNetworkManager::
                                    ChangeSessionTypeThreadProc;
                            loadingParams->lpParam = nullptr;

                            UIFullscreenProgressCompletionData* completionData =
                                new UIFullscreenProgressCompletionData();
                            completionData->bRequiresUserAction = true;
                            completionData->bShowBackground = true;
                            completionData->bShowLogo = true;
                            completionData->iPad = DEFAULT_XUI_MENU_USER;
                            if (ui.IsSceneInStack(
                                    ProfileManager.GetPrimaryPad(),
                                    eUIScene_EndPoem)) {
                                completionData->type =
                                    e_ProgressCompletion_NavigateBackToScene;
                                completionData->scene = eUIScene_EndPoem;
                            } else {
                                completionData->type =
                                    e_ProgressCompletion_CloseAllPlayersUIScenes;
                            }
                            loadingParams->completionData = completionData;

                            ui.NavigateToScene(ProfileManager.GetPrimaryPad(),
                                               eUIScene_FullscreenProgress,
                                               loadingParams);
                        }
                    } else if (g_NetworkManager.IsLeavingGame()) {
                        // If we are leaving the game, then ignore the state
                        // change
                        SetAction(i, eAppAction_Idle);
                    }
                } break;
                case eAppAction_SetDefaultOptions:
                    SetAction(i, eAppAction_Idle);
                    SetDefaultOptions((C_4JProfile::PROFILESETTINGS*)param, i);

                    // if the profile data has been changed, then force a
                    // profile write It seems we're allowed to break the 5
                    // minute rule if it's the result of a user action
                    CheckGameSettingsChanged(true, i);

                    break;

                case eAppAction_RemoteServerSave: {
                    // If the remote server save has already finished, don't
                    // complete the action
                    if (GetGameStarted()) {
                        SetAction(ProfileManager.GetPrimaryPad(),
                                  eAppAction_Idle);
                        break;
                    }

                    SetAction(i, eAppAction_WaitRemoteServerSaveComplete);

                    for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i) {
                        ui.CloseUIScenes(i, true);
                    }

                    // turn off the gamertags in splitscreen for the primary
                    // player, since they are about to be made fullscreen
                    ui.HideAllGameUIElements();

                    LoadingInputParams* loadingParams =
                        new LoadingInputParams();
                    loadingParams->func = &CMinecraftApp::RemoteSaveThreadProc;
                    loadingParams->lpParam = nullptr;

                    UIFullscreenProgressCompletionData* completionData =
                        new UIFullscreenProgressCompletionData();
                    completionData->bRequiresUserAction = false;
                    completionData->bShowBackground = true;
                    completionData->bShowLogo = true;
                    completionData->iPad = DEFAULT_XUI_MENU_USER;
                    if (ui.IsSceneInStack(ProfileManager.GetPrimaryPad(),
                                          eUIScene_EndPoem)) {
                        completionData->type =
                            e_ProgressCompletion_NavigateBackToScene;
                        completionData->scene = eUIScene_EndPoem;
                    } else {
                        completionData->type =
                            e_ProgressCompletion_CloseAllPlayersUIScenes;
                    }
                    loadingParams->completionData = completionData;

                    loadingParams->cancelFunc =
                        &CMinecraftApp::ExitGameFromRemoteSave;
                    loadingParams->cancelText = IDS_TOOLTIPS_EXIT;

                    ui.NavigateToScene(ProfileManager.GetPrimaryPad(),
                                       eUIScene_FullscreenProgress,
                                       loadingParams);
                } break;
                case eAppAction_WaitRemoteServerSaveComplete:
                    // Do nothing
                    break;
                case eAppAction_FailedToJoinNoPrivileges: {
                    unsigned int uiIDA[1];
                    uiIDA[0] = IDS_CONFIRM_OK;
                    C4JStorage::EMessageResult result = ui.RequestErrorMessage(
                        IDS_NO_MULTIPLAYER_PRIVILEGE_TITLE,
                        IDS_NO_MULTIPLAYER_PRIVILEGE_JOIN_TEXT, uiIDA, 1,
                        ProfileManager.GetPrimaryPad());
                    if (result != C4JStorage::EMessage_Busy)
                        SetAction(i, eAppAction_Idle);
                } break;
                case eAppAction_ProfileReadError:
                    // Return player to the main menu - code largely copied from
                    // that for handling eAppAction_PrimaryPlayerSignedOut,
                    // although I don't think we should have got as far as
                    // needing to halt the server, or running the game, before
                    // returning to the menu
                    if (!app.GetGameStarted())
                        MinecraftServer::HaltServer(true);

                    if (g_NetworkManager.IsInSession()) {
                        app.SetAction(
                            i, eAppAction_PrimaryPlayerSignedOutReturned);
                    } else {
                        app.SetAction(
                            i, eAppAction_PrimaryPlayerSignedOutReturned_Menus);
                        MinecraftServer::resetFlags();
                    }
                    break;

                case eAppAction_BanLevel: {
                    // It's possible that this state can get set after the game
                    // has been exited (e.g. by network disconnection) so we
                    // can't ban the level at that point
                    if (g_NetworkManager.IsInGameplay() &&
                        !g_NetworkManager.IsLeavingGame()) {
                        TelemetryManager->RecordBanLevel(i);

                        // primary player would exit the world, secondary would
                        // exit the player
                        if (ProfileManager.GetPrimaryPad() == i) {
                            SetAction(i, eAppAction_ExitWorld);
                        } else {
                            SetAction(i, eAppAction_ExitPlayer);
                        }
                    }
                } break;
                case eAppAction_LevelInBanLevelList: {
                    unsigned int uiIDA[2];
                    uiIDA[0] = IDS_BUTTON_REMOVE_FROM_BAN_LIST;
                    uiIDA[1] = IDS_EXIT_GAME;

                    // pass in the gamertag format std::string
                    wchar_t wchFormat[40];
                    INetworkPlayer* player =
                        g_NetworkManager.GetLocalPlayerByUserIndex(i);

                    // If not the primary player, but the primary player has
                    // banned this level and decided not to unban then we may
                    // have left the game by now
                    if (player) {
                        swprintf(wchFormat, 40, L"%ls\n\n%%ls",
                                 player->GetOnlineName());

                        C4JStorage::EMessageResult result =
                            ui.RequestErrorMessage(
                                IDS_BANNED_LEVEL_TITLE, IDS_PLAYER_BANNED_LEVEL,
                                uiIDA, 2, i,
                                &CMinecraftApp::BannedLevelDialogReturned, this,
                                wchFormat);
                        if (result != C4JStorage::EMessage_Busy)
                            SetAction(i, eAppAction_Idle);
                    } else {
                        SetAction(i, eAppAction_Idle);
                    }
                } break;
                case eAppAction_DebugText:
                    // launch the xui for text entry
                    {
                        SetAction(i, eAppAction_Idle);
                    }
                    break;

                case eAppAction_ReloadTexturePack: {
                    SetAction(i, eAppAction_Idle);
                    Minecraft* pMinecraft = Minecraft::GetInstance();
                    pMinecraft->textures->reloadAll();
                    pMinecraft->skins->updateUI();

                    if (!pMinecraft->skins->isUsingDefaultSkin()) {
                        TexturePack* pTexturePack =
                            pMinecraft->skins->getSelected();

                        DLCPack* pDLCPack = pTexturePack->getDLCPack();

                        bool purchased = false;
                        // do we have a license?
                        if (pDLCPack &&
                            pDLCPack->hasPurchasedFile(
                                DLCManager::e_DLCType_Texture, L"")) {
                            purchased = true;
                        }
                    }

                    // 4J-PB  - If the texture pack has audio, we need to switch
                    // to this
                    if (pMinecraft->skins->getSelected()->hasAudio()) {
                        Minecraft::GetInstance()->soundEngine->playStreaming(
                            L"", 0, 0, 0, 1, 1);
                    }
                } break;

                case eAppAction_ReloadFont: {
                    app.DebugPrintf(
                        "[Consoles_App] eAppAction_ReloadFont, ingame='%s'.\n",
                        app.GetGameStarted() ? "Yes" : "No");

                    SetAction(i, eAppAction_Idle);

                    ui.SetTooltips(i, -1);

                    ui.ReloadSkin();
                    ui.StartReloadSkinThread();

                    ui.setCleanupOnReload();
                } break;

                case eAppAction_TexturePackRequired: {
                    unsigned int uiIDA[2];

                    uiIDA[0] = IDS_TEXTUREPACK_FULLVERSION;
                    uiIDA[1] = IDS_TEXTURE_PACK_TRIALVERSION;

                    // Give the player a warning about the texture pack missing
                    ui.RequestErrorMessage(
                        IDS_DLC_TEXTUREPACK_NOT_PRESENT_TITLE,
                        IDS_DLC_TEXTUREPACK_NOT_PRESENT, uiIDA, 2,
                        ProfileManager.GetPrimaryPad(),
                        &CMinecraftApp::TexturePackDialogReturned, this);
                    SetAction(i, eAppAction_Idle);
                }

                break;
                default:
                    break;
            }
        }

        // Any TMS actions?

        eTMS = app.GetTMSAction(i);

        if (eTMS != eTMSAction_Idle) {
            switch (eTMS) {
                    // TMS++ actions
                case eTMSAction_TMSPP_RetrieveFiles_CreateLoad_SignInReturned:
                case eTMSAction_TMSPP_RetrieveFiles_RunPlayGame:
                    SetTMSAction(i, eTMSAction_TMSPP_UserFileList);
                    break;

                case eTMSAction_TMSPP_UserFileList:
                    // retrieve the file list first
                    SetTMSAction(i, eTMSAction_TMSPP_XUIDSFile);
                    break;
                case eTMSAction_TMSPP_XUIDSFile:
                    SetTMSAction(i, eTMSAction_TMSPP_DLCFile);

                    break;
                case eTMSAction_TMSPP_DLCFile:
                    SetTMSAction(i, eTMSAction_TMSPP_BannedListFile);
                    break;
                case eTMSAction_TMSPP_BannedListFile:
                    // If we have one in TMSPP, then we can assume we can ignore
                    // TMS
                    SetTMSAction(i, eTMSAction_TMS_RetrieveFiles_Complete);
                    break;

                    // SPECIAL CASE - where the user goes directly in to Help &
                    // Options from the main menu
                case eTMSAction_TMSPP_RetrieveFiles_HelpAndOptions:
                case eTMSAction_TMSPP_RetrieveFiles_DLCMain:
                    // retrieve the file list first
                    SetTMSAction(i, eTMSAction_TMSPP_DLCFileOnly);
                    break;
                case eTMSAction_TMSPP_RetrieveUserFilelist_DLCFileOnly:
                    SetTMSAction(i, eTMSAction_TMSPP_DLCFileOnly);

                    break;

                case eTMSAction_TMSPP_DLCFileOnly:
                    SetTMSAction(i, eTMSAction_TMSPP_RetrieveFiles_Complete);
                    break;

                case eTMSAction_TMSPP_RetrieveFiles_Complete:
                    SetTMSAction(i, eTMSAction_Idle);
                    break;

                    // TMS files
                    /*			case
                    eTMSAction_TMS_RetrieveFiles_CreateLoad_SignInReturned: case
                    eTMSAction_TMS_RetrieveFiles_RunPlayGame: #ifdef 0
                    SetTMSAction(i,eTMSAction_TMS_XUIDSFile_Waiting);
                    // pass in the next app action on the call or callback
                    completing
                    app.ReadXuidsFileFromTMS(i,eTMSAction_TMS_DLCFile,true);
                    #else
                    SetTMSAction(i,eTMSAction_TMS_DLCFile);
                    #endif
                    break;

                    case eTMSAction_TMS_DLCFile:
                    SetTMSAction(i,eTMSAction_TMS_BannedListFile);

                    break;

                    case eTMSAction_TMS_RetrieveFiles_HelpAndOptions:
                    case eTMSAction_TMS_RetrieveFiles_DLCMain:
                    SetTMSAction(i,eTMSAction_Idle);

                    break;
                    case eTMSAction_TMS_BannedListFile:

                    break;

                    */
                case eTMSAction_TMS_RetrieveFiles_Complete:
                    SetTMSAction(i, eTMSAction_Idle);
                    // 				if(StorageManager.SetSaveDevice(&CScene_Main::DeviceSelectReturned,pClass))
                    // 				{
                    // 					// save device already
                    // selected
                    // 					// ensure we've applied
                    // this player's settings
                    // 					app.ApplyGameSettingsChanged(ProfileManager.GetPrimaryPad());
                    // 					app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_MultiGameJoinLoad);
                    // 				}
                    break;
                default:
                    break;
            }
        }
    }
}

int CMinecraftApp::BannedLevelDialogReturned(
    void* pParam, int iPad, const C4JStorage::EMessageResult result) {
    CMinecraftApp* pApp = (CMinecraftApp*)pParam;
    // Minecraft *pMinecraft=Minecraft::GetInstance();

    if (result == C4JStorage::EMessage_ResultAccept) {
    } else {
        if (iPad == ProfileManager.GetPrimaryPad()) {
            pApp->SetAction(iPad, eAppAction_ExitWorld);
        } else {
            pApp->SetAction(iPad, eAppAction_ExitPlayer);
        }
    }

    return 0;
}
void CMinecraftApp::loadMediaArchive() {
    std::wstring mediapath = L"";

#if _WINDOWS64
    mediapath = L"Common\\Media\\MediaWindows64.arc";
#elif __linux__
    mediapath = L"Common/Media/MediaLinux.arc";
#endif

    if (!mediapath.empty()) {
        // boom headshot
#if defined(__linux__)
        std::wstring exeDirW = PathHelper::GetExecutableDirW();
        std::wstring candidate = exeDirW + File::pathSeparator + mediapath;
        if (File(candidate).exists()) {
            m_mediaArchive = new ArchiveFile(File(candidate));
        } else {
            m_mediaArchive = new ArchiveFile(File(mediapath));
        }
#else
        m_mediaArchive = new ArchiveFile(File(mediapath));
#endif
    }
}

void CMinecraftApp::loadStringTable() {
    if (m_stringTable != nullptr) {
        // we need to unload the current std::string table, this is a reload
        delete m_stringTable;
    }
    std::wstring localisationFile = L"languages.loc";
    if (m_mediaArchive->hasFile(localisationFile)) {
        byteArray locFile = m_mediaArchive->getFile(localisationFile);
        m_stringTable = new StringTable(locFile.data, locFile.length);
        delete[] locFile.data;
    } else {
        m_stringTable = nullptr;
        assert(false);
        // AHHHHHHHHH.
    }
}

int CMinecraftApp::PrimaryPlayerSignedOutReturned(
    void* pParam, int iPad, const C4JStorage::EMessageResult) {
    // CMinecraftApp* pApp = (CMinecraftApp*)pParam;
    // Minecraft *pMinecraft=Minecraft::GetInstance();

    // if the player is null, we're in the menus
    // if(Minecraft::GetInstance()->player!=nullptr)

    // We always create a session before kicking of any of the game code, so
    // even though we may still be joining/creating a game at this point we want
    // to handle it differently from just being in a menu
    if (g_NetworkManager.IsInSession()) {
        app.SetAction(iPad, eAppAction_PrimaryPlayerSignedOutReturned);
    } else {
        app.SetAction(iPad, eAppAction_PrimaryPlayerSignedOutReturned_Menus);
    }
    return 0;
}

int CMinecraftApp::EthernetDisconnectReturned(
    void* pParam, int iPad, const C4JStorage::EMessageResult) {
    // CMinecraftApp* pApp = (CMinecraftApp*)pParam;
    Minecraft* pMinecraft = Minecraft::GetInstance();

    // if the player is null, we're in the menus
    if (Minecraft::GetInstance()->player != nullptr) {
        app.SetAction(pMinecraft->player->GetXboxPad(),
                      eAppAction_EthernetDisconnectedReturned);
    } else {
        //  4J-PB - turn off the PSN store icon just in case this happened when
        //  we were in one of the DLC menus
        app.SetAction(iPad, eAppAction_EthernetDisconnectedReturned_Menus);
    }
    return 0;
}

int CMinecraftApp::SignoutExitWorldThreadProc(void* lpParameter) {
    // Share AABB & Vec3 pools with default (main thread) - should be ok as long
    // as we don't tick the main thread whilst this thread is running
    Compression::UseDefaultThreadStorage();

    // app.SetGameStarted(false);

    Minecraft* pMinecraft = Minecraft::GetInstance();

    int exitReasonStringId = -1;

    bool saveStats = false;
    if (pMinecraft->isClientSide() || g_NetworkManager.IsInSession()) {
        if (lpParameter != nullptr) {
            switch (app.GetDisconnectReason()) {
                case DisconnectPacket::eDisconnect_Kicked:
                    exitReasonStringId = IDS_DISCONNECTED_KICKED;
                    break;
                case DisconnectPacket::eDisconnect_NoUGC_AllLocal:
                    exitReasonStringId =
                        IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_ALL_LOCAL;
                    break;
                case DisconnectPacket::eDisconnect_NoUGC_Single_Local:
                    exitReasonStringId =
                        IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_SINGLE_LOCAL;
                    break;
                case DisconnectPacket::eDisconnect_NoFlying:
                    exitReasonStringId = IDS_DISCONNECTED_FLYING;
                    break;
                case DisconnectPacket::eDisconnect_OutdatedServer:
                    exitReasonStringId = IDS_DISCONNECTED_SERVER_OLD;
                    break;
                case DisconnectPacket::eDisconnect_OutdatedClient:
                    exitReasonStringId = IDS_DISCONNECTED_CLIENT_OLD;
                    break;
                default:
                    exitReasonStringId = IDS_DISCONNECTED;
            }
            pMinecraft->progressRenderer->progressStartNoAbort(
                exitReasonStringId);
            // 4J - Force a disconnection, this handles the situation that the
            // server has already disconnected
            if (pMinecraft->levels[0] != nullptr)
                pMinecraft->levels[0]->disconnect(false);
            if (pMinecraft->levels[1] != nullptr)
                pMinecraft->levels[1]->disconnect(false);
        } else {
            exitReasonStringId = IDS_EXITING_GAME;
            pMinecraft->progressRenderer->progressStartNoAbort(
                IDS_EXITING_GAME);

            if (pMinecraft->levels[0] != nullptr)
                pMinecraft->levels[0]->disconnect();
            if (pMinecraft->levels[1] != nullptr)
                pMinecraft->levels[1]->disconnect();
        }

        // 4J Stu - This only does something if we actually have a server, so
        // don't need to do any other checks
        MinecraftServer::HaltServer(true);

        // We need to call the stats & leaderboards save before we exit the
        // session
        // pMinecraft->forceStatsSave();
        saveStats = false;

        // 4J Stu - Leave the session once the disconnect packet has been sent
        g_NetworkManager.LeaveGame(false);
    } else {
        if (lpParameter != nullptr) {
            switch (app.GetDisconnectReason()) {
                case DisconnectPacket::eDisconnect_Kicked:
                    exitReasonStringId = IDS_DISCONNECTED_KICKED;
                    break;
                case DisconnectPacket::eDisconnect_NoUGC_AllLocal:
                    exitReasonStringId =
                        IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_ALL_LOCAL;
                    break;
                case DisconnectPacket::eDisconnect_NoUGC_Single_Local:
                    exitReasonStringId =
                        IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_SINGLE_LOCAL;
                    break;
                case DisconnectPacket::eDisconnect_OutdatedServer:
                    exitReasonStringId = IDS_DISCONNECTED_SERVER_OLD;
                    break;
                case DisconnectPacket::eDisconnect_OutdatedClient:
                    exitReasonStringId = IDS_DISCONNECTED_CLIENT_OLD;
                default:
                    exitReasonStringId = IDS_DISCONNECTED;
            }
            pMinecraft->progressRenderer->progressStartNoAbort(
                exitReasonStringId);
        }
    }
    pMinecraft->setLevel(nullptr, exitReasonStringId, nullptr, saveStats, true);

    // 4J-JEV: Fix for #106402 - TCR #014 BAS Debug Output:
    // TU12: Mass Effect Mash-UP: Save file "Default_DisplayName" is created on
    // all storage devices after signing out from a re-launched pre-generated
    // world
    app.m_gameRules.unloadCurrentGameRules();  //

    MinecraftServer::resetFlags();

    // We can't start/join a new game until the session is destroyed, so wait
    // for it to be idle again
    while (g_NetworkManager.IsInSession()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return S_OK;
}

int CMinecraftApp::UnlockFullInviteReturned(void* pParam, int iPad,
                                            C4JStorage::EMessageResult result) {
    // CMinecraftApp* pApp = (CMinecraftApp*)pParam;
    Minecraft* pMinecraft = Minecraft::GetInstance();
    bool bNoPlayer;

    // bug 11285 - TCR 001: BAS Game Stability: CRASH - When trying to join a
    // full version game with a trial version, the trial crashes 4J-PB - we may
    // be in the main menus here, and we don't have a pMinecraft->player

    if (pMinecraft->player == nullptr) {
        bNoPlayer = true;
    }

    if (result == C4JStorage::EMessage_ResultAccept) {
        if (ProfileManager.IsSignedInLive(iPad)) {
            // 4J-PB - need to check this user can access the store
            {
                ProfileManager.DisplayFullVersionPurchase(
                    false, iPad, eSen_UpsellID_Full_Version_Of_Game);
            }
        }
    } else {
        TelemetryManager->RecordUpsellResponded(
            iPad, eSen_UpsellID_Full_Version_Of_Game, app.m_dwOfferID,
            eSen_UpsellOutcome_Declined);
    }

    return 0;
}

int CMinecraftApp::UnlockFullSaveReturned(void* pParam, int iPad,
                                          C4JStorage::EMessageResult result) {
    // CMinecraftApp* pApp = (CMinecraftApp*)pParam;
    Minecraft* pMinecraft = Minecraft::GetInstance();

    if (result == C4JStorage::EMessage_ResultAccept) {
        if (ProfileManager.IsSignedInLive(pMinecraft->player->GetXboxPad())) {
            // 4J-PB - need to check this user can access the store
            {
                ProfileManager.DisplayFullVersionPurchase(
                    false, pMinecraft->player->GetXboxPad(),
                    eSen_UpsellID_Full_Version_Of_Game);
            }
        }
    } else {
        TelemetryManager->RecordUpsellResponded(
            iPad, eSen_UpsellID_Full_Version_Of_Game, app.m_dwOfferID,
            eSen_UpsellOutcome_Declined);
    }

    return 0;
}

int CMinecraftApp::UnlockFullExitReturned(void* pParam, int iPad,
                                          C4JStorage::EMessageResult result) {
    CMinecraftApp* pApp = (CMinecraftApp*)pParam;
    Minecraft* pMinecraft = Minecraft::GetInstance();

    if (result == C4JStorage::EMessage_ResultAccept) {
        if (ProfileManager.IsSignedInLive(pMinecraft->player->GetXboxPad())) {
            // 4J-PB - need to check this user can access the store
            {
                ProfileManager.DisplayFullVersionPurchase(
                    false, pMinecraft->player->GetXboxPad(),
                    eSen_UpsellID_Full_Version_Of_Game);
            }
        }
    } else {
        TelemetryManager->RecordUpsellResponded(
            iPad, eSen_UpsellID_Full_Version_Of_Game, app.m_dwOfferID,
            eSen_UpsellOutcome_Declined);
        pApp->SetAction(pMinecraft->player->GetXboxPad(),
                        eAppAction_ExitWorldTrial);
    }

    return 0;
}

int CMinecraftApp::TrialOverReturned(void* pParam, int iPad,
                                     C4JStorage::EMessageResult result) {
    CMinecraftApp* pApp = (CMinecraftApp*)pParam;
    Minecraft* pMinecraft = Minecraft::GetInstance();

    if (result == C4JStorage::EMessage_ResultAccept) {
        // we need a signed in user for the unlock
        if (ProfileManager.IsSignedInLive(pMinecraft->player->GetXboxPad())) {
            // 4J-PB - need to check this user can access the store
            {
                ProfileManager.DisplayFullVersionPurchase(
                    false, pMinecraft->player->GetXboxPad(),
                    eSen_UpsellID_Full_Version_Of_Game);
            }
        } else {
            pApp->SetAction(pMinecraft->player->GetXboxPad(),
                            eAppAction_ExitTrial);
        }
    } else {
        TelemetryManager->RecordUpsellResponded(
            iPad, eSen_UpsellID_Full_Version_Of_Game, app.m_dwOfferID,
            eSen_UpsellOutcome_Declined);

        pApp->SetAction(pMinecraft->player->GetXboxPad(), eAppAction_ExitTrial);
    }

    return 0;
}

void CMinecraftApp::ProfileReadErrorCallback(void* pParam) {
    CMinecraftApp* pApp = (CMinecraftApp*)pParam;
    int iPrimaryPlayer = ProfileManager.GetPrimaryPad();
    pApp->SetAction(iPrimaryPlayer, eAppAction_ProfileReadError);
}

void CMinecraftApp::ClearSignInChangeUsersMask() {
    // 4J-PB - When in the main menu, the user is on pad 0, and any change they
    // make to their profile will be to pad 0 data If they then go in as a
    // secondary player to a splitscreen game, their profile will not be read
    // again on pad 1 if they were previously in a splitscreen game This is
    // because m_uiLastSignInData remembers they were in previously, and doesn't
    // read the profile data for them again Fix this by resetting the
    // m_uiLastSignInData on pressing play game for secondary users. The Primary
    // user does a read profile on play game anyway
    int iPrimaryPlayer = ProfileManager.GetPrimaryPad();

    if (m_uiLastSignInData != 0) {
        if (iPrimaryPlayer >= 0) {
            m_uiLastSignInData = 1 << iPrimaryPlayer;
        } else {
            m_uiLastSignInData = 0;
        }
    }
}
void CMinecraftApp::SignInChangeCallback(void* pParam,
                                         bool bPrimaryPlayerChanged,
                                         unsigned int uiSignInData) {
    CMinecraftApp* pApp = (CMinecraftApp*)pParam;
    // check if the primary player signed out
    int iPrimaryPlayer = ProfileManager.GetPrimaryPad();

    if ((ProfileManager.GetLockedProfile() != -1) && iPrimaryPlayer != -1) {
        if (((uiSignInData & (1 << iPrimaryPlayer)) == 0) ||
            bPrimaryPlayerChanged) {
            // Primary Player gone or there's been a sign out and sign in of the
            // primary player, so kick them out
            pApp->SetAction(iPrimaryPlayer, eAppAction_PrimaryPlayerSignedOut);

            // 4J-PB - invalidate their banned level list
            pApp->InvalidateBannedList(iPrimaryPlayer);

            // need to ditch any DLCOffers info
            StorageManager.ClearDLCOffers();
            pApp->ClearAndResetDLCDownloadQueue();
            pApp->ClearDLCInstalled();
        } else {
            unsigned int uiChangedPlayers = uiSignInData ^ m_uiLastSignInData;

            if (g_NetworkManager.IsInSession()) {
                bool hasGuestIdChanged = false;
                for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i) {
                    unsigned int guestNumber = 0;
                    if (ProfileManager.IsSignedIn(i)) {
                        XUSER_SIGNIN_INFO info;
                        XUserGetSigninInfo(
                            i, XUSER_GET_SIGNIN_INFO_OFFLINE_XUID_ONLY, &info);
                        pApp->DebugPrintf(
                            "Player at index %d has guest number %d\n", i,
                            info.dwGuestNumber);
                        guestNumber = info.dwGuestNumber;
                    }
                    if (pApp->m_currentSigninInfo[i].dwGuestNumber != 0 &&
                        guestNumber != 0 &&
                        pApp->m_currentSigninInfo[i].dwGuestNumber !=
                            guestNumber) {
                        hasGuestIdChanged = true;
                    }
                }

                if (hasGuestIdChanged) {
                    unsigned int uiIDA[1];
                    uiIDA[0] = IDS_CONFIRM_OK;
                    ui.RequestErrorMessage(IDS_GUEST_ORDER_CHANGED_TITLE,
                                           IDS_GUEST_ORDER_CHANGED_TEXT, uiIDA,
                                           1, ProfileManager.GetPrimaryPad());
                }

                // 4J Stu - On PS4 we can also cause to exit players if they are
                // signed out here, but we shouldn't do that if we are going to
                // switch to an offline game as it will likely crash due to
                // incompatible parallel processes
                bool switchToOffline = false;
                // If it's an online game, and the primary profile is no longer
                // signed into LIVE then we act as if disconnected
                if (!ProfileManager.IsSignedInLive(
                        ProfileManager.GetLockedProfile()) &&
                    !g_NetworkManager.IsLocalGame()) {
                    switchToOffline = true;
                }

                // printf("Old: %x, New: %x, Changed: %x\n", m_ulLastSignInData,
                // ulSignInData, changedPlayers);
                for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i) {
                    // Primary player shouldn't be subjected to these checks,
                    // and shouldn't call ExitPlayer
                    if (i == iPrimaryPlayer) continue;

                    // A guest a signed in or out, out of order which
                    // invalidates all the guest players we have in the game
                    if (hasGuestIdChanged &&
                        pApp->m_currentSigninInfo[i].dwGuestNumber != 0 &&
                        g_NetworkManager.GetLocalPlayerByUserIndex(i) !=
                            nullptr) {
                        pApp->DebugPrintf(
                            "Recommending removal of player at index %d "
                            "because their guest id changed\n",
                            i);
                        pApp->SetAction(i, eAppAction_ExitPlayer);
                    } else {
                        XUSER_SIGNIN_INFO info;
                        XUserGetSigninInfo(
                            i, XUSER_GET_SIGNIN_INFO_OFFLINE_XUID_ONLY, &info);
                        // 4J Stu - Also need to detect the case where the sign
                        // in mask is the same, but the player has swapped users
                        // (eg still signed in but xuid different) Fix for
                        // #48451 - TU5: Code: UI: Splitscreen: Title crashes
                        // when switching to a profile previously signed out via
                        // splitscreen profile selection

                        // 4J-PB - compiler complained about if below ('&&'
                        // within '||') - making it easier to read
                        bool bPlayerChanged =
                            (uiChangedPlayers & (1 << i)) == (1 << i);
                        bool bPlayerSignedIn = ((uiSignInData & (1 << i)) != 0);

                        if (bPlayerChanged &&
                            (!bPlayerSignedIn ||
                             (bPlayerSignedIn &&
                              !ProfileManager.AreXUIDSEqual(
                                  pApp->m_currentSigninInfo[i].xuid,
                                  info.xuid)))) {
                            // 4J-PB - invalidate their banned level list
                            pApp->DebugPrintf(
                                "Player at index %d Left - invalidating their "
                                "banned list\n",
                                i);
                            pApp->InvalidateBannedList(i);

                            // 4J-HG: If either the player is in the network
                            // manager or in the game, need to exit player
                            // TODO: Do we need to check the network manager?
                            if (g_NetworkManager.GetLocalPlayerByUserIndex(i) !=
                                    nullptr ||
                                Minecraft::GetInstance()->localplayers[i] !=
                                    nullptr) {
                                pApp->DebugPrintf("Player %d signed out\n", i);
                                pApp->SetAction(i, eAppAction_ExitPlayer);
                            }
                        }
                    }
                }

                // If it's an online game, and the primary profile is no longer
                // signed into LIVE then we act as if disconnected
                if (switchToOffline) {
                    pApp->SetAction(iPrimaryPlayer,
                                    eAppAction_EthernetDisconnected);
                }

                g_NetworkManager.HandleSignInChange();
            }
            // Some menus require the player to be signed in to live, so if this
            // callback happens and the primary player is no longer signed in
            // then nav back
            else if (pApp->GetLiveLinkRequired() &&
                     !ProfileManager.IsSignedInLive(
                         ProfileManager.GetLockedProfile())) {
                {
                    pApp->SetAction(iPrimaryPlayer,
                                    eAppAction_EthernetDisconnected);
                }
            }
        }
        m_uiLastSignInData = uiSignInData;
    } else if (iPrimaryPlayer != -1) {
        // make sure the TMS banned list data is ditched - the player may have
        // gone in to help & options, backed out, and signed out
        pApp->InvalidateBannedList(iPrimaryPlayer);

        // need to ditch any DLCOffers info
        StorageManager.ClearDLCOffers();
        pApp->ClearAndResetDLCDownloadQueue();
        pApp->ClearDLCInstalled();
    }

    // Update the guest numbers to the current state
    for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i) {
        if (FAILED(XUserGetSigninInfo(i,
                                      XUSER_GET_SIGNIN_INFO_OFFLINE_XUID_ONLY,
                                      &pApp->m_currentSigninInfo[i]))) {
            pApp->m_currentSigninInfo[i].xuid = INVALID_XUID;
            pApp->m_currentSigninInfo[i].dwGuestNumber = 0;
        }
        app.DebugPrintf("Player at index %d has guest number %d\n", i,
                        pApp->m_currentSigninInfo[i].dwGuestNumber);
    }
}

void CMinecraftApp::NotificationsCallback(void* pParam,
                                          std::uint32_t dwNotification,
                                          unsigned int uiParam) {
    CMinecraftApp* pClass = (CMinecraftApp*)pParam;

    // push these on to the notifications to be handled in qnet's dowork

    PNOTIFICATION pNotification = new NOTIFICATION;

    pNotification->dwNotification = dwNotification;
    pNotification->uiParam = uiParam;

    switch (dwNotification) {
        case XN_SYS_SIGNINCHANGED: {
            pClass->DebugPrintf("Signing changed - %d\n", uiParam);
        } break;
        case XN_SYS_INPUTDEVICESCHANGED:
            if (app.GetGameStarted() && g_NetworkManager.IsInSession()) {
                for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i) {
                    if (!InputManager.IsPadConnected(i) &&
                        Minecraft::GetInstance()->localplayers[i] != nullptr &&
                        !ui.IsPauseMenuDisplayed(i) &&
                        !ui.IsSceneInStack(i, eUIScene_EndPoem)) {
                        ui.CloseUIScenes(i);
                        ui.NavigateToScene(i, eUIScene_PauseMenu);
                    }
                }
            }
            break;
        case XN_LIVE_CONTENT_INSTALLED:
            // Need to inform xuis that we've possibly had DLC installed
            {
                // app.m_dlcManager.SetNeedsUpdated(true);
                //  Clear the DLC installed flag to cause a GetDLC to run if
                //  it's called
                app.ClearDLCInstalled();

                ui.HandleDLCInstalled(ProfileManager.GetPrimaryPad());
            }
            break;
        case XN_SYS_STORAGEDEVICESCHANGED: {
        } break;
    }

    pClass->m_vNotifications.push_back(pNotification);
}

void CMinecraftApp::UpsellReturnedCallback(void* pParam, eUpsellType type,
                                           eUpsellResponse result,
                                           int iUserData) {
    ESen_UpsellID senType;
    ESen_UpsellOutcome senResponse;

    // Map the eUpsellResponse to the enum we use for sentient
    switch (result) {
        case eUpsellResponse_Accepted_NoPurchase:
            senResponse = eSen_UpsellOutcome_Went_To_Guide;
            break;
        case eUpsellResponse_Accepted_Purchase:
            senResponse = eSen_UpsellOutcome_Accepted;
            break;
        case eUpsellResponse_Declined:
        default:
            senResponse = eSen_UpsellOutcome_Declined;
            break;
    };

    // Map the eUpsellType to the enum we use for sentient
    switch (type) {
        case eUpsellType_Custom:
            senType = eSen_UpsellID_Full_Version_Of_Game;
            break;
        default:
            senType = eSen_UpsellID_Undefined;
            break;
    };

    // Always the primary pad that gets an upsell
    TelemetryManager->RecordUpsellResponded(ProfileManager.GetPrimaryPad(),
                                            eSen_UpsellID_Full_Version_Of_Game,
                                            app.m_dwOfferID, senResponse);
}

#if defined(_DEBUG_MENUS_ENABLED)
bool CMinecraftApp::DebugArtToolsOn() {
    return DebugSettingsOn() &&
           (GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad()) &
            (1L << eDebugSetting_ArtTools)) != 0;
}
#endif

void CMinecraftApp::SetDebugSequence(const char* pchSeq) {
    InputManager.SetDebugSequence(pchSeq, &CMinecraftApp::DebugInputCallback,
                                  this);
}
int CMinecraftApp::DebugInputCallback(void* pParam) {
    CMinecraftApp* pClass = (CMinecraftApp*)pParam;
    // printf("sequence matched\n");
    pClass->m_bDebugOptions = !pClass->m_bDebugOptions;

    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
        if (app.DebugSettingsOn()) {
            app.ActionDebugMask(i);
        } else {
            // force debug mask off
            app.ActionDebugMask(i, true);
        }
    }

    return 0;
}

int CMinecraftApp::GetLocalPlayerCount(void) {
    int iPlayerC = 0;
    Minecraft* pMinecraft = Minecraft::GetInstance();
    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
        if (pMinecraft != nullptr && pMinecraft->localplayers[i] != nullptr) {
            iPlayerC++;
        }
    }

    return iPlayerC;
}

int CMinecraftApp::MarketplaceCountsCallback(
    void* pParam, C4JStorage::DLC_TMS_DETAILS* pTMSDetails, int iPad) {
    app.DebugPrintf("Marketplace Counts= New - %d Total - %d\n",
                    pTMSDetails->dwNewOffers, pTMSDetails->dwTotalOffers);

    if (pTMSDetails->dwNewOffers > 0) {
        app.m_bNewDLCAvailable = true;
        app.m_bSeenNewDLCTip = false;
    } else {
        app.m_bNewDLCAvailable = false;
        app.m_bSeenNewDLCTip = true;
    }

    return 0;
}

bool CMinecraftApp::StartInstallDLCProcess(int iPad) {
    app.DebugPrintf("--- CMinecraftApp::StartInstallDLCProcess: pad=%i.\n",
                    iPad);

    // If there is already a call to this in progress, then do nothing
    // If the app says dlc is installed, then there has been no new system
    // message to tell us there's new DLC since the last call to
    // StartInstallDLCProcess
    if ((app.DLCInstallProcessCompleted() == false) &&
        (m_bDLCInstallPending == false)) {
        app.m_dlcManager.resetUnnamedCorruptCount();
        m_bDLCInstallPending = true;
        m_iTotalDLC = 0;
        m_iTotalDLCInstalled = 0;
        app.DebugPrintf(
            "--- CMinecraftApp::StartInstallDLCProcess - "
            "StorageManager.GetInstalledDLC\n");

        StorageManager.GetInstalledDLC(
            iPad, &CMinecraftApp::DLCInstalledCallback, this);
        return true;
    } else {
        app.DebugPrintf(
            "--- CMinecraftApp::StartInstallDLCProcess - nothing to do\n");

        return false;
    }
}

// Installed DLC callback
int CMinecraftApp::DLCInstalledCallback(void* pParam, int iInstalledC,
                                        int iPad) {
    app.DebugPrintf(
        "--- CMinecraftApp::DLCInstalledCallback: totalDLC=%i, pad=%i.\n",
        iInstalledC, iPad);
    app.m_iTotalDLC = iInstalledC;
    app.MountNextDLC(iPad);
    return 0;
}

void CMinecraftApp::MountNextDLC(int iPad) {
    app.DebugPrintf("--- CMinecraftApp::MountNextDLC: pad=%i.\n", iPad);
    if (m_iTotalDLCInstalled < m_iTotalDLC) {
        // Mount it
        // We also need to match the ones the user wants to mount with the
        // installed DLC We're supposed to use a generic save game as a cache of
        // these to do this, with XUSER_ANY

        if (StorageManager.MountInstalledDLC(iPad, m_iTotalDLCInstalled,
                                             &CMinecraftApp::DLCMountedCallback,
                                             this) != ERROR_IO_PENDING) {
            // corrupt DLC
            app.DebugPrintf("Failed to mount DLC %d for pad %d\n",
                            m_iTotalDLCInstalled, iPad);
            ++m_iTotalDLCInstalled;
            app.MountNextDLC(iPad);
        } else {
            app.DebugPrintf("StorageManager.MountInstalledDLC ok\n");
        }
    } else {
        /* Removed - now loading these on demand instead of as each pack is
        mounted if(m_iTotalDLCInstalled > 0)
        {
        Minecraft *pMinecraft=Minecraft::GetInstance();
        pMinecraft->levelRenderer->AddDLCSkinsToMemTextures();
        }
        */

        m_bDLCInstallPending = false;
        m_bDLCInstallProcessCompleted = true;

        ui.HandleDLCMountingComplete();
    }
}

// 4J-JEV: For the sake of clarity in DLCMountedCallback.
#if defined(_WINDOWS64)
#define CONTENT_DATA_DISPLAY_NAME(a) (a.szDisplayName)
#else
#define CONTENT_DATA_DISPLAY_NAME(a) (a.wszDisplayName)
#endif

int CMinecraftApp::DLCMountedCallback(void* pParam, int iPad,
                                      std::uint32_t dwErr,
                                      std::uint32_t dwLicenceMask) {
#if defined(_WINDOWS64)
    app.DebugPrintf("--- CMinecraftApp::DLCMountedCallback\n");

    if (dwErr != ERROR_SUCCESS) {
        // corrupt DLC
        app.DebugPrintf("Failed to mount DLC for pad %d: %u\n", iPad, dwErr);
        app.m_dlcManager.incrementUnnamedCorruptCount();
    } else {
        XCONTENT_DATA ContentData =
            StorageManager.GetDLC(app.m_iTotalDLCInstalled);

        DLCPack* pack =
            app.m_dlcManager.getPack(CONTENT_DATA_DISPLAY_NAME(ContentData));

        if (pack != nullptr && pack->IsCorrupt()) {
            app.DebugPrintf(
                "Pack '%ls' is corrupt, removing it from the DLC Manager.\n",
                CONTENT_DATA_DISPLAY_NAME(ContentData));

            app.m_dlcManager.removePack(pack);
            pack = nullptr;
        }

        if (pack == nullptr) {
            app.DebugPrintf("Pack \"%ls\" is not installed, so adding it\n",
                            CONTENT_DATA_DISPLAY_NAME(ContentData));

#if defined(_WINDOWS64)
            pack = new DLCPack(ContentData.szDisplayName, dwLicenceMask);
#else
            pack = new DLCPack(ContentData.wszDisplayName, dwLicenceMask);
#endif
            pack->SetDLCMountIndex(app.m_iTotalDLCInstalled);
            pack->SetDLCDeviceID(ContentData.DeviceID);
            app.m_dlcManager.addPack(pack);

            app.HandleDLC(pack);

            if (pack->getDLCItemsCount(DLCManager::e_DLCType_Texture) > 0) {
                Minecraft::GetInstance()->skins->addTexturePackFromDLC(
                    pack, pack->GetPackId());
            }
        } else {
            app.DebugPrintf(
                "Pack \"%ls\" is already installed. Updating license to %u\n",
                CONTENT_DATA_DISPLAY_NAME(ContentData), dwLicenceMask);

            pack->SetDLCMountIndex(app.m_iTotalDLCInstalled);
            pack->SetDLCDeviceID(ContentData.DeviceID);
            pack->updateLicenseMask(dwLicenceMask);
        }

        StorageManager.UnmountInstalledDLC();
    }
    ++app.m_iTotalDLCInstalled;
    app.MountNextDLC(iPad);

#endif
    return 0;
}
#undef CONTENT_DATA_DISPLAY_NAME

//  void CMinecraftApp::InstallDefaultCape()
//  {
// 	 if(!m_bDefaultCapeInstallAttempted)
// 	 {
// 		 // we only attempt to install the cape once per launch of the
// game 		 m_bDefaultCapeInstallAttempted=true;
//
// 		 std::wstring wTemp=L"Default_Cape.png";
// 		 bool bRes=app.IsFileInMemoryTextures(wTemp);
// 		 // if the file is not already in the memory textures, then read
// it from TMS 		 if(!bRes)
// 		 {
// 			 std::uint8_t *pBuffer=nullptr;
// 			 std::uint32_t dwSize=0;
// 			 // 4J-PB - out for now for DaveK so he doesn't get the
// birthday cape #ifdef _CONTENT_PACKAGE
// C4JStorage::ETMSStatus eTMSStatus;
// 			 eTMSStatus=StorageManager.ReadTMSFile(ProfileManager.GetPrimaryPad(),C4JStorage::eGlobalStorage_Title,C4JStorage::eTMS_FileType_Graphic,
// L"Default_Cape.png",&pBuffer, &dwSize);
// 			 if(eTMSStatus==C4JStorage::ETMSStatus_Idle)
// 			 {
// 				 app.AddMemoryTextureFile(wTemp,pBuffer,dwSize);
// 			 }
// #endif
// 		 }
// 	 }
//  }

void CMinecraftApp::HandleDLC(DLCPack* pack) {
    unsigned int dwFilesProcessed = 0;
#if defined(_WINDOWS64) || defined(__linux__)
    std::vector<std::string> dlcFilenames;
#endif
    StorageManager.GetMountedDLCFileList("DLCDrive", dlcFilenames);
    for (int i = 0; i < dlcFilenames.size(); i++) {
        m_dlcManager.readDLCDataFile(dwFilesProcessed, dlcFilenames[i], pack);
    }

    if (dwFilesProcessed == 0) m_dlcManager.removePack(pack);
}

//  int CMinecraftApp::DLCReadCallback(void*
//  pParam,C4JStorage::DLC_FILE_DETAILS *pDLCData)
//  {
//
//
// 	 return 0;
//  }

//-------------------------------------------------------------------------------------
// Name: InitTime()
// Desc: Initializes the timer variables
//-------------------------------------------------------------------------------------
void CMinecraftApp::InitTime() {
    // Get the frequency of the timer
    LARGE_INTEGER qwTicksPerSec;
    QueryPerformanceFrequency(&qwTicksPerSec);
    m_Time.fSecsPerTick = 1.0f / (float)qwTicksPerSec.QuadPart;

    // Save the start time
    QueryPerformanceCounter(&m_Time.qwTime);

    // Zero out the elapsed and total time
    m_Time.qwAppTime.QuadPart = 0;
    m_Time.fAppTime = 0.0f;
    m_Time.fElapsedTime = 0.0f;
}

//-------------------------------------------------------------------------------------
// Name: UpdateTime()
// Desc: Updates the elapsed time since our last frame.
//-------------------------------------------------------------------------------------
void CMinecraftApp::UpdateTime() {
    LARGE_INTEGER qwNewTime;
    LARGE_INTEGER qwDeltaTime;

    QueryPerformanceCounter(&qwNewTime);
    qwDeltaTime.QuadPart = qwNewTime.QuadPart - m_Time.qwTime.QuadPart;

    m_Time.qwAppTime.QuadPart += qwDeltaTime.QuadPart;
    m_Time.qwTime.QuadPart = qwNewTime.QuadPart;

    m_Time.fElapsedTime =
        m_Time.fSecsPerTick * static_cast<float>(qwDeltaTime.QuadPart);
    m_Time.fAppTime =
        m_Time.fSecsPerTick * static_cast<float>(m_Time.qwAppTime.QuadPart);
}

bool CMinecraftApp::isXuidNotch(PlayerUID xuid) {
    if (m_xuidNotch != INVALID_XUID && xuid != INVALID_XUID) {
        return ProfileManager.AreXUIDSEqual(xuid, m_xuidNotch);
    }
    return false;
}

bool CMinecraftApp::isXuidDeadmau5(PlayerUID xuid) {
    auto it = MojangData.find(xuid);  // 4J Stu - The .at and [] accessors
                                      // insert elements if they don't exist
    if (it != MojangData.end()) {
        MOJANG_DATA* pMojangData = MojangData[xuid];
        if (pMojangData && pMojangData->eXuid == eXUID_Deadmau5) {
            return true;
        }
    }

    return false;
}

void CMinecraftApp::AddMemoryTextureFile(const std::wstring& wName,
                                         std::uint8_t* pbData,
                                         unsigned int byteCount) {
    std::lock_guard<std::mutex> lock(csMemFilesLock);
    // check it's not already in
    PMEMDATA pData = nullptr;
    auto it = m_MEM_Files.find(wName);
    if (it != m_MEM_Files.end()) {
#if !defined(_CONTENT_PACKAGE)
        wprintf(L"Incrementing the memory texture file count for %ls\n",
                wName.c_str());
#endif
        pData = (*it).second;

        if (pData->byteCount == 0 && byteCount != 0) {
            // This should never be nullptr if dwBytes is 0
            if (pData->pbData != nullptr) delete[] pData->pbData;

            pData->pbData = pbData;
            pData->byteCount = byteCount;
        }

        ++pData->ucRefCount;
        return;
    }

#if !defined(_CONTENT_PACKAGE)
    // wprintf(L"Adding the memory texture file data for %ls\n", wName.c_str());
#endif
    // this is a texture (png) file

    // add this texture to the list of memory texture files - it will then be
    // picked up by the level renderer's AddEntity

    pData = new MEMDATA();
    pData->pbData = pbData;
    pData->byteCount = byteCount;
    pData->ucRefCount = 1;

    // use the xuid to access the skin data
    m_MEM_Files[wName] = pData;
}

void CMinecraftApp::RemoveMemoryTextureFile(const std::wstring& wName) {
    std::lock_guard<std::mutex> lock(csMemFilesLock);

    auto it = m_MEM_Files.find(wName);
    if (it != m_MEM_Files.end()) {
#if !defined(_CONTENT_PACKAGE)
        wprintf(L"Decrementing the memory texture file count for %ls\n",
                wName.c_str());
#endif
        PMEMDATA pData = (*it).second;
        --pData->ucRefCount;
        if (pData->ucRefCount <= 0) {
#if !defined(_CONTENT_PACKAGE)
            wprintf(L"Erasing the memory texture file data for %ls\n",
                    wName.c_str());
#endif
            delete pData;
            m_MEM_Files.erase(wName);
        }
    }
}

bool CMinecraftApp::DefaultCapeExists() {
    std::wstring wTex = L"Special_Cape.png";
    bool val = false;

    { std::lock_guard<std::mutex> lock(csMemFilesLock);
    auto it = m_MEM_Files.find(wTex);
    if (it != m_MEM_Files.end()) val = true;
    }

    return val;
}

bool CMinecraftApp::IsFileInMemoryTextures(const std::wstring& wName) {
    bool val = false;

    { std::lock_guard<std::mutex> lock(csMemFilesLock);
    auto it = m_MEM_Files.find(wName);
    if (it != m_MEM_Files.end()) val = true;
    }

    return val;
}

void CMinecraftApp::GetMemFileDetails(const std::wstring& wName,
                                      std::uint8_t** ppbData,
                                      unsigned int* pByteCount) {
    std::lock_guard<std::mutex> lock(csMemFilesLock);
    auto it = m_MEM_Files.find(wName);
    if (it != m_MEM_Files.end()) {
        PMEMDATA pData = (*it).second;
        *ppbData = pData->pbData;
        *pByteCount = pData->byteCount;
    }
}

void CMinecraftApp::AddMemoryTPDFile(int iConfig, std::uint8_t* pbData,
                                     unsigned int byteCount) {
    std::lock_guard<std::mutex> lock(csMemTPDLock);
    // check it's not already in
    PMEMDATA pData = nullptr;
    auto it = m_MEM_TPD.find(iConfig);
    if (it == m_MEM_TPD.end()) {
        pData = new MEMDATA();
        pData->pbData = pbData;
        pData->byteCount = byteCount;
        pData->ucRefCount = 1;

        m_MEM_TPD[iConfig] = pData;
    }
}

void CMinecraftApp::RemoveMemoryTPDFile(int iConfig) {
    std::lock_guard<std::mutex> lock(csMemTPDLock);
    // check it's not already in
    PMEMDATA pData = nullptr;
    auto it = m_MEM_TPD.find(iConfig);
    if (it != m_MEM_TPD.end()) {
        pData = m_MEM_TPD[iConfig];
        delete pData;
        m_MEM_TPD.erase(iConfig);
    }
}

#if defined(_WINDOWS64)
int CMinecraftApp::GetTPConfigVal(wchar_t* pwchDataFile) { return -1; }
#endif
bool CMinecraftApp::IsFileInTPD(int iConfig) {
    bool val = false;

    { std::lock_guard<std::mutex> lock(csMemTPDLock);
    auto it = m_MEM_TPD.find(iConfig);
    if (it != m_MEM_TPD.end()) val = true;
    }

    return val;
}

void CMinecraftApp::GetTPD(int iConfig, std::uint8_t** ppbData,
                           unsigned int* pByteCount) {
    std::lock_guard<std::mutex> lock(csMemTPDLock);
    auto it = m_MEM_TPD.find(iConfig);
    if (it != m_MEM_TPD.end()) {
        PMEMDATA pData = (*it).second;
        *ppbData = pData->pbData;
        *pByteCount = pData->byteCount;
    }
}

// bool CMinecraftApp::UploadFileToGlobalStorage(int iQuadrant,
// C4JStorage::eGlobalStorage eStorageFacility, std::wstring *wsFile  )
// {
// 	bool bRes=false;
// #ifndef _CONTENT_PACKAGE
// 	// read the local file
// 	File gtsFile( wsFile->c_str() );
//
// 	int64_t fileSize = gtsFile.length();
//
// 	if(fileSize!=0)
// 	{
// 		FileInputStream fis(gtsFile);
// 		byteArray ba((int)fileSize);
// 		fis.read(ba);
// 		fis.close();
//
// 		bRes=StorageManager.WriteTMSFile(iQuadrant,eStorageFacility,(wchar_t
// *)wsFile->c_str(),ba.data, ba.length);
//
// 	}
// #endif
// 	return bRes;
// }

void CMinecraftApp::StoreLaunchData() {}

void CMinecraftApp::ExitGame() {}

// Invites

void CMinecraftApp::ProcessInvite(std::uint32_t dwUserIndex,
                                  std::uint32_t dwLocalUsersMask,
                                  const INVITE_INFO* pInviteInfo) {
    m_InviteData.dwUserIndex = dwUserIndex;
    m_InviteData.dwLocalUsersMask = dwLocalUsersMask;
    m_InviteData.pInviteInfo = pInviteInfo;
    // memcpy(&m_InviteData,pJoinData,sizeof(JoinFromInviteData));
    SetAction(dwUserIndex, eAppAction_ExitAndJoinFromInvite);
}

int CMinecraftApp::ExitAndJoinFromInvite(void* pParam, int iPad,
                                         C4JStorage::EMessageResult result) {
    CMinecraftApp* pApp = (CMinecraftApp*)pParam;
    // Minecraft *pMinecraft=Minecraft::GetInstance();

    // buttons are swapped on this menu
    if (result == C4JStorage::EMessage_ResultDecline) {
        pApp->SetAction(iPad, eAppAction_ExitAndJoinFromInviteConfirmed);
    }

    return 0;
}

int CMinecraftApp::ExitAndJoinFromInviteSaveDialogReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    CMinecraftApp* pClass = (CMinecraftApp*)pParam;
    // Exit with or without saving
    // Decline means save in this dialog
    if (result == C4JStorage::EMessage_ResultDecline ||
        result == C4JStorage::EMessage_ResultThirdOption) {
        if (result == C4JStorage::EMessage_ResultDecline)  // Save
        {
            // Check they have the full texture pack if they are using one
            // 4J-PB - Is the player trying to save but they are using a trial
            // texturepack ?
            if (!Minecraft::GetInstance()->skins->isUsingDefaultSkin()) {
                TexturePack* tPack =
                    Minecraft::GetInstance()->skins->getSelected();

                DLCPack* pDLCPack = tPack->getDLCPack();
                if (!pDLCPack->hasPurchasedFile(DLCManager::e_DLCType_Texture,
                                                L"")) {
                    // upsell
                    // get the dlc texture pack

                    unsigned int uiIDA[2];
                    uiIDA[0] = IDS_CONFIRM_OK;
                    uiIDA[1] = IDS_CONFIRM_CANCEL;

                    // Give the player a warning about the trial version of the
                    // texture pack
                    ui.RequestErrorMessage(
                        IDS_WARNING_DLC_TRIALTEXTUREPACK_TITLE,
                        IDS_WARNING_DLC_TRIALTEXTUREPACK_TEXT, uiIDA, 2, iPad,
                        &CMinecraftApp::WarningTrialTexturePackReturned,
                        pClass);

                    return S_OK;
                }
            }
            // does the save exist?
            bool bSaveExists;
            StorageManager.DoesSaveExist(&bSaveExists);
            // 4J-PB - we check if the save exists inside the libs
            // we need to ask if they are sure they want to overwrite the
            // existing game
            if (bSaveExists) {
                unsigned int uiIDA[2];
                uiIDA[0] = IDS_CONFIRM_CANCEL;
                uiIDA[1] = IDS_CONFIRM_OK;
                ui.RequestErrorMessage(
                    IDS_TITLE_SAVE_GAME, IDS_CONFIRM_SAVE_GAME, uiIDA, 2,
                    ProfileManager.GetPrimaryPad(),
                    &CMinecraftApp::ExitAndJoinFromInviteAndSaveReturned,
                    pClass);
                return 0;
            } else {
                MinecraftServer::getInstance()->setSaveOnExit(true);
            }
        } else {
            // been a few requests for a confirm on exit without saving
            unsigned int uiIDA[2];
            uiIDA[0] = IDS_CONFIRM_CANCEL;
            uiIDA[1] = IDS_CONFIRM_OK;
            ui.RequestErrorMessage(
                IDS_TITLE_DECLINE_SAVE_GAME, IDS_CONFIRM_DECLINE_SAVE_GAME,
                uiIDA, 2, ProfileManager.GetPrimaryPad(),
                &CMinecraftApp::ExitAndJoinFromInviteDeclineSaveReturned,
                pClass);
            return 0;
        }

        app.SetAction(ProfileManager.GetPrimaryPad(),
                      eAppAction_ExitAndJoinFromInviteConfirmed);
    }
    return 0;
}

int CMinecraftApp::WarningTrialTexturePackReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    // 4J Stu - I added this in when fixing an X1 bug. We should probably add
    // this as well but I don't have time to test all platforms atm

    return 0;
}

int CMinecraftApp::ExitAndJoinFromInviteAndSaveReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    // CMinecraftApp* pClass = (CMinecraftApp*)pParam;

    // results switched for this dialog
    if (result == C4JStorage::EMessage_ResultDecline) {
        int saveOrCheckpointId = 0;

        // Check they have the full texture pack if they are using one
        // 4J-PB - Is the player trying to save but they are using a trial
        // texturepack ?
        if (!Minecraft::GetInstance()->skins->isUsingDefaultSkin()) {
            TexturePack* tPack = Minecraft::GetInstance()->skins->getSelected();

            DLCPack* pDLCPack = tPack->getDLCPack();
            if (!pDLCPack->hasPurchasedFile(DLCManager::e_DLCType_Texture,
                                            L"")) {
                // upsell
                // get the dlc texture pack

                unsigned int uiIDA[2];
                uiIDA[0] = IDS_CONFIRM_OK;
                uiIDA[1] = IDS_CONFIRM_CANCEL;

                // Give the player a warning about the trial version of the
                // texture pack
                ui.RequestErrorMessage(
                    IDS_WARNING_DLC_TRIALTEXTUREPACK_TITLE,
                    IDS_WARNING_DLC_TRIALTEXTUREPACK_TEXT, uiIDA, 2, iPad,
                    &CMinecraftApp::WarningTrialTexturePackReturned, nullptr);

                return S_OK;
            }
        }
        // bool validSave =
        // StorageManager.GetSaveUniqueNumber(&saveOrCheckpointId);
        // SentientManager.RecordLevelSaveOrCheckpoint(ProfileManager.GetPrimaryPad(),
        // saveOrCheckpointId);
        MinecraftServer::getInstance()->setSaveOnExit(true);
        // flag a app action of exit and join game from invite
        app.SetAction(iPad, eAppAction_ExitAndJoinFromInviteConfirmed);
    }
    return 0;
}

int CMinecraftApp::ExitAndJoinFromInviteDeclineSaveReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    // results switched for this dialog
    if (result == C4JStorage::EMessage_ResultDecline) {
        MinecraftServer::getInstance()->setSaveOnExit(false);
        // flag a app action of exit and join game from invite
        app.SetAction(iPad, eAppAction_ExitAndJoinFromInviteConfirmed);
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//
// FatalLoadError
//
// This is called when we can't load one of the required files at startup
// It tends to mean the files have been corrupted.
// We have to assume that we've not been able to load the text for the game.
//
//////////////////////////////////////////////////////////////////////////
void CMinecraftApp::FatalLoadError() {}

TIPSTRUCT CMinecraftApp::m_GameTipA[MAX_TIPS_GAMETIP] = {
    {0, IDS_TIPS_GAMETIP_1},  {0, IDS_TIPS_GAMETIP_2},
    {0, IDS_TIPS_GAMETIP_3},  {0, IDS_TIPS_GAMETIP_4},
    {0, IDS_TIPS_GAMETIP_5},  {0, IDS_TIPS_GAMETIP_6},
    {0, IDS_TIPS_GAMETIP_7},  {0, IDS_TIPS_GAMETIP_8},
    {0, IDS_TIPS_GAMETIP_9},  {0, IDS_TIPS_GAMETIP_10},
    {0, IDS_TIPS_GAMETIP_11}, {0, IDS_TIPS_GAMETIP_12},
    {0, IDS_TIPS_GAMETIP_13}, {0, IDS_TIPS_GAMETIP_14},
    {0, IDS_TIPS_GAMETIP_15}, {0, IDS_TIPS_GAMETIP_16},
    {0, IDS_TIPS_GAMETIP_17}, {0, IDS_TIPS_GAMETIP_18},
    {0, IDS_TIPS_GAMETIP_19}, {0, IDS_TIPS_GAMETIP_20},
    {0, IDS_TIPS_GAMETIP_21}, {0, IDS_TIPS_GAMETIP_22},
    {0, IDS_TIPS_GAMETIP_23}, {0, IDS_TIPS_GAMETIP_24},
    {0, IDS_TIPS_GAMETIP_25}, {0, IDS_TIPS_GAMETIP_26},
    {0, IDS_TIPS_GAMETIP_27}, {0, IDS_TIPS_GAMETIP_28},
    {0, IDS_TIPS_GAMETIP_29}, {0, IDS_TIPS_GAMETIP_30},
    {0, IDS_TIPS_GAMETIP_31}, {0, IDS_TIPS_GAMETIP_32},
    {0, IDS_TIPS_GAMETIP_33}, {0, IDS_TIPS_GAMETIP_34},
    {0, IDS_TIPS_GAMETIP_35}, {0, IDS_TIPS_GAMETIP_36},
    {0, IDS_TIPS_GAMETIP_37}, {0, IDS_TIPS_GAMETIP_38},
    {0, IDS_TIPS_GAMETIP_39}, {0, IDS_TIPS_GAMETIP_40},
    {0, IDS_TIPS_GAMETIP_41}, {0, IDS_TIPS_GAMETIP_42},
    {0, IDS_TIPS_GAMETIP_43}, {0, IDS_TIPS_GAMETIP_44},
    {0, IDS_TIPS_GAMETIP_45}, {0, IDS_TIPS_GAMETIP_46},
    {0, IDS_TIPS_GAMETIP_47}, {0, IDS_TIPS_GAMETIP_48},
    {0, IDS_TIPS_GAMETIP_49}, {0, IDS_TIPS_GAMETIP_50},
};

TIPSTRUCT CMinecraftApp::m_TriviaTipA[MAX_TIPS_TRIVIATIP] = {
    {0, IDS_TIPS_TRIVIA_1},  {0, IDS_TIPS_TRIVIA_2},  {0, IDS_TIPS_TRIVIA_3},
    {0, IDS_TIPS_TRIVIA_4},  {0, IDS_TIPS_TRIVIA_5},  {0, IDS_TIPS_TRIVIA_6},
    {0, IDS_TIPS_TRIVIA_7},  {0, IDS_TIPS_TRIVIA_8},  {0, IDS_TIPS_TRIVIA_9},
    {0, IDS_TIPS_TRIVIA_10}, {0, IDS_TIPS_TRIVIA_11}, {0, IDS_TIPS_TRIVIA_12},
    {0, IDS_TIPS_TRIVIA_13}, {0, IDS_TIPS_TRIVIA_14}, {0, IDS_TIPS_TRIVIA_15},
    {0, IDS_TIPS_TRIVIA_16}, {0, IDS_TIPS_TRIVIA_17}, {0, IDS_TIPS_TRIVIA_18},
    {0, IDS_TIPS_TRIVIA_19}, {0, IDS_TIPS_TRIVIA_20},
};

Random* CMinecraftApp::TipRandom = new Random();

int CMinecraftApp::TipsSortFunction(const void* a, const void* b) {
    // 4jcraft, scince the sortvalues can be negative, i changed it
    // to a three way comparison,
    // scince subtracting of signed integers can cause overflow.

    int s1 = ((TIPSTRUCT*)a)->iSortValue;
    int s2 = ((TIPSTRUCT*)b)->iSortValue;

    if (s1 > s2) {
        return 1;

    } else if (s1 == s2) {
        return 0;
    }

    return -1;
}

void CMinecraftApp::InitialiseTips() {
    // We'll randomise the tips at start up based on their priority

    ZeroMemory(m_TipIDA, sizeof(m_TipIDA));

    // Make the first tip tell you that you can play splitscreen in HD modes if
    // you are in SD
    if (!RenderManager.IsHiDef()) {
        m_GameTipA[0].uiStringID = IDS_TIPS_GAMETIP_0;
    }
    // randomise then quicksort
    // going to leave the multiplayer tip so it is always first

    // Only randomise the content package build
#if defined(_CONTENT_PACKAGE)

    for (int i = 1; i < MAX_TIPS_GAMETIP; i++) {
        m_GameTipA[i].iSortValue = TipRandom->nextInt();
    }
    qsort(&m_GameTipA[1], MAX_TIPS_GAMETIP - 1, sizeof(TIPSTRUCT),
          TipsSortFunction);
#endif

    for (int i = 0; i < MAX_TIPS_TRIVIATIP; i++) {
        m_TriviaTipA[i].iSortValue = TipRandom->nextInt();
    }
    qsort(m_TriviaTipA, MAX_TIPS_TRIVIATIP, sizeof(TIPSTRUCT),
          TipsSortFunction);

    int iCurrentGameTip = 0;
    int iCurrentTriviaTip = 0;

    for (int i = 0; i < MAX_TIPS_GAMETIP + MAX_TIPS_TRIVIATIP; i++) {
        // Add a trivia one every third tip (if there are any left)
        if ((i % 3 == 2) && (iCurrentTriviaTip < MAX_TIPS_TRIVIATIP)) {
            // Add a trivia one
            m_TipIDA[i] = m_TriviaTipA[iCurrentTriviaTip++].uiStringID;
        } else {
            if (iCurrentGameTip < MAX_TIPS_GAMETIP) {
                // Add a gametip
                m_TipIDA[i] = m_GameTipA[iCurrentGameTip++].uiStringID;
            } else {
                // Add a trivia one
                m_TipIDA[i] = m_TriviaTipA[iCurrentTriviaTip++].uiStringID;
            }
        }

        if (m_TipIDA[i] == 0) {
            // the m_TriviaTipA or the m_GameTipA are out of sync
#if !defined(_CONTENT_PACKAGE)
            __debugbreak();
#endif
        }
    }

    m_uiCurrentTip = 0;
}

int CMinecraftApp::GetNextTip() {
    static bool bShowSkinDLCTip = true;
    // don't display the DLC tip in the trial game
    if (ProfileManager.IsFullVersion() && app.GetNewDLCAvailable() &&
        app.DisplayNewDLCTip()) {
        return IDS_TIPS_GAMETIP_NEWDLC;
    } else {
        if (bShowSkinDLCTip && ProfileManager.IsFullVersion()) {
            bShowSkinDLCTip = false;
            if (app.DLCInstallProcessCompleted()) {
                if (app.m_dlcManager.getPackCount(DLCManager::e_DLCType_Skin) ==
                    0) {
                    return IDS_TIPS_GAMETIP_SKINPACKS;
                }
            } else {
                return IDS_TIPS_GAMETIP_SKINPACKS;
            }
        }
    }

    if (m_uiCurrentTip == MAX_TIPS_GAMETIP + MAX_TIPS_TRIVIATIP)
        m_uiCurrentTip = 0;

    return m_TipIDA[m_uiCurrentTip++];
}

int CMinecraftApp::GetHTMLColour(eMinecraftColour colour) {
    Minecraft* pMinecraft = Minecraft::GetInstance();
    return pMinecraft->skins->getSelected()->getColourTable()->getColour(
        colour);
}

int CMinecraftApp::GetHTMLFontSize(EHTMLFontSize size) {
    return s_iHTMLFontSizesA[size];
}

std::wstring CMinecraftApp::FormatHTMLString(
    int iPad, const std::wstring& desc, int shadowColour /*= 0xFFFFFFFF*/) {
    std::wstring text(desc);

    wchar_t replacements[64];
    // We will also insert line breaks here as couldn't figure out how to get
    // them to come through from strings.resx !
    text = replaceAll(text, L"{*B*}", L"<br />");
    swprintf(replacements, 64, L"<font color=\"#%08x\">",
             GetHTMLColour(eHTMLColor_T1));
    text = replaceAll(text, L"{*T1*}", replacements);
    swprintf(replacements, 64, L"<font color=\"#%08x\">",
             GetHTMLColour(eHTMLColor_T2));
    text = replaceAll(text, L"{*T2*}", replacements);
    swprintf(replacements, 64, L"<font color=\"#%08x\">",
             GetHTMLColour(eHTMLColor_T3));
    text = replaceAll(text, L"{*T3*}", replacements);  // for How To Play
    swprintf(replacements, 64, L"<font color=\"#%08x\">",
             GetHTMLColour(eHTMLColor_Black));
    text = replaceAll(text, L"{*ETB*}", replacements);
    swprintf(replacements, 64, L"<font color=\"#%08x\">",
             GetHTMLColour(eHTMLColor_White));
    text = replaceAll(text, L"{*ETW*}", replacements);
    text = replaceAll(text, L"{*EF*}", L"</font>");

    swprintf(replacements, 64, L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eHTMLColor_0), shadowColour);
    text = replaceAll(text, L"{*C0*}", replacements);
    swprintf(replacements, 64, L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eHTMLColor_1), shadowColour);
    text = replaceAll(text, L"{*C1*}", replacements);
    swprintf(replacements, 64, L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eHTMLColor_2), shadowColour);
    text = replaceAll(text, L"{*C2*}", replacements);
    swprintf(replacements, 64, L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eHTMLColor_3), shadowColour);
    text = replaceAll(text, L"{*C3*}", replacements);
    swprintf(replacements, 64, L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eHTMLColor_4), shadowColour);
    text = replaceAll(text, L"{*C4*}", replacements);
    swprintf(replacements, 64, L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eHTMLColor_5), shadowColour);
    text = replaceAll(text, L"{*C5*}", replacements);
    swprintf(replacements, 64, L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eHTMLColor_6), shadowColour);
    text = replaceAll(text, L"{*C6*}", replacements);
    swprintf(replacements, 64, L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eHTMLColor_7), shadowColour);
    text = replaceAll(text, L"{*C7*}", replacements);
    swprintf(replacements, 64, L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eHTMLColor_8), shadowColour);
    text = replaceAll(text, L"{*C8*}", replacements);
    swprintf(replacements, 64, L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eHTMLColor_9), shadowColour);
    text = replaceAll(text, L"{*C9*}", replacements);
    swprintf(replacements, 64, L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eHTMLColor_a), shadowColour);
    text = replaceAll(text, L"{*CA*}", replacements);
    swprintf(replacements, 64, L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eHTMLColor_b), shadowColour);
    text = replaceAll(text, L"{*CB*}", replacements);
    swprintf(replacements, 64, L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eHTMLColor_c), shadowColour);
    text = replaceAll(text, L"{*CC*}", replacements);
    swprintf(replacements, 64, L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eHTMLColor_d), shadowColour);
    text = replaceAll(text, L"{*CD*}", replacements);
    swprintf(replacements, 64, L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eHTMLColor_e), shadowColour);
    text = replaceAll(text, L"{*CE*}", replacements);
    swprintf(replacements, 64, L"<font color=\"#%08x\" shadowcolor=\"#%08x\">",
             GetHTMLColour(eHTMLColor_f), shadowColour);
    text = replaceAll(text, L"{*CF*}", replacements);

    // Swap for southpaw.
    if (app.GetGameSettings(iPad, eGameSetting_ControlSouthPaw)) {
        text =
            replaceAll(text, L"{*CONTROLLER_ACTION_MOVE*}",
                       GetActionReplacement(iPad, MINECRAFT_ACTION_LOOK_RIGHT));
        text = replaceAll(text, L"{*CONTROLLER_ACTION_LOOK*}",
                          GetActionReplacement(iPad, MINECRAFT_ACTION_RIGHT));

        text = replaceAll(text, L"{*CONTROLLER_MENU_NAVIGATE*}",
                          GetVKReplacement(VK_PAD_RTHUMB_LEFT));
    } else  // Normal right handed.
    {
        text = replaceAll(text, L"{*CONTROLLER_ACTION_MOVE*}",
                          GetActionReplacement(iPad, MINECRAFT_ACTION_RIGHT));
        text =
            replaceAll(text, L"{*CONTROLLER_ACTION_LOOK*}",
                       GetActionReplacement(iPad, MINECRAFT_ACTION_LOOK_RIGHT));

        text = replaceAll(text, L"{*CONTROLLER_MENU_NAVIGATE*}",
                          GetVKReplacement(VK_PAD_LTHUMB_LEFT));
    }

    text = replaceAll(text, L"{*CONTROLLER_ACTION_JUMP*}",
                      GetActionReplacement(iPad, MINECRAFT_ACTION_JUMP));
    text =
        replaceAll(text, L"{*CONTROLLER_ACTION_SNEAK*}",
                   GetActionReplacement(iPad, MINECRAFT_ACTION_SNEAK_TOGGLE));
    text = replaceAll(text, L"{*CONTROLLER_ACTION_USE*}",
                      GetActionReplacement(iPad, MINECRAFT_ACTION_USE));
    text = replaceAll(text, L"{*CONTROLLER_ACTION_ACTION*}",
                      GetActionReplacement(iPad, MINECRAFT_ACTION_ACTION));
    text = replaceAll(text, L"{*CONTROLLER_ACTION_LEFT_SCROLL*}",
                      GetActionReplacement(iPad, MINECRAFT_ACTION_LEFT_SCROLL));
    text =
        replaceAll(text, L"{*CONTROLLER_ACTION_RIGHT_SCROLL*}",
                   GetActionReplacement(iPad, MINECRAFT_ACTION_RIGHT_SCROLL));
    text = replaceAll(text, L"{*CONTROLLER_ACTION_INVENTORY*}",
                      GetActionReplacement(iPad, MINECRAFT_ACTION_INVENTORY));
    text = replaceAll(text, L"{*CONTROLLER_ACTION_CRAFTING*}",
                      GetActionReplacement(iPad, MINECRAFT_ACTION_CRAFTING));
    text = replaceAll(text, L"{*CONTROLLER_ACTION_DROP*}",
                      GetActionReplacement(iPad, MINECRAFT_ACTION_DROP));
    text = replaceAll(
        text, L"{*CONTROLLER_ACTION_CAMERA*}",
        GetActionReplacement(iPad, MINECRAFT_ACTION_RENDER_THIRD_PERSON));
    text = replaceAll(text, L"{*CONTROLLER_ACTION_MENU_PAGEDOWN*}",
                      GetActionReplacement(iPad, ACTION_MENU_PAGEDOWN));
    text =
        replaceAll(text, L"{*CONTROLLER_ACTION_DISMOUNT*}",
                   GetActionReplacement(iPad, MINECRAFT_ACTION_SNEAK_TOGGLE));
    text = replaceAll(text, L"{*CONTROLLER_VK_A*}", GetVKReplacement(VK_PAD_A));
    text = replaceAll(text, L"{*CONTROLLER_VK_B*}", GetVKReplacement(VK_PAD_B));
    text = replaceAll(text, L"{*CONTROLLER_VK_X*}", GetVKReplacement(VK_PAD_X));
    text = replaceAll(text, L"{*CONTROLLER_VK_Y*}", GetVKReplacement(VK_PAD_Y));
    text = replaceAll(text, L"{*CONTROLLER_VK_LB*}",
                      GetVKReplacement(VK_PAD_LSHOULDER));
    text = replaceAll(text, L"{*CONTROLLER_VK_RB*}",
                      GetVKReplacement(VK_PAD_RSHOULDER));
    text = replaceAll(text, L"{*CONTROLLER_VK_LS*}",
                      GetVKReplacement(VK_PAD_LTHUMB_UP));
    text = replaceAll(text, L"{*CONTROLLER_VK_RS*}",
                      GetVKReplacement(VK_PAD_RTHUMB_UP));
    text = replaceAll(text, L"{*CONTROLLER_VK_LT*}",
                      GetVKReplacement(VK_PAD_LTRIGGER));
    text = replaceAll(text, L"{*CONTROLLER_VK_RT*}",
                      GetVKReplacement(VK_PAD_RTRIGGER));
    text = replaceAll(text, L"{*ICON_SHANK_01*}",
                      GetIconReplacement(XZP_ICON_SHANK_01));
    text = replaceAll(text, L"{*ICON_SHANK_03*}",
                      GetIconReplacement(XZP_ICON_SHANK_03));
    text = replaceAll(text, L"{*CONTROLLER_ACTION_DPAD_UP*}",
                      GetActionReplacement(iPad, MINECRAFT_ACTION_DPAD_UP));
    text = replaceAll(text, L"{*CONTROLLER_ACTION_DPAD_DOWN*}",
                      GetActionReplacement(iPad, MINECRAFT_ACTION_DPAD_DOWN));
    text = replaceAll(text, L"{*CONTROLLER_ACTION_DPAD_RIGHT*}",
                      GetActionReplacement(iPad, MINECRAFT_ACTION_DPAD_RIGHT));
    text = replaceAll(text, L"{*CONTROLLER_ACTION_DPAD_LEFT*}",
                      GetActionReplacement(iPad, MINECRAFT_ACTION_DPAD_LEFT));

    // Fix for #8903 - UI: Localization: KOR/JPN/CHT: Button Icons are rendered
    // with padding space, which looks no good
    std::uint32_t dwLanguage = XGetLanguage();
    switch (dwLanguage) {
        case XC_LANGUAGE_KOREAN:
        case XC_LANGUAGE_JAPANESE:
        case XC_LANGUAGE_TCHINESE:
            text = replaceAll(text, L"&nbsp;", L"");
            break;
    }

    return text;
}

std::wstring CMinecraftApp::GetActionReplacement(int iPad,
                                                 unsigned char ucAction) {
    unsigned int input = InputManager.GetGameJoypadMaps(
        InputManager.GetJoypadMapVal(iPad), ucAction);

    std::wstring replacement = L"";

    // 4J Stu - Some of our actions can be mapped to multiple physical buttons,
    // so replaces the switch that was here
    if (input & _360_JOY_BUTTON_A)
        replacement = L"ButtonA";
    else if (input & _360_JOY_BUTTON_B)
        replacement = L"ButtonB";
    else if (input & _360_JOY_BUTTON_X)
        replacement = L"ButtonX";
    else if (input & _360_JOY_BUTTON_Y)
        replacement = L"ButtonY";
    else if ((input & _360_JOY_BUTTON_LSTICK_UP) ||
             (input & _360_JOY_BUTTON_LSTICK_DOWN) ||
             (input & _360_JOY_BUTTON_LSTICK_LEFT) ||
             (input & _360_JOY_BUTTON_LSTICK_RIGHT)) {
        replacement = L"ButtonLeftStick";
    } else if ((input & _360_JOY_BUTTON_RSTICK_LEFT) ||
               (input & _360_JOY_BUTTON_RSTICK_RIGHT) ||
               (input & _360_JOY_BUTTON_RSTICK_UP) ||
               (input & _360_JOY_BUTTON_RSTICK_DOWN)) {
        replacement = L"ButtonRightStick";
    } else if (input & _360_JOY_BUTTON_DPAD_LEFT)
        replacement = L"ButtonDpadL";
    else if (input & _360_JOY_BUTTON_DPAD_RIGHT)
        replacement = L"ButtonDpadR";
    else if (input & _360_JOY_BUTTON_DPAD_UP)
        replacement = L"ButtonDpadU";
    else if (input & _360_JOY_BUTTON_DPAD_DOWN)
        replacement = L"ButtonDpadD";
    else if (input & _360_JOY_BUTTON_LT)
        replacement = L"ButtonLeftTrigger";
    else if (input & _360_JOY_BUTTON_RT)
        replacement = L"ButtonRightTrigger";
    else if (input & _360_JOY_BUTTON_RB)
        replacement = L"ButtonRightBumper";
    else if (input & _360_JOY_BUTTON_LB)
        replacement = L"ButtonLeftBumper";
    else if (input & _360_JOY_BUTTON_BACK)
        replacement = L"ButtonBack";
    else if (input & _360_JOY_BUTTON_START)
        replacement = L"ButtonStart";
    else if (input & _360_JOY_BUTTON_RTHUMB)
        replacement = L"ButtonRS";
    else if (input & _360_JOY_BUTTON_LTHUMB)
        replacement = L"ButtonLS";

    wchar_t string[128];

#if defined(_WIN64)
    int size = 45;
    if (ui.getScreenWidth() < 1920) size = 30;
#else
    int size = 45;
#endif

    swprintf(string, 128,
             L"<img src=\"%ls\" align=\"middle\" height=\"%d\" width=\"%d\"/>",
             replacement.c_str(), size, size);

    return string;
}

std::wstring CMinecraftApp::GetVKReplacement(unsigned int uiVKey) {
    std::wstring replacement = L"";
    switch (uiVKey) {
        case VK_PAD_A:
            replacement = L"ButtonA";
            break;
        case VK_PAD_B:
            replacement = L"ButtonB";
            break;
        case VK_PAD_X:
            replacement = L"ButtonX";
            break;
        case VK_PAD_Y:
            replacement = L"ButtonY";
            break;
        case VK_PAD_LSHOULDER:
            replacement = L"ButtonLeftBumper";
            break;
        case VK_PAD_RSHOULDER:
            replacement = L"ButtonRightBumper";
            break;
        case VK_PAD_LTRIGGER:
            replacement = L"ButtonLeftTrigger";
            break;
        case VK_PAD_RTRIGGER:
            replacement = L"ButtonRightTrigger";
            break;
        case VK_PAD_LTHUMB_UP:
        case VK_PAD_LTHUMB_DOWN:
        case VK_PAD_LTHUMB_RIGHT:
        case VK_PAD_LTHUMB_LEFT:
        case VK_PAD_LTHUMB_UPLEFT:
        case VK_PAD_LTHUMB_UPRIGHT:
        case VK_PAD_LTHUMB_DOWNRIGHT:
        case VK_PAD_LTHUMB_DOWNLEFT:
            replacement = L"ButtonLeftStick";
            break;
        case VK_PAD_RTHUMB_UP:
        case VK_PAD_RTHUMB_DOWN:
        case VK_PAD_RTHUMB_RIGHT:
        case VK_PAD_RTHUMB_LEFT:
        case VK_PAD_RTHUMB_UPLEFT:
        case VK_PAD_RTHUMB_UPRIGHT:
        case VK_PAD_RTHUMB_DOWNRIGHT:
        case VK_PAD_RTHUMB_DOWNLEFT:
            replacement = L"ButtonRightStick";
            break;
        default:
            break;
    }
    wchar_t string[128];

#if defined(_WIN64)
    int size = 45;
    if (ui.getScreenWidth() < 1920) size = 30;
#else
    int size = 45;
#endif

    swprintf(string, 128,
             L"<img src=\"%ls\" align=\"middle\" height=\"%d\" width=\"%d\"/>",
             replacement.c_str(), size, size);

    return string;
}

std::wstring CMinecraftApp::GetIconReplacement(unsigned int uiIcon) {
    wchar_t string[128];

#if defined(_WIN64)
    int size = 33;
    if (ui.getScreenWidth() < 1920) size = 22;
#else
    int size = 33;
#endif

    swprintf(string, 128,
             L"<img src=\"Icon_Shank\" align=\"middle\" height=\"%d\" "
             L"width=\"%d\"/>",
             size, size);
    std::wstring result = L"";
    switch (uiIcon) {
        case XZP_ICON_SHANK_01:
            result = string;
            break;
        case XZP_ICON_SHANK_03:
            result.append(string).append(string).append(string);
            break;
        default:
            break;
    }
    return result;
}

std::unordered_map<PlayerUID, MOJANG_DATA*> CMinecraftApp::MojangData;
std::unordered_map<int, uint64_t> CMinecraftApp::DLCTextures_PackID;
std::unordered_map<uint64_t, DLC_INFO*> CMinecraftApp::DLCInfo_Trial;
std::unordered_map<uint64_t, DLC_INFO*> CMinecraftApp::DLCInfo_Full;
std::unordered_map<std::wstring, uint64_t> CMinecraftApp::DLCInfo_SkinName;

int32_t CMinecraftApp::RegisterMojangData(wchar_t* pXuidName, PlayerUID xuid,
                                          wchar_t* pSkin, wchar_t* pCape) {
    int32_t hr = S_OK;
    eXUID eTempXuid = eXUID_Undefined;
    MOJANG_DATA* pMojangData = nullptr;

    // ignore the names if we don't recognize them
    if (pXuidName != nullptr) {
        if (wcscmp(pXuidName, L"XUID_NOTCH") == 0) {
            eTempXuid =
                eXUID_Notch;  // might be needed for the apple at some point
        } else if (wcscmp(pXuidName, L"XUID_DEADMAU5") == 0) {
            eTempXuid = eXUID_Deadmau5;  // Needed for the deadmau5 ears
        } else {
            eTempXuid = eXUID_NoName;
        }
    }

    if (eTempXuid != eXUID_Undefined) {
        pMojangData = new MOJANG_DATA;
        ZeroMemory(pMojangData, sizeof(MOJANG_DATA));
        pMojangData->eXuid = eTempXuid;

        wcsncpy(pMojangData->wchSkin, pSkin, MAX_CAPENAME_SIZE);
        wcsncpy(pMojangData->wchCape, pCape, MAX_CAPENAME_SIZE);
        MojangData[xuid] = pMojangData;
    }

    return hr;
}

MOJANG_DATA* CMinecraftApp::GetMojangDataForXuid(PlayerUID xuid) {
    return MojangData[xuid];
}

int32_t CMinecraftApp::RegisterConfigValues(wchar_t* pType, int iValue) {
    int32_t hr = S_OK;

    // #ifdef 0
    // 	if(pType!=nullptr)
    // 	{
    // 		if(wcscmp(pType,L"XboxOneTransfer")==0)
    // 		{
    // 			if(iValue>0)
    // 			{
    // 				app.m_bTransferSavesToXboxOne=true;
    // 			}
    // 			else
    // 			{
    // 				app.m_bTransferSavesToXboxOne=false;
    // 			}
    // 		}
    // 		else if(wcscmp(pType,L"TransferSlotCount")==0)
    // 		{
    // 			app.m_uiTransferSlotC=iValue;
    // 		}
    //
    // 	}
    // #endif

    return hr;
}

#if defined(_WINDOWS64)
int32_t CMinecraftApp::RegisterDLCData(wchar_t* pType, wchar_t* pBannerName,
                                       int iGender, uint64_t ullOfferID_Full,
                                       uint64_t ullOfferID_Trial,
                                       wchar_t* pFirstSkin,
                                       unsigned int uiSortIndex, int iConfig,
                                       wchar_t* pDataFile) {
    int32_t hr = S_OK;
    DLC_INFO* pDLCData = new DLC_INFO;
    ZeroMemory(pDLCData, sizeof(DLC_INFO));
    pDLCData->ullOfferID_Full = ullOfferID_Full;
    pDLCData->ullOfferID_Trial = ullOfferID_Trial;
    pDLCData->eDLCType = e_DLC_NotDefined;
    pDLCData->iGender = iGender;
    pDLCData->uiSortIndex = uiSortIndex;
    pDLCData->iConfig = iConfig;

    // ignore the names if we don't recognize them
    if (pBannerName != L"") {
        wcsncpy_s(pDLCData->wchBanner, pBannerName, MAX_BANNERNAME_SIZE);
    }

    if (pDataFile[0] != 0) {
        wcsncpy_s(pDLCData->wchDataFile, pDataFile, MAX_BANNERNAME_SIZE);
    }

    if (pType != nullptr) {
        if (wcscmp(pType, L"Skin") == 0) {
            pDLCData->eDLCType = e_DLC_SkinPack;
        } else if (wcscmp(pType, L"Gamerpic") == 0) {
            pDLCData->eDLCType = e_DLC_Gamerpics;
        } else if (wcscmp(pType, L"Theme") == 0) {
            pDLCData->eDLCType = e_DLC_Themes;
        } else if (wcscmp(pType, L"Avatar") == 0) {
            pDLCData->eDLCType = e_DLC_AvatarItems;
        } else if (wcscmp(pType, L"MashUpPack") == 0) {
            pDLCData->eDLCType = e_DLC_MashupPacks;
            DLCTextures_PackID[pDLCData->iConfig] = ullOfferID_Full;
        } else if (wcscmp(pType, L"TexturePack") == 0) {
            pDLCData->eDLCType = e_DLC_TexturePacks;
            DLCTextures_PackID[pDLCData->iConfig] = ullOfferID_Full;
        }
    }

    if (ullOfferID_Trial != 0ll) DLCInfo_Trial[ullOfferID_Trial] = pDLCData;
    if (ullOfferID_Full != 0ll) DLCInfo_Full[ullOfferID_Full] = pDLCData;
    if (pFirstSkin[0] != 0) DLCInfo_SkinName[pFirstSkin] = ullOfferID_Full;

    return hr;
}
#elif defined(__linux__)
int32_t CMinecraftApp::RegisterDLCData(wchar_t* pType, wchar_t* pBannerName,
                                       int iGender, uint64_t ullOfferID_Full,
                                       uint64_t ullOfferID_Trial,
                                       wchar_t* pFirstSkin,
                                       unsigned int uiSortIndex, int iConfig,
                                       wchar_t* pDataFile) {
    fprintf(stderr,
            "warning: CMinecraftApp::RegisterDLCData unimplemented for "
            "platform `__linux__`\n");
    return S_OK;
}
#else

int32_t CMinecraftApp::RegisterDLCData(char* pchDLCName,
                                       unsigned int uiSortIndex,
                                       char* pchImageURL) {
    // on PS3 we get all the required info from the name
    char chDLCType[3];
    int32_t hr = S_OK;
    DLC_INFO* pDLCData = new DLC_INFO;
    ZeroMemory(pDLCData, sizeof(DLC_INFO));

    chDLCType[0] = pchDLCName[0];
    chDLCType[1] = pchDLCName[1];
    chDLCType[2] = 0;

    pDLCData->iConfig = app.GetiConfigFromName(pchDLCName);
    pDLCData->uiSortIndex = uiSortIndex;
    pDLCData->eDLCType = app.GetDLCTypeFromName(pchDLCName);
    strcpy(pDLCData->chImageURL, pchImageURL);
    // bool bIsTrialDLC = app.GetTrialFromName(pchDLCName);

    switch (pDLCData->eDLCType) {
        case e_DLC_TexturePacks: {
            char* pchName = (char*)malloc(strlen(pchDLCName) + 1);
            strcpy(pchName, pchDLCName);
            DLCTextures_PackID[pDLCData->iConfig] = pchName;
        } break;
        case e_DLC_MashupPacks: {
            char* pchName = (char*)malloc(strlen(pchDLCName) + 1);
            strcpy(pchName, pchDLCName);
            DLCTextures_PackID[pDLCData->iConfig] = pchName;
        } break;
        default:
            break;
    }

    app.DebugPrintf(5, "Adding DLC - %s\n", pchDLCName);
    DLCInfo[pchDLCName] = pDLCData;

    // 	if(ullOfferID_Trial!=0ll) DLCInfo_Trial[ullOfferID_Trial]=pDLCData;
    // 	if(ullOfferID_Full!=0ll) DLCInfo_Full[ullOfferID_Full]=pDLCData;
    // 	if(pFirstSkin[0]!=0) DLCInfo_SkinName[pFirstSkin]=ullOfferID_Full;

    //	DLCInfo[ullOfferID_Trial]=pDLCData;

    return hr;
}
#endif

bool CMinecraftApp::GetDLCFullOfferIDForSkinID(const std::wstring& FirstSkin,
                                               uint64_t* pullVal) {
    auto it = DLCInfo_SkinName.find(FirstSkin);
    if (it == DLCInfo_SkinName.end()) {
        return false;
    } else {
        *pullVal = (uint64_t)it->second;
        return true;
    }
}
bool CMinecraftApp::GetDLCFullOfferIDForPackID(const int iPackID,
                                               uint64_t* pullVal) {
    auto it = DLCTextures_PackID.find(iPackID);
    if (it == DLCTextures_PackID.end()) {
        *pullVal = (uint64_t)0;
        return false;
    } else {
        *pullVal = (uint64_t)it->second;
        return true;
    }
}
DLC_INFO* CMinecraftApp::GetDLCInfoForTrialOfferID(uint64_t ullOfferID_Trial) {
    // DLC_INFO *pDLCInfo=NULL;
    if (DLCInfo_Trial.size() > 0) {
        auto it = DLCInfo_Trial.find(ullOfferID_Trial);

        if (it == DLCInfo_Trial.end()) {
            // nothing for this
            return nullptr;
        } else {
            return it->second;
        }
    } else
        return nullptr;
}

DLC_INFO* CMinecraftApp::GetDLCInfoTrialOffer(int iIndex) {
    std::unordered_map<uint64_t, DLC_INFO*>::iterator it =
        DLCInfo_Trial.begin();

    for (int i = 0; i < iIndex; i++) {
        ++it;
    }

    return it->second;
}
DLC_INFO* CMinecraftApp::GetDLCInfoFullOffer(int iIndex) {
    std::unordered_map<uint64_t, DLC_INFO*>::iterator it = DLCInfo_Full.begin();

    for (int i = 0; i < iIndex; i++) {
        ++it;
    }

    return it->second;
}
uint64_t CMinecraftApp::GetDLCInfoTexturesFullOffer(int iIndex) {
    std::unordered_map<int, uint64_t>::iterator it = DLCTextures_PackID.begin();

    for (int i = 0; i < iIndex; i++) {
        ++it;
    }

    return it->second;
}

DLC_INFO* CMinecraftApp::GetDLCInfoForFullOfferID(uint64_t ullOfferID_Full) {
    if (DLCInfo_Full.size() > 0) {
        auto it = DLCInfo_Full.find(ullOfferID_Full);

        if (it == DLCInfo_Full.end()) {
            // nothing for this
            return nullptr;
        } else {
            return it->second;
        }
    } else
        return nullptr;
}

void CMinecraftApp::EnterSaveNotificationSection() {
    std::lock_guard<std::mutex> lock(m_saveNotificationCriticalSection);
    if (m_saveNotificationDepth++ == 0) {
        if (g_NetworkManager
                .IsInSession())  // this can be triggered from the front end if
                                 // we're downloading a save
        {
            MinecraftServer::getInstance()->broadcastStartSavingPacket();

            if (g_NetworkManager.IsLocalGame() &&
                g_NetworkManager.GetPlayerCount() == 1) {
                app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),
                                       eXuiServerAction_PauseServer,
                                       (void*)true);
            }
        }
    }
}

void CMinecraftApp::LeaveSaveNotificationSection() {
    std::lock_guard<std::mutex> lock(m_saveNotificationCriticalSection);
    if (--m_saveNotificationDepth == 0) {
        if (g_NetworkManager
                .IsInSession())  // this can be triggered from the front end if
                                 // we're downloading a save
        {
            MinecraftServer::getInstance()->broadcastStopSavingPacket();

            if (g_NetworkManager.IsLocalGame() &&
                g_NetworkManager.GetPlayerCount() == 1) {
                app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),
                                       eXuiServerAction_PauseServer,
                                       (void*)false);
            }
        }
    }
}

int CMinecraftApp::RemoteSaveThreadProc(void* lpParameter) {
    // The game should be stopped while we are doing this, but the connections
    // ticks may try to create some AABB's or Vec3's
    Compression::UseDefaultThreadStorage();

    // 4J-PB - Xbox 360 - 163153 - [CRASH] TU17: Code: Multiplayer: During the
    // Autosave in an online Multiplayer session, the game occasionally crashes
    // for one or more Clients callstack - >	if(tls->tileId != this->id)
    // updateDefaultShape(); callstack - >
    // default.exe!WaterlilyTile::getAABB(Level * level, int x, int y, int z)
    // line 38 + 8 bytes	C++
    // ...
    //  	default.exe!CMinecraftApp::RemoteSaveThreadProc(void *
    //  lpParameter)  line 6694	C++
    // host autosave, and the clients can crash on receiving handleMoveEntity
    // when it's a tile within this thread, so need to do the tls for tiles
    Tile::CreateNewThreadStorage();

    Minecraft* pMinecraft = Minecraft::GetInstance();

    pMinecraft->progressRenderer->progressStartNoAbort(
        IDS_PROGRESS_HOST_SAVING);
    pMinecraft->progressRenderer->progressStage(-1);
    pMinecraft->progressRenderer->progressStagePercentage(0);

    while (!app.GetGameStarted() &&
           app.GetXuiAction(ProfileManager.GetPrimaryPad()) ==
               eAppAction_WaitRemoteServerSaveComplete) {
        // Tick all the games connections
        pMinecraft->tickAllConnections();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (app.GetXuiAction(ProfileManager.GetPrimaryPad()) !=
        eAppAction_WaitRemoteServerSaveComplete) {
        // Something cancelled us?
        return ERROR_CANCELLED;
    }
    app.SetAction(ProfileManager.GetPrimaryPad(), eAppAction_Idle);

    ui.UpdatePlayerBasePositions();

    Tile::ReleaseThreadStorage();

    return S_OK;
}

void CMinecraftApp::ExitGameFromRemoteSave(void* lpParameter) {
    int primaryPad = ProfileManager.GetPrimaryPad();

    unsigned int uiIDA[3];
    uiIDA[0] = IDS_CONFIRM_CANCEL;
    uiIDA[1] = IDS_CONFIRM_OK;

    ui.RequestAlertMessage(
        IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 2, primaryPad,
        &CMinecraftApp::ExitGameFromRemoteSaveDialogReturned, nullptr);
}

int CMinecraftApp::ExitGameFromRemoteSaveDialogReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    // CScene_Pause* pClass = (CScene_Pause*)pParam;

    // results switched for this dialog
    if (result == C4JStorage::EMessage_ResultDecline) {
        app.SetAction(iPad, eAppAction_ExitWorld);
    } else {
        // Inform fullscreen progress scene that it's not being cancelled after
        // all
        UIScene_FullscreenProgress* pScene =
            (UIScene_FullscreenProgress*)ui.FindScene(
                eUIScene_FullscreenProgress);
        if (pScene != nullptr) {
            pScene->SetWasCancelled(false);
        }
    }
    return 0;
}

void CMinecraftApp::SetSpecialTutorialCompletionFlag(int iPad, int index) {
    if (index >= 0 && index < 32 && GameSettingsA[iPad] != nullptr) {
        GameSettingsA[iPad]->uiSpecialTutorialBitmask |= (1 << index);
    }
}

// BANNED LIST FUNCTIONS

void CMinecraftApp::SetUniqueMapName(char* pszUniqueMapName) {
    memcpy(m_pszUniqueMapName, pszUniqueMapName, 14);
}

char* CMinecraftApp::GetUniqueMapName(void) { return m_pszUniqueMapName; }

void CMinecraftApp::InvalidateBannedList(int iPad) {
    if (m_bRead_BannedListA[iPad] == true) {
        m_bRead_BannedListA[iPad] = false;
        SetBanListCheck(iPad, false);
        m_vBannedListA[iPad]->clear();

        if (BannedListA[iPad].pBannedList) {
            delete[] BannedListA[iPad].pBannedList;
            BannedListA[iPad].pBannedList = nullptr;
        }
    }
}

void CMinecraftApp::AddLevelToBannedLevelList(int iPad, PlayerUID xuid,
                                              char* pszLevelName,
                                              bool bWriteToTMS) {
    // we will have retrieved the banned level list from TMS, so add this one to
    // it and write it back to TMS

    BANNEDLISTDATA* pBannedListData = new BANNEDLISTDATA;
    memset(pBannedListData, 0, sizeof(BANNEDLISTDATA));

    memcpy(&pBannedListData->xuid, &xuid, sizeof(PlayerUID));
    strcpy(pBannedListData->pszLevelName, pszLevelName);
    m_vBannedListA[iPad]->push_back(pBannedListData);

    if (bWriteToTMS) {
        const std::size_t bannedListCount = m_vBannedListA[iPad]->size();
        const unsigned int dataBytes =
            static_cast<unsigned int>(sizeof(BANNEDLISTDATA) * bannedListCount);
        PBANNEDLISTDATA pBannedList = new BANNEDLISTDATA[bannedListCount];
        int iCount = 0;
        for (auto it = m_vBannedListA[iPad]->begin();
             it != m_vBannedListA[iPad]->end(); ++it) {
            PBANNEDLISTDATA pData = *it;
            memcpy(&pBannedList[iCount++], pData, sizeof(BANNEDLISTDATA));
        }

        // 4J-PB - write to TMS++ now

        // bool
        // bRes=StorageManager.WriteTMSFile(iPad,C4JStorage::eGlobalStorage_TitleUser,L"BannedList",(std::uint8_t*)pBannedList,
        // dwDataBytes);

        delete[] pBannedList;
    }
    // update telemetry too
}

bool CMinecraftApp::IsInBannedLevelList(int iPad, PlayerUID xuid,
                                        char* pszLevelName) {
    for (auto it = m_vBannedListA[iPad]->begin();
         it != m_vBannedListA[iPad]->end(); ++it) {
        PBANNEDLISTDATA pData = *it;
        if (IsEqualXUID(pData->xuid, xuid) &&
            (strcmp(pData->pszLevelName, pszLevelName) == 0)) {
            return true;
        }
    }

    return false;
}

void CMinecraftApp::RemoveLevelFromBannedLevelList(int iPad, PlayerUID xuid,
                                                   char* pszLevelName) {
    // bool bFound=false;
    // bool bRes;

    // we will have retrieved the banned level list from TMS, so remove this one
    // from it and write it back to TMS
    for (auto it = m_vBannedListA[iPad]->begin();
         it != m_vBannedListA[iPad]->end();) {
        PBANNEDLISTDATA pBannedListData = *it;

        if (pBannedListData != nullptr) {
            if (IsEqualXUID(pBannedListData->xuid, xuid) &&
                (strcmp(pBannedListData->pszLevelName, pszLevelName) == 0)) {
                TelemetryManager->RecordUnBanLevel(iPad);

                // match found, so remove this entry
                it = m_vBannedListA[iPad]->erase(it);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }

    const std::size_t bannedListCount = m_vBannedListA[iPad]->size();
    const unsigned int dataBytes =
        static_cast<unsigned int>(sizeof(BANNEDLISTDATA) * bannedListCount);
    if (dataBytes == 0) {
        // wipe the file
    } else {
        PBANNEDLISTDATA pBannedList =
            (BANNEDLISTDATA*)(new std::uint8_t[dataBytes]);

        for (std::size_t i = 0; i < bannedListCount; ++i) {
            PBANNEDLISTDATA pBannedListData = m_vBannedListA[iPad]->at(i);

            memcpy(&pBannedList[i], pBannedListData, sizeof(BANNEDLISTDATA));
        }
        delete[] pBannedList;
    }

    // update telemetry too
}

// function to add credits for the DLC packs
void CMinecraftApp::AddCreditText(const wchar_t* lpStr) {
    DebugPrintf("ADDING CREDIT - %ls\n", lpStr);
    // add a std::string from the DLC to a credits std::vector
    SCreditTextItemDef* pCreditStruct = new SCreditTextItemDef;
    pCreditStruct->m_eType = eSmallText;
    pCreditStruct->m_iStringID[0] = NO_TRANSLATED_STRING;
    pCreditStruct->m_iStringID[1] = NO_TRANSLATED_STRING;
    pCreditStruct->m_Text = new wchar_t[wcslen(lpStr) + 1];
    wcscpy((wchar_t*)pCreditStruct->m_Text, lpStr);

    vDLCCredits.push_back(pCreditStruct);
}

bool CMinecraftApp::AlreadySeenCreditText(const std::wstring& wstemp) {
    for (unsigned int i = 0; i < m_vCreditText.size(); i++) {
        std::wstring temp = m_vCreditText.at(i);

        // if they are the same, break out of the case
        if (temp.compare(wstemp) == 0) {
            return true;
        }
    }

    // add this text
    m_vCreditText.push_back((wchar_t*)wstemp.c_str());
    return false;
}

unsigned int CMinecraftApp::GetDLCCreditsCount() {
    return (unsigned int)vDLCCredits.size();
}

SCreditTextItemDef* CMinecraftApp::GetDLCCredits(int iIndex) {
    return vDLCCredits.at(iIndex);
}

// Game Host options

void CMinecraftApp::SetGameHostOption(eGameHostOption eVal,
                                      unsigned int uiVal) {
    SetGameHostOption(m_uiGameHostSettings, eVal, uiVal);
}

void CMinecraftApp::SetGameHostOption(unsigned int& uiHostSettings,
                                      eGameHostOption eVal,
                                      unsigned int uiVal) {
    switch (eVal) {
        case eGameHostOption_FriendsOfFriends:
            if (uiVal != 0) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_FRIENDSOFFRIENDS;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_FRIENDSOFFRIENDS;
            }
            break;
        case eGameHostOption_Difficulty:
            // clear the difficulty first
            uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_DIFFICULTY;
            uiHostSettings |= (GAME_HOST_OPTION_BITMASK_DIFFICULTY & uiVal);
            break;
        case eGameHostOption_Gamertags:
            if (uiVal != 0) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_GAMERTAGS;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_GAMERTAGS;
            }

            break;
        case eGameHostOption_GameType:
            // clear the game type first
            uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_GAMETYPE;
            uiHostSettings |=
                (GAME_HOST_OPTION_BITMASK_GAMETYPE & (uiVal << 4));
            break;
        case eGameHostOption_LevelType:
            if (uiVal != 0) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_LEVELTYPE;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_LEVELTYPE;
            }

            break;
        case eGameHostOption_Structures:
            if (uiVal != 0) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_STRUCTURES;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_STRUCTURES;
            }

            break;
        case eGameHostOption_BonusChest:
            if (uiVal != 0) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_BONUSCHEST;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_BONUSCHEST;
            }

            break;
        case eGameHostOption_HasBeenInCreative:
            if (uiVal != 0) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_BEENINCREATIVE;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_BEENINCREATIVE;
            }

            break;
        case eGameHostOption_PvP:
            if (uiVal != 0) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_PVP;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_PVP;
            }

            break;
        case eGameHostOption_TrustPlayers:
            if (uiVal != 0) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_TRUSTPLAYERS;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_TRUSTPLAYERS;
            }

            break;
        case eGameHostOption_TNT:
            if (uiVal != 0) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_TNT;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_TNT;
            }

            break;
        case eGameHostOption_FireSpreads:
            if (uiVal != 0) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_FIRESPREADS;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_FIRESPREADS;
            }
            break;
        case eGameHostOption_CheatsEnabled:
            if (uiVal != 0) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_HOSTFLY;
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_HOSTHUNGER;
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_HOSTINVISIBLE;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_HOSTFLY;
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_HOSTHUNGER;
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_HOSTINVISIBLE;
            }
            break;
        case eGameHostOption_HostCanFly:
            if (uiVal != 0) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_HOSTFLY;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_HOSTFLY;
            }
            break;
        case eGameHostOption_HostCanChangeHunger:
            if (uiVal != 0) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_HOSTHUNGER;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_HOSTHUNGER;
            }
            break;
        case eGameHostOption_HostCanBeInvisible:
            if (uiVal != 0) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_HOSTINVISIBLE;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_HOSTINVISIBLE;
            }
            break;

        case eGameHostOption_BedrockFog:
            if (uiVal != 0) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_BEDROCKFOG;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_BEDROCKFOG;
            }
            break;
        case eGameHostOption_DisableSaving:
            if (uiVal != 0) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_DISABLESAVE;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_DISABLESAVE;
            }
            break;
        case eGameHostOption_WasntSaveOwner:
            if (uiVal != 0) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_NOTOWNER;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_NOTOWNER;
            }
            break;
        case eGameHostOption_MobGriefing:
            if (uiVal != 1) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_MOBGRIEFING;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_MOBGRIEFING;
            }
            break;
        case eGameHostOption_KeepInventory:
            if (uiVal != 0) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_KEEPINVENTORY;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_KEEPINVENTORY;
            }
            break;
        case eGameHostOption_DoMobSpawning:
            if (uiVal != 1) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_DOMOBSPAWNING;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_DOMOBSPAWNING;
            }
            break;
        case eGameHostOption_DoMobLoot:
            if (uiVal != 1) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_DOMOBLOOT;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_DOMOBLOOT;
            }
            break;
        case eGameHostOption_DoTileDrops:
            if (uiVal != 1) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_DOTILEDROPS;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_DOTILEDROPS;
            }
            break;
        case eGameHostOption_NaturalRegeneration:
            if (uiVal != 1) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_NATURALREGEN;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_NATURALREGEN;
            }
            break;
        case eGameHostOption_DoDaylightCycle:
            if (uiVal != 1) {
                uiHostSettings |= GAME_HOST_OPTION_BITMASK_DODAYLIGHTCYCLE;
            } else {
                // off
                uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_DODAYLIGHTCYCLE;
            }
            break;
        case eGameHostOption_WorldSize:
            // clear the difficulty first
            uiHostSettings &= ~GAME_HOST_OPTION_BITMASK_WORLDSIZE;
            uiHostSettings |=
                (GAME_HOST_OPTION_BITMASK_WORLDSIZE &
                 (uiVal << GAME_HOST_OPTION_BITMASK_WORLDSIZE_BITSHIFT));
            break;
        case eGameHostOption_All:
            uiHostSettings = uiVal;
            break;
        default:
            break;
    }
}

unsigned int CMinecraftApp::GetGameHostOption(eGameHostOption eVal) {
    return GetGameHostOption(m_uiGameHostSettings, eVal);
}

unsigned int CMinecraftApp::GetGameHostOption(unsigned int uiHostSettings,
                                              eGameHostOption eVal) {
    // unsigned int uiVal=0;
    switch (eVal) {
        case eGameHostOption_FriendsOfFriends:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_FRIENDSOFFRIENDS);
            break;
        case eGameHostOption_Difficulty:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_DIFFICULTY);
            break;
        case eGameHostOption_Gamertags:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_GAMERTAGS);
            break;
        case eGameHostOption_GameType:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_GAMETYPE) >> 4;
            break;
        case eGameHostOption_All:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_ALL);
            break;
        case eGameHostOption_Tutorial:
            // special case - tutorial is offline, but we want the gamertag
            // option, and set Easy mode, structures on, fire on, tnt on, pvp
            // on, trust players on
            return ((uiHostSettings & GAME_HOST_OPTION_BITMASK_GAMERTAGS) |
                    GAME_HOST_OPTION_BITMASK_TRUSTPLAYERS |
                    GAME_HOST_OPTION_BITMASK_FIRESPREADS |
                    GAME_HOST_OPTION_BITMASK_TNT |
                    GAME_HOST_OPTION_BITMASK_PVP |
                    GAME_HOST_OPTION_BITMASK_STRUCTURES | 1);
            break;
        case eGameHostOption_LevelType:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_LEVELTYPE);
            break;
        case eGameHostOption_Structures:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_STRUCTURES);
            break;
        case eGameHostOption_BonusChest:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_BONUSCHEST);
            break;
        case eGameHostOption_HasBeenInCreative:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_BEENINCREATIVE);
            break;
        case eGameHostOption_PvP:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_PVP);
            break;
        case eGameHostOption_TrustPlayers:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_TRUSTPLAYERS);
            break;
        case eGameHostOption_TNT:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_TNT);
            break;
        case eGameHostOption_FireSpreads:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_FIRESPREADS);
            break;
        case eGameHostOption_CheatsEnabled:
            return (uiHostSettings & (GAME_HOST_OPTION_BITMASK_HOSTFLY |
                                      GAME_HOST_OPTION_BITMASK_HOSTHUNGER |
                                      GAME_HOST_OPTION_BITMASK_HOSTINVISIBLE));
            break;
        case eGameHostOption_HostCanFly:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_HOSTFLY);
            break;
        case eGameHostOption_HostCanChangeHunger:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_HOSTHUNGER);
            break;
        case eGameHostOption_HostCanBeInvisible:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_HOSTINVISIBLE);
            break;
        case eGameHostOption_BedrockFog:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_BEDROCKFOG);
            break;
        case eGameHostOption_DisableSaving:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_DISABLESAVE);
            break;
        case eGameHostOption_WasntSaveOwner:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_NOTOWNER);
        case eGameHostOption_WorldSize:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_WORLDSIZE) >>
                   GAME_HOST_OPTION_BITMASK_WORLDSIZE_BITSHIFT;
        case eGameHostOption_MobGriefing:
            return !(uiHostSettings & GAME_HOST_OPTION_BITMASK_MOBGRIEFING);
        case eGameHostOption_KeepInventory:
            return (uiHostSettings & GAME_HOST_OPTION_BITMASK_KEEPINVENTORY);
        case eGameHostOption_DoMobSpawning:
            return !(uiHostSettings & GAME_HOST_OPTION_BITMASK_DOMOBSPAWNING);
        case eGameHostOption_DoMobLoot:
            return !(uiHostSettings & GAME_HOST_OPTION_BITMASK_DOMOBLOOT);
        case eGameHostOption_DoTileDrops:
            return !(uiHostSettings & GAME_HOST_OPTION_BITMASK_DOTILEDROPS);
        case eGameHostOption_NaturalRegeneration:
            return !(uiHostSettings & GAME_HOST_OPTION_BITMASK_NATURALREGEN);
        case eGameHostOption_DoDaylightCycle:
            return !(uiHostSettings & GAME_HOST_OPTION_BITMASK_DODAYLIGHTCYCLE);
            break;
        default:
            return 0;
    }

    return false;
}

bool CMinecraftApp::CanRecordStatsAndAchievements() {
    bool isTutorial = Minecraft::GetInstance() != nullptr &&
                      Minecraft::GetInstance()->isTutorial();
    // 4J Stu - All of these options give the host player some advantage, so
    // should not allow achievements
    return !(app.GetGameHostOption(eGameHostOption_HasBeenInCreative) ||
             app.GetGameHostOption(eGameHostOption_HostCanBeInvisible) ||
             app.GetGameHostOption(eGameHostOption_HostCanChangeHunger) ||
             app.GetGameHostOption(eGameHostOption_HostCanFly) ||
             app.GetGameHostOption(eGameHostOption_WasntSaveOwner) ||
             !app.GetGameHostOption(eGameHostOption_MobGriefing) ||
             app.GetGameHostOption(eGameHostOption_KeepInventory) ||
             !app.GetGameHostOption(eGameHostOption_DoMobSpawning) ||
             (!app.GetGameHostOption(eGameHostOption_DoDaylightCycle) &&
              !isTutorial));
}

void CMinecraftApp::processSchematics(LevelChunk* levelChunk) {
    m_gameRules.processSchematics(levelChunk);
}

void CMinecraftApp::processSchematicsLighting(LevelChunk* levelChunk) {
    m_gameRules.processSchematicsLighting(levelChunk);
}

void CMinecraftApp::loadDefaultGameRules() {
    m_gameRules.loadDefaultGameRules();
}

void CMinecraftApp::setLevelGenerationOptions(
    LevelGenerationOptions* levelGen) {
    m_gameRules.setLevelGenerationOptions(levelGen);
}

const wchar_t* CMinecraftApp::GetGameRulesString(const std::wstring& key) {
    return m_gameRules.GetGameRulesString(key);
}

unsigned char CMinecraftApp::m_szPNG[8] = {137, 80, 78, 71, 13, 10, 26, 10};

#define PNG_TAG_tEXt 0x74455874

unsigned int CMinecraftApp::FromBigEndian(unsigned int uiValue) {
    unsigned int uiReturn =
        ((uiValue >> 24) & 0x000000ff) | ((uiValue >> 8) & 0x0000ff00) |
        ((uiValue << 8) & 0x00ff0000) | ((uiValue << 24) & 0xff000000);
    return uiReturn;
}

void CMinecraftApp::GetImageTextData(std::uint8_t* imageData,
                                     unsigned int imageBytes,
                                     unsigned char* seedText,
                                     unsigned int& uiHostOptions,
                                     bool& bHostOptionsRead,
                                     std::uint32_t& uiTexturePack) {
    auto readPngUInt32 = [](const std::uint8_t* data) -> unsigned int {
        unsigned int value = 0;
        std::memcpy(&value, data, sizeof(value));
        return value;
    };

    std::uint8_t* ucPtr = imageData;
    unsigned int uiCount = 0;
    unsigned int uiChunkLen;
    unsigned int uiChunkType;
    unsigned int uiCRC;
    char szKeyword[80];

    // check it's a png
    for (int i = 0; i < 8; i++) {
        if (m_szPNG[i] != ucPtr[i]) return;
    }

    uiCount += 8;

    while (uiCount < imageBytes) {
        uiChunkLen = FromBigEndian(readPngUInt32(&ucPtr[uiCount]));
        uiCount += sizeof(int);
        uiChunkType = FromBigEndian(readPngUInt32(&ucPtr[uiCount]));
        uiCount += sizeof(int);

        if (uiChunkType == PNG_TAG_tEXt)  // tEXt
        {
            // check that it's the 4J text
            unsigned char* pszKeyword = &ucPtr[uiCount];
            while (pszKeyword < ucPtr + uiCount + uiChunkLen) {
                ZeroMemory(szKeyword, 80);
                unsigned int uiKeywordC = 0;
                while (*pszKeyword != 0) {
                    szKeyword[uiKeywordC++] = *pszKeyword;
                    pszKeyword++;
                }
                pszKeyword++;
                if (strcmp(szKeyword, "4J_SEED") == 0) {
                    // read the seed value
                    unsigned int uiValueC = 0;
                    while (*pszKeyword != 0 &&
                           (pszKeyword < ucPtr + uiCount + uiChunkLen)) {
                        seedText[uiValueC++] = *pszKeyword;
                        pszKeyword++;
                    }
                    // memcpy(seedText,pszKeyword,uiChunkLen-8);
                } else if (strcmp(szKeyword, "4J_HOSTOPTIONS") == 0) {
                    bHostOptionsRead = true;
                    // read the host options value
                    unsigned int uiValueC = 0;
                    unsigned char pszHostOptions[9];  // Hex representation of
                                                      // unsigned int
                    ZeroMemory(&pszHostOptions, 9);
                    while (*pszKeyword != 0 &&
                           (pszKeyword < ucPtr + uiCount + uiChunkLen) &&
                           uiValueC < 8) {
                        pszHostOptions[uiValueC++] = *pszKeyword;
                        pszKeyword++;
                    }

                    uiHostOptions = 0;
                    std::stringstream ss;
                    ss << pszHostOptions;
                    ss >> std::hex >> uiHostOptions;
                } else if (strcmp(szKeyword, "4J_TEXTUREPACK") == 0) {
                    // read the texture pack value
                    unsigned int uiValueC = 0;
                    unsigned char pszTexturePack[9];  // Hex representation of
                                                      // unsigned int
                    ZeroMemory(&pszTexturePack, 9);
                    while (*pszKeyword != 0 &&
                           (pszKeyword < ucPtr + uiCount + uiChunkLen) &&
                           uiValueC < 8) {
                        pszTexturePack[uiValueC++] = *pszKeyword;
                        pszKeyword++;
                    }

                    std::stringstream ss;
                    ss << pszTexturePack;
                    ss >> std::hex >> uiTexturePack;
                }
            }
        }
        uiCount += uiChunkLen;
        uiCRC = FromBigEndian(readPngUInt32(&ucPtr[uiCount]));
        uiCount += sizeof(int);
    }

    return;
}

unsigned int CMinecraftApp::CreateImageTextData(std::uint8_t* textMetadata,
                                                int64_t seed, bool hasSeed,
                                                unsigned int uiHostOptions,
                                                unsigned int uiTexturePackId) {
    int iTextMetadataBytes = 0;
    if (hasSeed) {
        strcpy((char*)textMetadata, "4J_SEED");
        snprintf((char*)&textMetadata[8], 42, "%lld", (long long)seed);

        // get the length
        iTextMetadataBytes += 8;
        while (textMetadata[iTextMetadataBytes] != 0) iTextMetadataBytes++;
        ++iTextMetadataBytes;  // Add a null terminator at the end of the seed
                               // value
    }

    // Save the host options that this world was last played with
    strcpy((char*)&textMetadata[iTextMetadataBytes], "4J_HOSTOPTIONS");
    snprintf((char*)&textMetadata[iTextMetadataBytes + 15], 9, "%X",
             uiHostOptions);

    iTextMetadataBytes += 15;
    while (textMetadata[iTextMetadataBytes] != 0) iTextMetadataBytes++;
    ++iTextMetadataBytes;  // Add a null terminator at the end of the host
                           // options value

    // Save the texture pack id
    strcpy((char*)&textMetadata[iTextMetadataBytes], "4J_TEXTUREPACK");
    snprintf((char*)&textMetadata[iTextMetadataBytes + 15], 9, "%X",
             uiHostOptions);

    iTextMetadataBytes += 15;
    while (textMetadata[iTextMetadataBytes] != 0) iTextMetadataBytes++;

    return iTextMetadataBytes;
}

void CMinecraftApp::AddTerrainFeaturePosition(_eTerrainFeatureType eFeatureType,
                                              int x, int z) {
    // check we don't already have this in
    for (auto it = m_vTerrainFeatures.begin(); it < m_vTerrainFeatures.end();
         ++it) {
        FEATURE_DATA* pFeatureData = *it;

        if ((pFeatureData->eTerrainFeature == eFeatureType) &&
            (pFeatureData->x == x) && (pFeatureData->z == z))
            return;
    }

    FEATURE_DATA* pFeatureData = new FEATURE_DATA;
    pFeatureData->eTerrainFeature = eFeatureType;
    pFeatureData->x = x;
    pFeatureData->z = z;

    m_vTerrainFeatures.push_back(pFeatureData);
}

_eTerrainFeatureType CMinecraftApp::IsTerrainFeature(int x, int z) {
    for (auto it = m_vTerrainFeatures.begin(); it < m_vTerrainFeatures.end();
         ++it) {
        FEATURE_DATA* pFeatureData = *it;

        if ((pFeatureData->x == x) && (pFeatureData->z == z))
            return pFeatureData->eTerrainFeature;
    }

    return eTerrainFeature_None;
}

bool CMinecraftApp::GetTerrainFeaturePosition(_eTerrainFeatureType eType,
                                              int* pX, int* pZ) {
    for (auto it = m_vTerrainFeatures.begin(); it < m_vTerrainFeatures.end();
         ++it) {
        FEATURE_DATA* pFeatureData = *it;

        if (pFeatureData->eTerrainFeature == eType) {
            *pX = pFeatureData->x;
            *pZ = pFeatureData->z;
            return true;
        }
    }

    return false;
}

void CMinecraftApp::ClearTerrainFeaturePosition() {
    FEATURE_DATA* pFeatureData;
    while (m_vTerrainFeatures.size() > 0) {
        pFeatureData = m_vTerrainFeatures.back();
        m_vTerrainFeatures.pop_back();
        delete pFeatureData;
    }
}

void CMinecraftApp::UpdatePlayerInfo(std::uint8_t networkSmallId,
                                     int16_t playerColourIndex,
                                     unsigned int playerGamePrivileges) {
    for (unsigned int i = 0; i < MINECRAFT_NET_MAX_PLAYERS; ++i) {
        if (m_playerColours[i] == networkSmallId) {
            m_playerColours[i] = 0;
            m_playerGamePrivileges[i] = 0;
        }
    }
    if (playerColourIndex >= 0 &&
        playerColourIndex < MINECRAFT_NET_MAX_PLAYERS) {
        m_playerColours[playerColourIndex] = networkSmallId;
        m_playerGamePrivileges[playerColourIndex] = playerGamePrivileges;
    }
}

short CMinecraftApp::GetPlayerColour(std::uint8_t networkSmallId) {
    short index = -1;
    for (unsigned int i = 0; i < MINECRAFT_NET_MAX_PLAYERS; ++i) {
        if (m_playerColours[i] == networkSmallId) {
            index = i;
            break;
        }
    }
    return index;
}

unsigned int CMinecraftApp::GetPlayerPrivileges(std::uint8_t networkSmallId) {
    unsigned int privileges = 0;
    for (unsigned int i = 0; i < MINECRAFT_NET_MAX_PLAYERS; ++i) {
        if (m_playerColours[i] == networkSmallId) {
            privileges = m_playerGamePrivileges[i];
            break;
        }
    }
    return privileges;
}

std::wstring CMinecraftApp::getEntityName(eINSTANCEOF type) {
    switch (type) {
        case eTYPE_WOLF:
            return app.GetString(IDS_WOLF);
        case eTYPE_CREEPER:
            return app.GetString(IDS_CREEPER);
        case eTYPE_SKELETON:
            return app.GetString(IDS_SKELETON);
        case eTYPE_SPIDER:
            return app.GetString(IDS_SPIDER);
        case eTYPE_ZOMBIE:
            return app.GetString(IDS_ZOMBIE);
        case eTYPE_PIGZOMBIE:
            return app.GetString(IDS_PIGZOMBIE);
        case eTYPE_ENDERMAN:
            return app.GetString(IDS_ENDERMAN);
        case eTYPE_SILVERFISH:
            return app.GetString(IDS_SILVERFISH);
        case eTYPE_CAVESPIDER:
            return app.GetString(IDS_CAVE_SPIDER);
        case eTYPE_GHAST:
            return app.GetString(IDS_GHAST);
        case eTYPE_SLIME:
            return app.GetString(IDS_SLIME);
        case eTYPE_ARROW:
            return app.GetString(IDS_ITEM_ARROW);
        case eTYPE_ENDERDRAGON:
            return app.GetString(IDS_ENDERDRAGON);
        case eTYPE_BLAZE:
            return app.GetString(IDS_BLAZE);
        case eTYPE_LAVASLIME:
            return app.GetString(IDS_LAVA_SLIME);
            // 4J-PB - fix for #107167 - Customer Encountered: TU12: Content:
            // UI: There is no information what killed Player after being slain
            // by Iron Golem.
        case eTYPE_VILLAGERGOLEM:
            return app.GetString(IDS_IRONGOLEM);
        case eTYPE_HORSE:
            return app.GetString(IDS_HORSE);
        case eTYPE_WITCH:
            return app.GetString(IDS_WITCH);
        case eTYPE_WITHERBOSS:
            return app.GetString(IDS_WITHER);
        case eTYPE_BAT:
            return app.GetString(IDS_BAT);
        default:
            break;
    };

    return L"";
}

std::uint32_t CMinecraftApp::m_dwContentTypeA[e_Marketplace_MAX] = {
    XMARKETPLACE_OFFERING_TYPE_CONTENT,  // e_DLC_SkinPack, e_DLC_TexturePacks,
                                         // e_DLC_MashupPacks
    XMARKETPLACE_OFFERING_TYPE_THEME,    // e_DLC_Themes
    XMARKETPLACE_OFFERING_TYPE_AVATARITEM,  // e_DLC_AvatarItems
    XMARKETPLACE_OFFERING_TYPE_TILE,        // e_DLC_Gamerpics
};

unsigned int CMinecraftApp::AddDLCRequest(eDLCMarketplaceType eType,
                                          bool bPromote) {
    // lock access
    { std::lock_guard<std::mutex> lock(csDLCDownloadQueue);

    // If it's already in there, promote it to the top of the list
    int iPosition = 0;
    for (auto it = m_DLCDownloadQueue.begin(); it != m_DLCDownloadQueue.end();
         ++it) {
        DLCRequest* pCurrent = *it;

        if (pCurrent->dwType == m_dwContentTypeA[eType]) {
            // already got this in the list
            if (pCurrent->eState == e_DLC_ContentState_Retrieving ||
                pCurrent->eState == e_DLC_ContentState_Retrieved) {
                // already retrieved this
                return 0;
            } else {
                // promote
                if (bPromote) {
                    m_DLCDownloadQueue.erase(m_DLCDownloadQueue.begin() +
                                             iPosition);
                    m_DLCDownloadQueue.insert(m_DLCDownloadQueue.begin(),
                                              pCurrent);
                }
                return 0;
            }
        }
        iPosition++;
    }

    DLCRequest* pDLCreq = new DLCRequest;
    pDLCreq->dwType = m_dwContentTypeA[eType];
    pDLCreq->eState = e_DLC_ContentState_Idle;

    m_DLCDownloadQueue.push_back(pDLCreq);

    m_bAllDLCContentRetrieved = false;
    }

    app.DebugPrintf("[Consoles_App] Added DLC request.\n");
    return 1;
}

unsigned int CMinecraftApp::AddTMSPPFileTypeRequest(eDLCContentType eType,
                                                    bool bPromote) {
    // lock access
    std::lock_guard<std::mutex> lock(csTMSPPDownloadQueue);

    // If it's already in there, promote it to the top of the list
    int iPosition = 0;
    // ignore promoting for now
    /*
    bool bPromoted=false;


    for(auto it = m_TMSPPDownloadQueue.begin(); it !=
    m_TMSPPDownloadQueue.end(); ++it)
    {
    TMSPPRequest  *pCurrent = *it;

    if(pCurrent->eType==eType)
    {
    if(!(pCurrent->eState == e_TMS_ContentState_Retrieving || pCurrent->eState
    == e_TMS_ContentState_Retrieved))
    {
    // promote
    if(bPromote)
    {
    m_TMSPPDownloadQueue.erase(m_TMSPPDownloadQueue.begin()+iPosition);
    m_TMSPPDownloadQueue.insert(m_TMSPPDownloadQueue.begin(),pCurrent);
    bPromoted=true;
    }
    }
    }
    iPosition++;
    }

    if(bPromoted)
    {
    // re-ordered the list, so leave now
    return 0;
    }
    */

    // special case for data files (not image files)
    if (eType == e_DLC_TexturePackData) {
        int iCount = GetDLCInfoFullOffersCount();

        for (int i = 0; i < iCount; i++) {
            DLC_INFO* pDLC = GetDLCInfoFullOffer(i);

            if ((pDLC->eDLCType == e_DLC_TexturePacks) ||
                (pDLC->eDLCType == e_DLC_MashupPacks)) {
                // first check if the image is already in the memory textures,
                // since we might be loading some from the Title Update
                // partition
                if (pDLC->wchDataFile[0] != 0) {
                    // wchar_t *cString = pDLC->wchDataFile;
                    //  4J-PB - shouldn't check this here - let the TMS files
                    //  override it, so if they are on TMS, we'll take them
                    //  first
                    // int iIndex =
                    // app.GetLocalTMSFileIndex(pDLC->wchDataFile,true);

                    // if(iIndex!=-1)
                    {
                        bool bPresent = app.IsFileInTPD(pDLC->iConfig);

                        if (!bPresent) {
                            // this may already be present in the vector because
                            // of a previous trial/full offer

                            bool bAlreadyInQueue = false;
                            for (auto it = m_TMSPPDownloadQueue.begin();
                                 it != m_TMSPPDownloadQueue.end(); ++it) {
                                TMSPPRequest* pCurrent = *it;

                                if (wcscmp(pDLC->wchDataFile,
                                           pCurrent->wchFilename) == 0) {
                                    bAlreadyInQueue = true;
                                    break;
                                }
                            }

                            if (!bAlreadyInQueue) {
                                TMSPPRequest* pTMSPPreq = new TMSPPRequest;

                                pTMSPPreq->CallbackFunc =
                                    &CMinecraftApp::TMSPPFileReturned;
                                pTMSPPreq->lpCallbackParam = this;
                                pTMSPPreq->eStorageFacility =
                                    C4JStorage::eGlobalStorage_Title;
                                pTMSPPreq->eFileTypeVal =
                                    C4JStorage::TMS_FILETYPE_BINARY;
                                memcpy(pTMSPPreq->wchFilename,
                                       pDLC->wchDataFile,
                                       sizeof(wchar_t) * MAX_BANNERNAME_SIZE);
                                pTMSPPreq->eType = e_DLC_TexturePackData;
                                pTMSPPreq->eState = e_TMS_ContentState_Queued;
                                m_bAllTMSContentRetrieved = false;
                                m_TMSPPDownloadQueue.push_back(pTMSPPreq);
                            }
                        } else {
                            app.DebugPrintf(
                                "Texture data already present in the TPD\n");
                        }
                    }
                }
            }
        }
    } else {  // for all the files of type eType, add them to the download list

        // run through the trial offers first, then the full offers. Any
        // duplicates won't be added to the download queue
        int iCount;
        // and the full offers

        iCount = GetDLCInfoFullOffersCount();
        for (int i = 0; i < iCount; i++) {
            DLC_INFO* pDLC = GetDLCInfoFullOffer(i);
            // if(wcscmp(pDLC->wchType,wchDLCTypeNames[eType])==0)
            if (pDLC->eDLCType == eType) {
                // first check if the image is already in the memory textures,
                // since we might be loading some from the Title Update
                // partition

                wchar_t* cString = pDLC->wchBanner;
                // 4J-PB - shouldn't check this here - let the TMS files
                // override it, so if they are on TMS, we'll take them first
                // int iIndex = app.GetLocalTMSFileIndex(cString,true);

                // if(iIndex!=-1)
                {
                    bool bPresent = app.IsFileInMemoryTextures(cString);

                    if (!bPresent) {
                        // this may already be present in the vector because of
                        // a previous trial/full offer

                        bool bAlreadyInQueue = false;
                        for (auto it = m_TMSPPDownloadQueue.begin();
                             it != m_TMSPPDownloadQueue.end(); ++it) {
                            TMSPPRequest* pCurrent = *it;

                            if (wcscmp(pDLC->wchBanner,
                                       pCurrent->wchFilename) == 0) {
                                bAlreadyInQueue = true;
                                break;
                            }
                        }

                        if (!bAlreadyInQueue) {
                            // app.DebugPrintf("Adding a request to the TMSPP
                            // download queue - %ls\n",pDLC->wchBanner);
                            TMSPPRequest* pTMSPPreq = new TMSPPRequest;
                            ZeroMemory(pTMSPPreq, sizeof(TMSPPRequest));

                            pTMSPPreq->CallbackFunc =
                                &CMinecraftApp::TMSPPFileReturned;
                            pTMSPPreq->lpCallbackParam = this;
                            // 4J-PB - testing for now
                            // pTMSPPreq->eStorageFacility=C4JStorage::eGlobalStorage_TitleUser;
                            pTMSPPreq->eStorageFacility =
                                C4JStorage::eGlobalStorage_Title;
                            pTMSPPreq->eFileTypeVal =
                                C4JStorage::TMS_FILETYPE_BINARY;
                            // wcstombs(pTMSPPreq->szFilename,pDLC->wchBanner,MAX_TMSFILENAME_SIZE);

                            memcpy(pTMSPPreq->wchFilename, pDLC->wchBanner,
                                   sizeof(wchar_t) * MAX_BANNERNAME_SIZE);
                            pTMSPPreq->eType = eType;
                            pTMSPPreq->eState = e_TMS_ContentState_Queued;
                            m_bAllTMSContentRetrieved = false;
                            m_TMSPPDownloadQueue.push_back(pTMSPPreq);
                            app.DebugPrintf(
                                "===m_TMSPPDownloadQueue Adding %ls, q size is "
                                "%d\n",
                                pTMSPPreq->wchFilename,
                                m_TMSPPDownloadQueue.size());
                        }
                    }
                }
            }
        }
    }

    return 1;
}

bool CMinecraftApp::CheckTMSDLCCanStop() {
    std::lock_guard<std::mutex> lock(csTMSPPDownloadQueue);
    for (auto it = m_TMSPPDownloadQueue.begin();
         it != m_TMSPPDownloadQueue.end(); ++it) {
        TMSPPRequest* pCurrent = *it;

        if (pCurrent->eState == e_TMS_ContentState_Retrieving) {
            return false;
        }
    }

    return true;
}

bool CMinecraftApp::RetrieveNextDLCContent() {
    // If there's already a retrieve in progress, quit
    // we may have re-ordered the list, so need to check every item

    // is there a primary player and a network connection?
    int primPad = ProfileManager.GetPrimaryPad();
    if (primPad == -1 || !ProfileManager.IsSignedInLive(primPad)) {
        return true;  // 4J-JEV: We need to wait until the primary player is
                      // online.
    }

    { std::lock_guard<std::mutex> lock(csDLCDownloadQueue);
    for (auto it = m_DLCDownloadQueue.begin(); it != m_DLCDownloadQueue.end();
         ++it) {
        DLCRequest* pCurrent = *it;

        if (pCurrent->eState == e_DLC_ContentState_Retrieving) {
            return true;
        }
    }

    // Now look for the next retrieval
    for (auto it = m_DLCDownloadQueue.begin(); it != m_DLCDownloadQueue.end();
         ++it) {
        DLCRequest* pCurrent = *it;

        if (pCurrent->eState == e_DLC_ContentState_Idle) {
#if defined(_DEBUG)
            app.DebugPrintf("RetrieveNextDLCContent - type = %d\n",
                            pCurrent->dwType);
#endif

            C4JStorage::EDLCStatus status = StorageManager.GetDLCOffers(
                ProfileManager.GetPrimaryPad(),
                &CMinecraftApp::DLCOffersReturned, this, pCurrent->dwType);
            if (status == C4JStorage::EDLC_Pending) {
                pCurrent->eState = e_DLC_ContentState_Retrieving;
            } else {
                // no content of this type, or some other problem
                app.DebugPrintf("RetrieveNextDLCContent - PROBLEM\n");
                pCurrent->eState = e_DLC_ContentState_Retrieved;
            }
            return true;
        }
    }
    }

    app.DebugPrintf("[Consoles_App] Finished downloading dlc content.\n");
    return false;
}

int CMinecraftApp::TMSPPFileReturned(void* pParam, int iPad, int iUserData,
                                     C4JStorage::PTMSPP_FILEDATA pFileData,
                                     const char* szFilename) {
    CMinecraftApp* pClass = (CMinecraftApp*)pParam;

    // find the right one in the vector
    { std::lock_guard<std::mutex> lock(pClass->csTMSPPDownloadQueue);
    for (auto it = pClass->m_TMSPPDownloadQueue.begin();
         it != pClass->m_TMSPPDownloadQueue.end(); ++it) {
        TMSPPRequest* pCurrent = *it;
#if defined(_WINDOWS64)
        char szFile[MAX_TMSFILENAME_SIZE];
        wcstombs(szFile, pCurrent->wchFilename, MAX_TMSFILENAME_SIZE);

        if (strcmp(szFilename, szFile) == 0)
#endif
        {
            // set this to retrieved whether it found it or not
            pCurrent->eState = e_TMS_ContentState_Retrieved;

            if (pFileData != nullptr) {
                switch (pCurrent->eType) {
                    case e_DLC_TexturePackData: {
                        app.DebugPrintf("--- Got texturepack data %ls\n",
                                        pCurrent->wchFilename);
                        // get the config value for the texture pack
                        int iConfig = app.GetTPConfigVal(pCurrent->wchFilename);
                        app.AddMemoryTPDFile(iConfig, pFileData->pbData,
                                             pFileData->size);
                    } break;
                    default:
                        app.DebugPrintf("--- Got image data - %ls\n",
                                        pCurrent->wchFilename);
                        app.AddMemoryTextureFile(pCurrent->wchFilename,
                                                 pFileData->pbData,
                                                 pFileData->size);
                        break;
                }
            } else {
                app.DebugPrintf("TMSImageReturned failed (%s)...\n",
                                szFilename);
            }
            break;
        }
    }
    }

    return 0;
}

bool CMinecraftApp::RetrieveNextTMSPPContent() { return false; }

void CMinecraftApp::TickDLCOffersRetrieved() {
    if (!m_bAllDLCContentRetrieved) {
        if (!app.RetrieveNextDLCContent()) {
            app.DebugPrintf("[Consoles_App] All content retrieved.\n");
            m_bAllDLCContentRetrieved = true;
        }
    }
}
void CMinecraftApp::ClearAndResetDLCDownloadQueue() {
    app.DebugPrintf("[Consoles_App] Clear and reset download queue.\n");

    int iPosition = 0;
    { std::lock_guard<std::mutex> lock(csTMSPPDownloadQueue);
    for (auto it = m_DLCDownloadQueue.begin(); it != m_DLCDownloadQueue.end();
         ++it) {
        DLCRequest* pCurrent = *it;

        delete pCurrent;
        iPosition++;
    }
    m_DLCDownloadQueue.clear();
    m_bAllDLCContentRetrieved = true;
    }
}

void CMinecraftApp::TickTMSPPFilesRetrieved() {
    if (m_bTickTMSDLCFiles && !m_bAllTMSContentRetrieved) {
        if (app.RetrieveNextTMSPPContent() == false) {
            m_bAllTMSContentRetrieved = true;
        }
    }
}
void CMinecraftApp::ClearTMSPPFilesRetrieved() {
    int iPosition = 0;
    { std::lock_guard<std::mutex> lock(csTMSPPDownloadQueue);
    for (auto it = m_TMSPPDownloadQueue.begin();
         it != m_TMSPPDownloadQueue.end(); ++it) {
        TMSPPRequest* pCurrent = *it;

        delete pCurrent;
        iPosition++;
    }
    m_TMSPPDownloadQueue.clear();
    m_bAllTMSContentRetrieved = true;
    }
}

int CMinecraftApp::DLCOffersReturned(void* pParam, int iOfferC,
                                     std::uint32_t dwType, int iPad) {
    CMinecraftApp* pClass = (CMinecraftApp*)pParam;

    // find the right one in the vector
    { std::lock_guard<std::mutex> lock(pClass->csTMSPPDownloadQueue);
    for (auto it = pClass->m_DLCDownloadQueue.begin();
         it != pClass->m_DLCDownloadQueue.end(); ++it) {
        DLCRequest* pCurrent = *it;

        // avatar items are coming back as type Content, so we can't trust the
        // type setting
        if (pCurrent->dwType == static_cast<std::uint32_t>(dwType)) {
            pClass->m_iDLCOfferC = iOfferC;
            app.DebugPrintf(
                "DLCOffersReturned - type %u, count %d - setting to "
                "retrieved\n",
                dwType, iOfferC);
            pCurrent->eState = e_DLC_ContentState_Retrieved;
            break;
        }
    }
    }
    return 0;
}

eDLCContentType CMinecraftApp::Find_eDLCContentType(std::uint32_t dwType) {
    for (int i = 0; i < e_DLC_MAX; i++) {
        if (m_dwContentTypeA[i] == dwType) {
            return (eDLCContentType)i;
        }
    }
    return (eDLCContentType)0;
}
bool CMinecraftApp::DLCContentRetrieved(eDLCMarketplaceType eType) {
    // If there's already a retrieve in progress, quit
    // we may have re-ordered the list, so need to check every item
    std::lock_guard<std::mutex> lock(csDLCDownloadQueue);
    for (auto it = m_DLCDownloadQueue.begin(); it != m_DLCDownloadQueue.end();
         ++it) {
        DLCRequest* pCurrent = *it;

        if ((pCurrent->dwType == m_dwContentTypeA[eType]) &&
            (pCurrent->eState == e_DLC_ContentState_Retrieved)) {
            return true;
        }
    }
    return false;
}

void CMinecraftApp::SetAdditionalSkinBoxes(std::uint32_t dwSkinID,
                                           SKIN_BOX* SkinBoxA,
                                           unsigned int dwSkinBoxC) {
    EntityRenderer* renderer =
        EntityRenderDispatcher::instance->getRenderer(eTYPE_PLAYER);
    Model* pModel = renderer->getModel();
    std::vector<ModelPart*>* pvModelPart = new std::vector<ModelPart*>;
    std::vector<SKIN_BOX*>* pvSkinBoxes = new std::vector<SKIN_BOX*>;

    { std::lock_guard<std::mutex> lock_mp(csAdditionalModelParts);
    std::lock_guard<std::mutex> lock_sb(csAdditionalSkinBoxes);

    app.DebugPrintf(
        "*** SetAdditionalSkinBoxes - Inserting model parts for skin %d from "
        "array of Skin Boxes\n",
        dwSkinID & 0x0FFFFFFF);

    // convert the skin boxes into model parts, and add to the humanoid model
    for (unsigned int i = 0; i < dwSkinBoxC; i++) {
        if (pModel) {
            ModelPart* pModelPart = pModel->AddOrRetrievePart(&SkinBoxA[i]);
            pvModelPart->push_back(pModelPart);
            pvSkinBoxes->push_back(&SkinBoxA[i]);
        }
    }

    m_AdditionalModelParts.insert(
        std::pair<std::uint32_t, std::vector<ModelPart*>*>(dwSkinID,
                                                           pvModelPart));
    m_AdditionalSkinBoxes.insert(
        std::pair<std::uint32_t, std::vector<SKIN_BOX*>*>(dwSkinID,
                                                          pvSkinBoxes));
    }
}

std::vector<ModelPart*>* CMinecraftApp::SetAdditionalSkinBoxes(
    std::uint32_t dwSkinID, std::vector<SKIN_BOX*>* pvSkinBoxA) {
    EntityRenderer* renderer =
        EntityRenderDispatcher::instance->getRenderer(eTYPE_PLAYER);
    Model* pModel = renderer->getModel();
    std::vector<ModelPart*>* pvModelPart = new std::vector<ModelPart*>;

    { std::lock_guard<std::mutex> lock_mp(csAdditionalModelParts);
    std::lock_guard<std::mutex> lock_sb(csAdditionalSkinBoxes);
    app.DebugPrintf(
        "*** SetAdditionalSkinBoxes - Inserting model parts for skin %d from "
        "array of Skin Boxes\n",
        dwSkinID & 0x0FFFFFFF);

    // convert the skin boxes into model parts, and add to the humanoid model
    for (auto it = pvSkinBoxA->begin(); it != pvSkinBoxA->end(); ++it) {
        if (pModel) {
            ModelPart* pModelPart = pModel->AddOrRetrievePart(*it);
            pvModelPart->push_back(pModelPart);
        }
    }

    m_AdditionalModelParts.insert(
        std::pair<std::uint32_t, std::vector<ModelPart*>*>(dwSkinID,
                                                           pvModelPart));
    m_AdditionalSkinBoxes.insert(
        std::pair<std::uint32_t, std::vector<SKIN_BOX*>*>(dwSkinID,
                                                          pvSkinBoxA));
    }
    return pvModelPart;
}

std::vector<ModelPart*>* CMinecraftApp::GetAdditionalModelParts(
    std::uint32_t dwSkinID) {
    std::lock_guard<std::mutex> lock(csAdditionalModelParts);
    std::vector<ModelPart*>* pvModelParts = nullptr;
    if (m_AdditionalModelParts.size() > 0) {
        auto it = m_AdditionalModelParts.find(dwSkinID);
        if (it != m_AdditionalModelParts.end()) {
            pvModelParts = (*it).second;
        }
    }

    return pvModelParts;
}

std::vector<SKIN_BOX*>* CMinecraftApp::GetAdditionalSkinBoxes(
    std::uint32_t dwSkinID) {
    std::lock_guard<std::mutex> lock(csAdditionalSkinBoxes);
    std::vector<SKIN_BOX*>* pvSkinBoxes = nullptr;
    if (m_AdditionalSkinBoxes.size() > 0) {
        auto it = m_AdditionalSkinBoxes.find(dwSkinID);
        if (it != m_AdditionalSkinBoxes.end()) {
            pvSkinBoxes = (*it).second;
        }
    }

    return pvSkinBoxes;
}

unsigned int CMinecraftApp::GetAnimOverrideBitmask(std::uint32_t dwSkinID) {
    std::lock_guard<std::mutex> lock(csAnimOverrideBitmask);
    unsigned int uiAnimOverrideBitmask = 0L;

    if (m_AnimOverrides.size() > 0) {
        auto it = m_AnimOverrides.find(dwSkinID);
        if (it != m_AnimOverrides.end()) {
            uiAnimOverrideBitmask = (*it).second;
        }
    }

    return uiAnimOverrideBitmask;
}

void CMinecraftApp::SetAnimOverrideBitmask(std::uint32_t dwSkinID,
                                           unsigned int uiAnimOverrideBitmask) {
    // Make thread safe
    std::lock_guard<std::mutex> lock(csAnimOverrideBitmask);

    if (m_AnimOverrides.size() > 0) {
        auto it = m_AnimOverrides.find(dwSkinID);
        if (it != m_AnimOverrides.end()) {
            return;  // already in here
        }
    }
    m_AnimOverrides.insert(std::pair<std::uint32_t, unsigned int>(
        dwSkinID, uiAnimOverrideBitmask));
}

std::uint32_t CMinecraftApp::getSkinIdFromPath(const std::wstring& skin) {
    bool dlcSkin = false;
    unsigned int skinId = 0;

    if (skin.size() >= 14) {
        dlcSkin = skin.substr(0, 3).compare(L"dlc") == 0;

        std::wstring skinValue = skin.substr(7, skin.size());
        skinValue = skinValue.substr(0, skinValue.find_first_of(L'.'));

        std::wstringstream ss;
        // 4J Stu - dlc skins are numbered using decimal to make it easier for
        // artists/people to number manually Everything else is numbered using
        // hex
        if (dlcSkin)
            ss << std::dec << skinValue.c_str();
        else
            ss << std::hex << skinValue.c_str();
        ss >> skinId;

        skinId = MAKE_SKIN_BITMASK(dlcSkin, skinId);
    }
    return skinId;
}

std::wstring CMinecraftApp::getSkinPathFromId(std::uint32_t skinId) {
    // 4J Stu - This function maps the encoded uint32_t we store in the player
    // profile to a filename that is stored as a memory texture and shared
    // between systems in game
    wchar_t chars[256];
    if (GET_IS_DLC_SKIN_FROM_BITMASK(skinId)) {
        // 4J Stu - DLC skins are numbered using decimal rather than hex to make
        // it easier to number manually
        swprintf(chars, 256, L"dlcskin%08d.png",
                 GET_DLC_SKIN_ID_FROM_BITMASK(skinId));

    } else {
        std::uint32_t ugcSkinIndex = GET_UGC_SKIN_ID_FROM_BITMASK(skinId);
        std::uint32_t defaultSkinIndex =
            GET_DEFAULT_SKIN_ID_FROM_BITMASK(skinId);
        if (ugcSkinIndex == 0) {
            swprintf(chars, 256, L"defskin%08X.png", defaultSkinIndex);
        } else {
            swprintf(chars, 256, L"ugcskin%08X.png", ugcSkinIndex);
        }
    }
    return chars;
}

int CMinecraftApp::TexturePackDialogReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    return 0;
}

int CMinecraftApp::getArchiveFileSize(const std::wstring& filename) {
    TexturePack* tPack = nullptr;
    Minecraft* pMinecraft = Minecraft::GetInstance();
    if (pMinecraft && pMinecraft->skins)
        tPack = pMinecraft->skins->getSelected();
    if (tPack && tPack->hasData() && tPack->getArchiveFile() &&
        tPack->getArchiveFile()->hasFile(filename)) {
        return tPack->getArchiveFile()->getFileSize(filename);
    } else
        return m_mediaArchive->getFileSize(filename);
}

bool CMinecraftApp::hasArchiveFile(const std::wstring& filename) {
    TexturePack* tPack = nullptr;
    Minecraft* pMinecraft = Minecraft::GetInstance();
    if (pMinecraft && pMinecraft->skins)
        tPack = pMinecraft->skins->getSelected();
    if (tPack && tPack->hasData() && tPack->getArchiveFile() &&
        tPack->getArchiveFile()->hasFile(filename))
        return true;
    else
        return m_mediaArchive->hasFile(filename);
}

byteArray CMinecraftApp::getArchiveFile(const std::wstring& filename) {
    TexturePack* tPack = nullptr;
    Minecraft* pMinecraft = Minecraft::GetInstance();
    if (pMinecraft && pMinecraft->skins)
        tPack = pMinecraft->skins->getSelected();
    if (tPack && tPack->hasData() && tPack->getArchiveFile() &&
        tPack->getArchiveFile()->hasFile(filename)) {
        return tPack->getArchiveFile()->getFile(filename);
    } else
        return m_mediaArchive->getFile(filename);
}

// DLC

int CMinecraftApp::GetDLCInfoTrialOffersCount() {
    return (int)DLCInfo_Trial.size();
}

int CMinecraftApp::GetDLCInfoFullOffersCount() {
    return (int)DLCInfo_Full.size();
}

int CMinecraftApp::GetDLCInfoTexturesOffersCount() {
    return (int)DLCTextures_PackID.size();
}

// AUTOSAVE
void CMinecraftApp::SetAutosaveTimerTime(void) {
    m_uiAutosaveTimer =
        GetTickCount() +
        GetGameSettings(ProfileManager.GetPrimaryPad(), eGameSetting_Autosave) *
            1000 * 60 * 15;
}  // value x 15 to get mins, x60 for secs

bool CMinecraftApp::AutosaveDue(void) {
    return (GetTickCount() > m_uiAutosaveTimer);
}

unsigned int CMinecraftApp::SecondsToAutosave() {
    return (m_uiAutosaveTimer - GetTickCount()) / 1000;
}

void CMinecraftApp::SetTrialTimerStart(void) {
    m_fTrialTimerStart = m_Time.fAppTime;
    mfTrialPausedTime = 0.0f;
}

float CMinecraftApp::getTrialTimer(void) {
    return m_Time.fAppTime - m_fTrialTimerStart - mfTrialPausedTime;
}

bool CMinecraftApp::IsLocalMultiplayerAvailable() {
    unsigned int connectedControllers = 0;
    for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i) {
        if (InputManager.IsPadConnected(i) || ProfileManager.IsSignedIn(i))
            ++connectedControllers;
    }

    bool available = RenderManager.IsHiDef() && connectedControllers > 1;

    return available;

    // Found this in GameNetworkManager?
    // #ifdef 0
    //		iOtherConnectedControllers =
    // InputManager.GetConnectedGamepadCount();
    //		if((InputManager.IsPadConnected(userIndex) ||
    // ProfileManager.IsSignedIn(userIndex)))
    //		{
    //			--iOtherConnectedControllers;
    //		}
    // #else
    //		for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
    //		{
    //			if( (i!=userIndex) && (InputManager.IsPadConnected(i) ||
    // ProfileManager.IsSignedIn(i)) )
    //			{
    //				iOtherConnectedControllers++;
    //			}
    //		}
    // #endif
}

// 4J-PB - language and locale function

void CMinecraftApp::getLocale(std::vector<std::wstring>& vecWstrLocales) {
    std::vector<eMCLang> locales;

    const unsigned int systemLanguage = XGetLanguage();

    // 4J-PB - restrict the 360 language until we're ready to have them in

    switch (systemLanguage) {
        case XC_LANGUAGE_ENGLISH:
            switch (XGetLocale()) {
                case XC_LOCALE_AUSTRALIA:
                case XC_LOCALE_CANADA:
                case XC_LOCALE_CZECH_REPUBLIC:
                case XC_LOCALE_GREECE:
                case XC_LOCALE_HONG_KONG:
                case XC_LOCALE_HUNGARY:
                case XC_LOCALE_INDIA:
                case XC_LOCALE_IRELAND:
                case XC_LOCALE_ISRAEL:
                case XC_LOCALE_NEW_ZEALAND:
                case XC_LOCALE_SAUDI_ARABIA:
                case XC_LOCALE_SINGAPORE:
                case XC_LOCALE_SLOVAK_REPUBLIC:
                case XC_LOCALE_SOUTH_AFRICA:
                case XC_LOCALE_UNITED_ARAB_EMIRATES:
                case XC_LOCALE_GREAT_BRITAIN:
                    locales.push_back(eMCLang_enGB);
                    break;
                default:  // XC_LOCALE_UNITED_STATES
                    break;
            }
            break;
        case XC_LANGUAGE_JAPANESE:
            locales.push_back(eMCLang_jaJP);
            break;
        case XC_LANGUAGE_GERMAN:
            switch (XGetLocale()) {
                case XC_LOCALE_AUSTRIA:
                    locales.push_back(eMCLang_deAT);
                    break;
                case XC_LOCALE_SWITZERLAND:
                    locales.push_back(eMCLang_deCH);
                    break;
                default:  // XC_LOCALE_GERMANY:
                    break;
            }
            locales.push_back(eMCLang_deDE);
            break;
        case XC_LANGUAGE_FRENCH:
            switch (XGetLocale()) {
                case XC_LOCALE_BELGIUM:
                    locales.push_back(eMCLang_frBE);
                    break;
                case XC_LOCALE_CANADA:
                    locales.push_back(eMCLang_frCA);
                    break;
                case XC_LOCALE_SWITZERLAND:
                    locales.push_back(eMCLang_frCH);
                    break;
                default:  // XC_LOCALE_FRANCE:
                    break;
            }
            locales.push_back(eMCLang_frFR);
            break;
        case XC_LANGUAGE_SPANISH:
            switch (XGetLocale()) {
                case XC_LOCALE_MEXICO:
                case XC_LOCALE_ARGENTINA:
                case XC_LOCALE_CHILE:
                case XC_LOCALE_COLOMBIA:
                case XC_LOCALE_UNITED_STATES:
                case XC_LOCALE_LATIN_AMERICA:
                    locales.push_back(eMCLang_laLAS);
                    locales.push_back(eMCLang_esMX);
                    break;
                default:  // XC_LOCALE_SPAIN
                    break;
            }
            locales.push_back(eMCLang_esES);
            break;
        case XC_LANGUAGE_ITALIAN:
            locales.push_back(eMCLang_itIT);
            break;
        case XC_LANGUAGE_KOREAN:
            locales.push_back(eMCLang_koKR);
            break;
        case XC_LANGUAGE_TCHINESE:
            switch (XGetLocale()) {
                case XC_LOCALE_HONG_KONG:
                    locales.push_back(eMCLang_zhHK);
                    locales.push_back(eMCLang_zhTW);
                    break;
                case XC_LOCALE_TAIWAN:
                    locales.push_back(eMCLang_zhTW);
                    locales.push_back(eMCLang_zhHK);
                default:
                    break;
            }
            locales.push_back(eMCLang_hant);
            locales.push_back(eMCLang_zhCHT);
            break;
        case XC_LANGUAGE_PORTUGUESE:
            if (XGetLocale() == XC_LOCALE_BRAZIL) {
                locales.push_back(eMCLang_ptBR);
            }
            locales.push_back(eMCLang_ptPT);
            break;
        case XC_LANGUAGE_POLISH:
            locales.push_back(eMCLang_plPL);
            break;
        case XC_LANGUAGE_RUSSIAN:
            locales.push_back(eMCLang_ruRU);
            break;
        case XC_LANGUAGE_SWEDISH:
            locales.push_back(eMCLang_svSV);
            locales.push_back(eMCLang_svSE);
            break;
        case XC_LANGUAGE_TURKISH:
            locales.push_back(eMCLang_trTR);
            break;
        case XC_LANGUAGE_BNORWEGIAN:
            locales.push_back(eMCLang_nbNO);
            locales.push_back(eMCLang_noNO);
            locales.push_back(eMCLang_nnNO);
            break;
        case XC_LANGUAGE_DUTCH:
            switch (XGetLocale()) {
                case XC_LOCALE_BELGIUM:
                    locales.push_back(eMCLang_nlBE);
                    break;
                default:
                    break;
            }
            locales.push_back(eMCLang_nlNL);
            break;
        case XC_LANGUAGE_SCHINESE:
            switch (XGetLocale()) {
                case XC_LOCALE_SINGAPORE:
                    locales.push_back(eMCLang_zhSG);
                    break;
                default:
                    break;
            }
            locales.push_back(eMCLang_hans);
            locales.push_back(eMCLang_csCS);
            locales.push_back(eMCLang_zhCN);
            break;
    }

    locales.push_back(eMCLang_enUS);
    locales.push_back(eMCLang_null);

    for (int i = 0; i < locales.size(); i++) {
        eMCLang lang = locales.at(i);
        vecWstrLocales.push_back(m_localeA[lang]);
    }
}

int CMinecraftApp::get_eMCLang(wchar_t* pwchLocale) {
    return m_eMCLangA[pwchLocale];
}

int CMinecraftApp::get_xcLang(wchar_t* pwchLocale) {
    return m_xcLangA[pwchLocale];
}

void CMinecraftApp::LocaleAndLanguageInit() {
    m_localeA[eMCLang_zhCHT] = L"zh-CHT";
    m_localeA[eMCLang_csCS] = L"cs-CS";
    m_localeA[eMCLang_laLAS] = L"la-LAS";
    m_localeA[eMCLang_null] = L"en-EN";
    m_localeA[eMCLang_enUS] = L"en-US";
    m_localeA[eMCLang_enGB] = L"en-GB";
    m_localeA[eMCLang_enIE] = L"en-IE";
    m_localeA[eMCLang_enAU] = L"en-AU";
    m_localeA[eMCLang_enNZ] = L"en-NZ";
    m_localeA[eMCLang_enCA] = L"en-CA";
    m_localeA[eMCLang_jaJP] = L"ja-JP";
    m_localeA[eMCLang_deDE] = L"de-DE";
    m_localeA[eMCLang_deAT] = L"de-AT";
    m_localeA[eMCLang_frFR] = L"fr-FR";
    m_localeA[eMCLang_frCA] = L"fr-CA";
    m_localeA[eMCLang_esES] = L"es-ES";
    m_localeA[eMCLang_esMX] = L"es-MX";
    m_localeA[eMCLang_itIT] = L"it-IT";
    m_localeA[eMCLang_koKR] = L"ko-KR";
    m_localeA[eMCLang_ptPT] = L"pt-PT";
    m_localeA[eMCLang_ptBR] = L"pt-BR";
    m_localeA[eMCLang_ruRU] = L"ru-RU";
    m_localeA[eMCLang_nlNL] = L"nl-NL";
    m_localeA[eMCLang_fiFI] = L"fi-FI";
    m_localeA[eMCLang_svSV] = L"sv-SV";
    m_localeA[eMCLang_daDA] = L"da-DA";
    m_localeA[eMCLang_noNO] = L"no-NO";
    m_localeA[eMCLang_plPL] = L"pl-PL";
    m_localeA[eMCLang_trTR] = L"tr-TR";
    m_localeA[eMCLang_elEL] = L"el-EL";

    m_localeA[eMCLang_zhSG] = L"zh-SG";
    m_localeA[eMCLang_zhCN] = L"zh-CN";
    m_localeA[eMCLang_zhHK] = L"zh-HK";
    m_localeA[eMCLang_zhTW] = L"zh-TW";
    m_localeA[eMCLang_nlBE] = L"nl-BE";
    m_localeA[eMCLang_daDK] = L"da-DK";
    m_localeA[eMCLang_frBE] = L"fr-BE";
    m_localeA[eMCLang_frCH] = L"fr-CH";
    m_localeA[eMCLang_deCH] = L"de-CH";
    m_localeA[eMCLang_nbNO] = L"nb-NO";
    m_localeA[eMCLang_enGR] = L"en-GR";
    m_localeA[eMCLang_enHK] = L"en-HK";
    m_localeA[eMCLang_enSA] = L"en-SA";
    m_localeA[eMCLang_enHU] = L"en-HU";
    m_localeA[eMCLang_enIN] = L"en-IN";
    m_localeA[eMCLang_enIL] = L"en-IL";
    m_localeA[eMCLang_enSG] = L"en-SG";
    m_localeA[eMCLang_enSK] = L"en-SK";
    m_localeA[eMCLang_enZA] = L"en-ZA";
    m_localeA[eMCLang_enCZ] = L"en-CZ";
    m_localeA[eMCLang_enAE] = L"en-AE";
    m_localeA[eMCLang_esAR] = L"es-AR";
    m_localeA[eMCLang_esCL] = L"es-CL";
    m_localeA[eMCLang_esCO] = L"es-CO";
    m_localeA[eMCLang_esUS] = L"es-US";
    m_localeA[eMCLang_svSE] = L"sv-SE";

    m_localeA[eMCLang_csCZ] = L"cs-CZ";
    m_localeA[eMCLang_elGR] = L"el-GR";
    m_localeA[eMCLang_nnNO] = L"nn-NO";
    m_localeA[eMCLang_skSK] = L"sk-SK";

    m_localeA[eMCLang_hans] = L"zh-HANS";
    m_localeA[eMCLang_hant] = L"zh-HANT";

    m_eMCLangA[L"zh-CHT"] = eMCLang_zhCHT;
    m_eMCLangA[L"cs-CS"] = eMCLang_csCS;
    m_eMCLangA[L"la-LAS"] = eMCLang_laLAS;
    m_eMCLangA[L"en-EN"] = eMCLang_null;
    m_eMCLangA[L"en-US"] = eMCLang_enUS;
    m_eMCLangA[L"en-GB"] = eMCLang_enGB;
    m_eMCLangA[L"en-IE"] = eMCLang_enIE;
    m_eMCLangA[L"en-AU"] = eMCLang_enAU;
    m_eMCLangA[L"en-NZ"] = eMCLang_enNZ;
    m_eMCLangA[L"en-CA"] = eMCLang_enCA;
    m_eMCLangA[L"ja-JP"] = eMCLang_jaJP;
    m_eMCLangA[L"de-DE"] = eMCLang_deDE;
    m_eMCLangA[L"de-AT"] = eMCLang_deAT;
    m_eMCLangA[L"fr-FR"] = eMCLang_frFR;
    m_eMCLangA[L"fr-CA"] = eMCLang_frCA;
    m_eMCLangA[L"es-ES"] = eMCLang_esES;
    m_eMCLangA[L"es-MX"] = eMCLang_esMX;
    m_eMCLangA[L"it-IT"] = eMCLang_itIT;
    m_eMCLangA[L"ko-KR"] = eMCLang_koKR;
    m_eMCLangA[L"pt-PT"] = eMCLang_ptPT;
    m_eMCLangA[L"pt-BR"] = eMCLang_ptBR;
    m_eMCLangA[L"ru-RU"] = eMCLang_ruRU;
    m_eMCLangA[L"nl-NL"] = eMCLang_nlNL;
    m_eMCLangA[L"fi-FI"] = eMCLang_fiFI;
    m_eMCLangA[L"sv-SV"] = eMCLang_svSV;
    m_eMCLangA[L"da-DA"] = eMCLang_daDA;
    m_eMCLangA[L"no-NO"] = eMCLang_noNO;
    m_eMCLangA[L"pl-PL"] = eMCLang_plPL;
    m_eMCLangA[L"tr-TR"] = eMCLang_trTR;
    m_eMCLangA[L"el-EL"] = eMCLang_elEL;

    m_eMCLangA[L"zh-SG"] = eMCLang_zhSG;
    m_eMCLangA[L"zh-CN"] = eMCLang_zhCN;
    m_eMCLangA[L"zh-HK"] = eMCLang_zhHK;
    m_eMCLangA[L"zh-TW"] = eMCLang_zhTW;
    m_eMCLangA[L"nl-BE"] = eMCLang_nlBE;
    m_eMCLangA[L"da-DK"] = eMCLang_daDK;
    m_eMCLangA[L"fr-BE"] = eMCLang_frBE;
    m_eMCLangA[L"fr-CH"] = eMCLang_frCH;
    m_eMCLangA[L"de-CH"] = eMCLang_deCH;
    m_eMCLangA[L"nb-NO"] = eMCLang_nbNO;
    m_eMCLangA[L"en-GR"] = eMCLang_enGR;
    m_eMCLangA[L"en-HK"] = eMCLang_enHK;
    m_eMCLangA[L"en-SA"] = eMCLang_enSA;
    m_eMCLangA[L"en-HU"] = eMCLang_enHU;
    m_eMCLangA[L"en-IN"] = eMCLang_enIN;
    m_eMCLangA[L"en-IL"] = eMCLang_enIL;
    m_eMCLangA[L"en-SG"] = eMCLang_enSG;
    m_eMCLangA[L"en-SK"] = eMCLang_enSK;
    m_eMCLangA[L"en-ZA"] = eMCLang_enZA;
    m_eMCLangA[L"en-CZ"] = eMCLang_enCZ;
    m_eMCLangA[L"en-AE"] = eMCLang_enAE;
    m_eMCLangA[L"es-AR"] = eMCLang_esAR;
    m_eMCLangA[L"es-CL"] = eMCLang_esCL;
    m_eMCLangA[L"es-CO"] = eMCLang_esCO;
    m_eMCLangA[L"es-US"] = eMCLang_esUS;
    m_eMCLangA[L"sv-SE"] = eMCLang_svSE;

    m_eMCLangA[L"cs-CZ"] = eMCLang_csCZ;
    m_eMCLangA[L"el-GR"] = eMCLang_elGR;
    m_eMCLangA[L"nn-NO"] = eMCLang_nnNO;
    m_eMCLangA[L"sk-SK"] = eMCLang_skSK;

    m_eMCLangA[L"zh-HANS"] = eMCLang_hans;
    m_eMCLangA[L"zh-HANT"] = eMCLang_hant;

    m_xcLangA[L"zh-CHT"] = XC_LOCALE_CHINA;
    m_xcLangA[L"cs-CS"] = XC_LOCALE_CHINA;
    m_xcLangA[L"en-EN"] = XC_LOCALE_UNITED_STATES;
    m_xcLangA[L"en-US"] = XC_LOCALE_UNITED_STATES;
    m_xcLangA[L"en-GB"] = XC_LOCALE_GREAT_BRITAIN;
    m_xcLangA[L"en-IE"] = XC_LOCALE_IRELAND;
    m_xcLangA[L"en-AU"] = XC_LOCALE_AUSTRALIA;
    m_xcLangA[L"en-NZ"] = XC_LOCALE_NEW_ZEALAND;
    m_xcLangA[L"en-CA"] = XC_LOCALE_CANADA;
    m_xcLangA[L"ja-JP"] = XC_LOCALE_JAPAN;
    m_xcLangA[L"de-DE"] = XC_LOCALE_GERMANY;
    m_xcLangA[L"de-AT"] = XC_LOCALE_AUSTRIA;
    m_xcLangA[L"fr-FR"] = XC_LOCALE_FRANCE;
    m_xcLangA[L"fr-CA"] = XC_LOCALE_CANADA;
    m_xcLangA[L"es-ES"] = XC_LOCALE_SPAIN;
    m_xcLangA[L"es-MX"] = XC_LOCALE_MEXICO;
    m_xcLangA[L"it-IT"] = XC_LOCALE_ITALY;
    m_xcLangA[L"ko-KR"] = XC_LOCALE_KOREA;
    m_xcLangA[L"pt-PT"] = XC_LOCALE_PORTUGAL;
    m_xcLangA[L"pt-BR"] = XC_LOCALE_BRAZIL;
    m_xcLangA[L"ru-RU"] = XC_LOCALE_RUSSIAN_FEDERATION;
    m_xcLangA[L"nl-NL"] = XC_LOCALE_NETHERLANDS;
    m_xcLangA[L"fi-FI"] = XC_LOCALE_FINLAND;
    m_xcLangA[L"sv-SV"] = XC_LOCALE_SWEDEN;
    m_xcLangA[L"da-DA"] = XC_LOCALE_DENMARK;
    m_xcLangA[L"no-NO"] = XC_LOCALE_NORWAY;
    m_xcLangA[L"pl-PL"] = XC_LOCALE_POLAND;
    m_xcLangA[L"tr-TR"] = XC_LOCALE_TURKEY;
    m_xcLangA[L"el-EL"] = XC_LOCALE_GREECE;
    m_xcLangA[L"la-LAS"] = XC_LOCALE_LATIN_AMERICA;

    // New ones for Xbox One
    m_xcLangA[L"zh-SG"] = XC_LOCALE_SINGAPORE;
    m_xcLangA[L"Zh-CN"] = XC_LOCALE_CHINA;
    m_xcLangA[L"zh-HK"] = XC_LOCALE_HONG_KONG;
    m_xcLangA[L"zh-TW"] = XC_LOCALE_TAIWAN;
    m_xcLangA[L"nl-BE"] = XC_LOCALE_BELGIUM;
    m_xcLangA[L"da-DK"] = XC_LOCALE_DENMARK;
    m_xcLangA[L"fr-BE"] = XC_LOCALE_BELGIUM;
    m_xcLangA[L"fr-CH"] = XC_LOCALE_SWITZERLAND;
    m_xcLangA[L"de-CH"] = XC_LOCALE_SWITZERLAND;
    m_xcLangA[L"nb-NO"] = XC_LOCALE_NORWAY;
    m_xcLangA[L"en-GR"] = XC_LOCALE_GREECE;
    m_xcLangA[L"en-HK"] = XC_LOCALE_HONG_KONG;
    m_xcLangA[L"en-SA"] = XC_LOCALE_SAUDI_ARABIA;
    m_xcLangA[L"en-HU"] = XC_LOCALE_HUNGARY;
    m_xcLangA[L"en-IN"] = XC_LOCALE_INDIA;
    m_xcLangA[L"en-IL"] = XC_LOCALE_ISRAEL;
    m_xcLangA[L"en-SG"] = XC_LOCALE_SINGAPORE;
    m_xcLangA[L"en-SK"] = XC_LOCALE_SLOVAK_REPUBLIC;
    m_xcLangA[L"en-ZA"] = XC_LOCALE_SOUTH_AFRICA;
    m_xcLangA[L"en-CZ"] = XC_LOCALE_CZECH_REPUBLIC;
    m_xcLangA[L"en-AE"] = XC_LOCALE_UNITED_ARAB_EMIRATES;
    m_xcLangA[L"ja-IP"] = XC_LOCALE_JAPAN;
    m_xcLangA[L"es-AR"] = XC_LOCALE_ARGENTINA;
    m_xcLangA[L"es-CL"] = XC_LOCALE_CHILE;
    m_xcLangA[L"es-CO"] = XC_LOCALE_COLOMBIA;
    m_xcLangA[L"es-US"] = XC_LOCALE_UNITED_STATES;
    m_xcLangA[L"sv-SE"] = XC_LOCALE_SWEDEN;

    m_xcLangA[L"cs-CZ"] = XC_LOCALE_CZECH_REPUBLIC;
    m_xcLangA[L"el-GR"] = XC_LOCALE_GREECE;
    m_xcLangA[L"sk-SK"] = XC_LOCALE_SLOVAK_REPUBLIC;

    m_xcLangA[L"zh-HANS"] = XC_LOCALE_CHINA;
    m_xcLangA[L"zh-HANT"] = XC_LOCALE_CHINA;
}

void CMinecraftApp::SetTickTMSDLCFiles(bool bVal) {
    // 4J-PB - we need to stop the retrieval of minecraft store images from TMS
    // when we aren't in the DLC, since going in to Play Game will change the
    // title id group
    m_bTickTMSDLCFiles = bVal;
}

std::wstring CMinecraftApp::getFilePath(std::uint32_t packId,
                                        std::wstring filename,
                                        bool bAddDataFolder,
                                        std::wstring mountPoint) {
    std::wstring path =
        getRootPath(packId, true, bAddDataFolder, mountPoint) + filename;
    File f(path);
    if (f.exists()) {
        return path;
    }
    return getRootPath(packId, false, true, mountPoint) + filename;
}

enum ETitleUpdateTexturePacks {
    // eTUTP_MassEffect = 0x400,
    // eTUTP_Skyrim = 0x401,
    // eTUTP_Halo = 0x402,
    // eTUTP_Festive = 0x405,

    // eTUTP_Plastic = 0x801,
    // eTUTP_Candy = 0x802,
    // eTUTP_Fantasy = 0x803,
    eTUTP_Halloween = 0x804,
    // eTUTP_Natural = 0x805,
    // eTUTP_City = 0x01000806, // 4J Stu - The released City pack had a
    // sub-pack ID eTUTP_Cartoon = 0x807, eTUTP_Steampunk = 0x01000808, // 4J
    // Stu - The released Steampunk pack had a sub-pack ID
};

#if defined(_WINDOWS64)
std::wstring titleUpdateTexturePackRoot = L"Windows64\\DLC\\";
#else
std::wstring titleUpdateTexturePackRoot = L"CU\\DLC\\";
#endif

std::wstring CMinecraftApp::getRootPath(std::uint32_t packId,
                                        bool allowOverride, bool bAddDataFolder,
                                        std::wstring mountPoint) {
    std::wstring path = mountPoint;
    if (allowOverride) {
        switch (packId) {
            case eTUTP_Halloween:
                path = titleUpdateTexturePackRoot + L"Halloween Texture Pack";
                break;
        };
        File folder(path);
        if (!folder.exists()) {
            path = mountPoint;
        }
    }

    if (bAddDataFolder) {
        return path + L"\\Data\\";
    } else {
        return path + L"\\";
    }
}
