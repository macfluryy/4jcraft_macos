#pragma once

#include <cstdint>
#include <mutex>

#include "util/Timer.h"
#include "platform/sdl2/Profile.h"
#include "platform/sdl2/Storage.h"



#include "app/common/IPlatformGame.h"
#include "app/common/App_structs.h"
#include "app/common/src/Audio/Consoles_SoundEngine.h"
#include "app/common/src/DLC/DLCManager.h"
#include "app/common/src/GameRules/ConsoleGameRulesConstants.h"
#include "app/common/src/GameRules/GameRuleManager.h"
#include "app/common/src/Localisation/StringTable.h"
#include "app/common/src/Tutorial/TutorialEnum.h"
#include "app/common/src/UI/All Platforms/ArchiveFile.h"
#include "app/common/src/UI/All Platforms/UIStructs.h"
#include "app/include/NetTypes.h"
#include "app/include/SkinBox.h"
#include "app/include/XboxStubs.h"
#include "minecraft/network/packet/DisconnectPacket.h"
#include "minecraft/world/entity/item/MinecartHopper.h"

typedef struct _JoinFromInviteData {
    std::uint32_t dwUserIndex;       
    std::uint32_t dwLocalUsersMask;  
    const INVITE_INFO* pInviteInfo;  
} JoinFromInviteData;

class Player;
class Inventory;
class Level;
class FurnaceTileEntity;
class Container;
class DispenserTileEntity;
class SignTileEntity;
class BrewingStandTileEntity;
class CommandBlockEntity;
class HopperTileEntity;

class EntityHorse;
class BeaconTileEntity;
class LocalPlayer;
class DLCPack;
class LevelRuleset;
class ConsoleSchematicFile;
class Model;
class ModelPart;
class StringTable;
class Merchant;

class CMinecraftAudio;

class Game : public IPlatformGame {
private:
    static int s_iHTMLFontSizesA[eHTMLSize_COUNT];

public:
    Game();

    static const float fSafeZoneX;  
    static const float fSafeZoneY;  

    typedef std::vector<PMEMDATA> VMEMFILES;
    typedef std::vector<PNOTIFICATION> VNOTIFICATIONS;

    
    std::vector<std::wstring> vSkinNames;
    DLCManager m_dlcManager;

    
    std::vector<std::wstring> m_vCreditText;
    static const int GAME_SETTINGS_PROFILE_DATA_BYTES = 204;

#if defined(_EXTENDED_ACHIEVEMENTS)
    static const int GAME_DEFINED_PROFILE_DATA_BYTES = 2 * 972;  
#else
    static const int GAME_DEFINED_PROFILE_DATA_BYTES = 972;  
#endif
    unsigned int uiGameDefinedDataChangedBitmask;

    void DebugPrintf(const char* szFormat, ...);
    void DebugPrintfVerbose(bool bVerbose, const char* szFormat,
                            ...);  
    void DebugPrintf(int user, const char* szFormat, ...);

    static const int USER_NONE = 0;  
    static const int USER_GENERAL = 1;
    static const int USER_JV = 2;
    static const int USER_MH = 3;
    static const int USER_PB = 4;
    static const int USER_RR = 5;
    static const int USER_SR = 6;
    static const int USER_UI =
        7;  

    void HandleButtonPresses();
    bool IntroRunning() { return m_bIntroRunning; }
    void SetIntroRunning(bool bSet) { m_bIntroRunning = bSet; }
#if defined(_CONTENT_PACKAGE)
#if !defined(_FINAL_BUILD)
    bool PartnernetPasswordRunning() { return m_bPartnernetPasswordRunning; }
    void SetPartnernetPasswordRunning(bool bSet) {
        m_bPartnernetPasswordRunning = bSet;
    }
#endif
#endif

    bool IsAppPaused();
    void SetAppPaused(bool val);
    int displaySavingMessage(const C4JStorage::ESavingMessage eMsg, int iPad);
    bool GetGameStarted() { return m_bGameStarted; }
    void SetGameStarted(bool bVal) {
        if (bVal)
            DebugPrintf("SetGameStarted - true\n");
        else
            DebugPrintf("SetGameStarted - false\n");
        m_bGameStarted = bVal;
        m_bIsAppPaused = !bVal;
    }
    int GetLocalPlayerCount(void);
    bool LoadInventoryMenu(int iPad, std::shared_ptr<LocalPlayer> player,
                           bool bNavigateBack = false);
    bool LoadCreativeMenu(int iPad, std::shared_ptr<LocalPlayer> player,
                          bool bNavigateBack = false);
    bool LoadEnchantingMenu(int iPad, std::shared_ptr<Inventory> inventory,
                            int x, int y, int z, Level* level,
                            const std::wstring& name);
    bool LoadFurnaceMenu(int iPad, std::shared_ptr<Inventory> inventory,
                         std::shared_ptr<FurnaceTileEntity> furnace);
    bool LoadBrewingStandMenu(
        int iPad, std::shared_ptr<Inventory> inventory,
        std::shared_ptr<BrewingStandTileEntity> brewingStand);
    bool LoadContainerMenu(int iPad, std::shared_ptr<Container> inventory,
                           std::shared_ptr<Container> container);
    bool LoadTrapMenu(int iPad, std::shared_ptr<Container> inventory,
                      std::shared_ptr<DispenserTileEntity> trap);
    bool LoadCrafting2x2Menu(int iPad, std::shared_ptr<LocalPlayer> player);
    bool LoadCrafting3x3Menu(int iPad, std::shared_ptr<LocalPlayer> player,
                             int x, int y, int z);
    bool LoadFireworksMenu(int iPad, std::shared_ptr<LocalPlayer> player, int x,
                           int y, int z);
    bool LoadSignEntryMenu(int iPad, std::shared_ptr<SignTileEntity> sign);
    bool LoadRepairingMenu(int iPad, std::shared_ptr<Inventory> inventory,
                           Level* level, int x, int y, int z);
    bool LoadTradingMenu(int iPad, std::shared_ptr<Inventory> inventory,
                         std::shared_ptr<Merchant> trader, Level* level,
                         const std::wstring& name);

    bool LoadCommandBlockMenu(
        int iPad, std::shared_ptr<CommandBlockEntity> commandBlock) {
        return false;
    }
    bool LoadHopperMenu(int iPad, std::shared_ptr<Inventory> inventory,
                        std::shared_ptr<HopperTileEntity> hopper);
    bool LoadHopperMenu(int iPad, std::shared_ptr<Inventory> inventory,
                        std::shared_ptr<MinecartHopper> hopper);
    bool LoadHorseMenu(int iPad, std::shared_ptr<Inventory> inventory,
                       std::shared_ptr<Container> container,
                       std::shared_ptr<EntityHorse> horse);
    bool LoadBeaconMenu(int iPad, std::shared_ptr<Inventory> inventory,
                        std::shared_ptr<BeaconTileEntity> beacon);

    bool GetTutorialMode() { return m_bTutorialMode; }
    void SetTutorialMode(bool bSet) { m_bTutorialMode = bSet; }

    void SetSpecialTutorialCompletionFlag(int iPad, int index);

    static const wchar_t* GetString(int iID);

    eGameMode GetGameMode() { return m_eGameMode; }
    void SetGameMode(eGameMode eMode) { m_eGameMode = eMode; }

    eXuiAction GetGlobalXuiAction() { return m_eGlobalXuiAction; }
    void SetGlobalXuiAction(eXuiAction action) { m_eGlobalXuiAction = action; }
    eXuiAction GetXuiAction(int iPad) { return m_eXuiAction[iPad]; }
    void SetAction(int iPad, eXuiAction action, void* param = nullptr);
    void SetTMSAction(int iPad, eTMSAction action) {
        m_eTMSAction[iPad] = action;
    }
    eTMSAction GetTMSAction(int iPad) { return m_eTMSAction[iPad]; }
    eXuiServerAction GetXuiServerAction(int iPad) {
        return m_eXuiServerAction[iPad];
    }
    void* GetXuiServerActionParam(int iPad) {
        return m_eXuiServerActionParam[iPad];
    }
    void SetXuiServerAction(int iPad, eXuiServerAction action,
                            void* param = nullptr) {
        m_eXuiServerAction[iPad] = action;
        m_eXuiServerActionParam[iPad] = param;
    }
    eXuiServerAction GetGlobalXuiServerAction() {
        return m_eGlobalXuiServerAction;
    }
    void SetGlobalXuiServerAction(eXuiServerAction action) {
        m_eGlobalXuiServerAction = action;
    }

    DisconnectPacket::eDisconnectReason GetDisconnectReason() {
        return m_disconnectReason;
    }
    void SetDisconnectReason(DisconnectPacket::eDisconnectReason bVal) {
        m_disconnectReason = bVal;
    }
    const std::wstring& GetDisconnectReasonText() {
        return m_disconnectReasonText;
    }
    void SetDisconnectReasonText(const std::wstring& text) {
        m_disconnectReasonText = text;
    }

    bool GetChangingSessionType() { return m_bChangingSessionType; }
    void SetChangingSessionType(bool bVal) { m_bChangingSessionType = bVal; }

    bool GetReallyChangingSessionType() { return m_bReallyChangingSessionType; }
    void SetReallyChangingSessionType(bool bVal) {
        m_bReallyChangingSessionType = bVal;
    }

    
    
    static void SetActionConfirmed(void* param);
    void HandleXuiActions(void);

    
    bool GetLoadSavesFromFolderEnabled() {
        return m_bLoadSavesFromFolderEnabled;
    }
    void SetLoadSavesFromFolderEnabled(bool bVal) {
        m_bLoadSavesFromFolderEnabled = bVal;
    }

    
    bool GetWriteSavesToFolderEnabled() { return m_bWriteSavesToFolderEnabled; }
    void SetWriteSavesToFolderEnabled(bool bVal) {
        m_bWriteSavesToFolderEnabled = bVal;
    }
    bool GetMobsDontAttackEnabled() { return m_bMobsDontAttack; }
    void SetMobsDontAttackEnabled(bool bVal) { m_bMobsDontAttack = bVal; }
    bool GetUseDPadForDebug() { return m_bUseDPadForDebug; }
    void SetUseDPadForDebug(bool bVal) { m_bUseDPadForDebug = bVal; }
    bool GetMobsDontTickEnabled() { return m_bMobsDontTick; }
    void SetMobsDontTickEnabled(bool bVal) { m_bMobsDontTick = bVal; }

    bool GetFreezePlayers() { return m_bFreezePlayers; }
    void SetFreezePlayers(bool bVal) { m_bFreezePlayers = bVal; }

    
    void ShowSafeArea(bool show) {}
    
    virtual void CaptureScreenshot(int iPad) {};
    
    

    void InitGameSettings();
    static int OldProfileVersionCallback(void* pParam, unsigned char* pucData,
                                         const unsigned short usVersion,
                                         const int iPad);

    static int DefaultOptionsCallback(void* pParam,
                                      C_4JProfile::PROFILESETTINGS* pSettings,
                                      const int iPad);
    int SetDefaultOptions(C_4JProfile::PROFILESETTINGS* pSettings,
                          const int iPad);
    void SetRichPresenceContext(int iPad, int contextId) override = 0;

    void SetGameSettings(int iPad, eGameSetting eVal, unsigned char ucVal);
    unsigned char GetGameSettings(int iPad, eGameSetting eVal);
    unsigned char GetGameSettings(eGameSetting eVal);  
    void SetPlayerSkin(int iPad, const std::wstring& name);
    void SetPlayerSkin(int iPad, std::uint32_t dwSkinId);
    void SetPlayerCape(int iPad, const std::wstring& name);
    void SetPlayerCape(int iPad, std::uint32_t dwCapeId);
    void SetPlayerFavoriteSkin(int iPad, int iIndex, unsigned int uiSkinID);
    unsigned int GetPlayerFavoriteSkin(int iPad, int iIndex);
    unsigned char GetPlayerFavoriteSkinsPos(int iPad);
    void SetPlayerFavoriteSkinsPos(int iPad, int iPos);
    unsigned int GetPlayerFavoriteSkinsCount(int iPad);
    void ValidateFavoriteSkins(
        int iPad);  

    
    void HideMashupPackWorld(int iPad, unsigned int iMashupPackID);
    void EnableMashupPackWorlds(int iPad);
    unsigned int GetMashupPackWorlds(int iPad);

    
    void SetMinecraftLanguage(int iPad, unsigned char ucLanguage);
    unsigned char GetMinecraftLanguage(int iPad);
    void SetMinecraftLocale(int iPad, unsigned char ucLanguage);
    unsigned char GetMinecraftLocale(int iPad);

    
    
    unsigned int GetOpacityTimer(int iPad) {
        return m_uiOpacityCountDown[iPad];
    }
    void SetOpacityTimer(int iPad) {
        m_uiOpacityCountDown[iPad] = 120;
    }  
    void TickOpacityTimer(int iPad) {
        if (m_uiOpacityCountDown[iPad] > 0) m_uiOpacityCountDown[iPad]--;
    }

public:
    std::wstring GetPlayerSkinName(int iPad);
    std::uint32_t GetPlayerSkinId(int iPad);
    std::wstring GetPlayerCapeName(int iPad);
    std::uint32_t GetPlayerCapeId(int iPad);
    std::uint32_t GetAdditionalModelParts(int iPad);
    void CheckGameSettingsChanged(bool bOverride5MinuteTimer = false,
                                  int iPad = XUSER_INDEX_ANY);
    void ApplyGameSettingsChanged(int iPad);
    void ClearGameSettingsChangedFlag(int iPad);
    void ActionGameSettings(int iPad, eGameSetting eVal);
    unsigned int GetGameSettingsDebugMask(int iPad = -1,
                                          bool bOverridePlayer = false);
    void SetGameSettingsDebugMask(int iPad, unsigned int uiVal);
    void ActionDebugMask(int iPad, bool bSetAllClear = false);

    
    bool IsLocalMultiplayerAvailable();

    
    static void SignInChangeCallback(void* pParam, bool bVal,
                                     unsigned int uiSignInData);
    static void ClearSignInChangeUsersMask();
    static int SignoutExitWorldThreadProc(void* lpParameter);
    static int PrimaryPlayerSignedOutReturned(void* pParam, int iPad,
                                              const C4JStorage::EMessageResult);
    static int EthernetDisconnectReturned(void* pParam, int iPad,
                                          const C4JStorage::EMessageResult);
    static void ProfileReadErrorCallback(void* pParam);

    
    virtual void FatalLoadError();

    
    static void NotificationsCallback(void* pParam,
                                      std::uint32_t dwNotification,
                                      unsigned int uiParam);

    
    static void LiveLinkChangeCallback(void* pParam, bool bConnected);
    bool GetLiveLinkRequired() { return m_bLiveLinkRequired; }
    void SetLiveLinkRequired(bool required) { m_bLiveLinkRequired = required; }

#if defined(_DEBUG_MENUS_ENABLED)
    bool DebugSettingsOn() { return m_bDebugOptions; }
    bool DebugArtToolsOn();
#else
    bool DebugSettingsOn() { return false; }
    bool DebugArtToolsOn() { return false; }
#endif
    void SetDebugSequence(const char* pchSeq);
    
    

    
    bool StartInstallDLCProcess(int iPad);
    int dlcInstalledCallback(int iOfferC, int iPad);
    void HandleDLCLicenseChange();
    int dlcMountedCallback(int iPad, std::uint32_t dwErr,
                           std::uint32_t dwLicenceMask);
    void MountNextDLC(int iPad);
    
    
    void HandleDLC(DLCPack* pack);
    bool DLCInstallPending() { return m_bDLCInstallPending; }
    bool DLCInstallProcessCompleted() { return m_bDLCInstallProcessCompleted; }
    void ClearDLCInstalled() { m_bDLCInstallProcessCompleted = false; }
    static int MarketplaceCountsCallback(void* pParam,
                                         C4JStorage::DLC_TMS_DETAILS*,
                                         int iPad);

    bool AlreadySeenCreditText(const std::wstring& wstemp);

    void ClearNewDLCAvailable(void) {
        m_bNewDLCAvailable = false;
        m_bSeenNewDLCTip = true;
    }
    bool GetNewDLCAvailable() { return m_bNewDLCAvailable; }
    void DisplayNewDLCTipAgain() { m_bSeenNewDLCTip = false; }
    bool DisplayNewDLCTip() {
        if (!m_bSeenNewDLCTip) {
            m_bSeenNewDLCTip = true;
            return true;
        } else
            return false;
    }

    
    
    virtual void StoreLaunchData();
    virtual void ExitGame();

    bool isXuidNotch(PlayerUID xuid);
    bool isXuidDeadmau5(PlayerUID xuid);

    void AddMemoryTextureFile(const std::wstring& wName, std::uint8_t* pbData,
                              unsigned int byteCount);
    void RemoveMemoryTextureFile(const std::wstring& wName);
    void GetMemFileDetails(const std::wstring& wName, std::uint8_t** ppbData,
                           unsigned int* pByteCount);
    bool IsFileInMemoryTextures(const std::wstring& wName);

    
    void AddMemoryTPDFile(int iConfig, std::uint8_t* pbData,
                          unsigned int byteCount);
    void RemoveMemoryTPDFile(int iConfig);
    bool IsFileInTPD(int iConfig);
    void GetTPD(int iConfig, std::uint8_t** ppbData, unsigned int* pByteCount);
    int GetTPDSize() { return m_MEM_TPD.size(); }
    int GetTPConfigVal(wchar_t* pwchDataFile);

    bool DefaultCapeExists();
    
    

    
    
    void ProcessInvite(std::uint32_t dwUserIndex,
                       std::uint32_t dwLocalUsersMask,
                       const INVITE_INFO* pInviteInfo);

    
    void AddCreditText(const wchar_t* lpStr);

private:
    PlayerUID m_xuidNotch;
    std::unordered_map<PlayerUID, std::uint8_t*> m_GTS_Files;

    
    std::unordered_map<std::wstring, PMEMDATA> m_MEM_Files;
    
    std::unordered_map<int, PMEMDATA> m_MEM_TPD;
    std::mutex csMemFilesLock;  
    std::mutex csMemTPDLock;    

    VNOTIFICATIONS m_vNotifications;

public:
    
    std::uint8_t* m_pLaunchData;
    unsigned int m_dwLaunchDataSize;

public:
    
    void AddLevelToBannedLevelList(int iPad, PlayerUID xuid, char* pszLevelName,
                                   bool bWriteToTMS);
    bool IsInBannedLevelList(int iPad, PlayerUID xuid, char* pszLevelName);
    void RemoveLevelFromBannedLevelList(int iPad, PlayerUID xuid,
                                        char* pszLevelName);
    void InvalidateBannedList(int iPad);
    void SetUniqueMapName(char* pszUniqueMapName);
    char* GetUniqueMapName(void);

public:
    bool GetResourcesLoaded() { return m_bResourcesLoaded; }
    void SetResourcesLoaded(bool bVal) { m_bResourcesLoaded = bVal; }

public:
    bool m_bGameStarted;
    bool m_bIntroRunning;
    bool m_bTutorialMode;
    bool m_bIsAppPaused;

    bool m_bChangingSessionType;
    bool m_bReallyChangingSessionType;

    
    

    void loadMediaArchive();
    void loadStringTable();

protected:
    ArchiveFile* m_mediaArchive;
    StringTable* m_stringTable;

public:
    int getArchiveFileSize(const std::wstring& filename);
    bool hasArchiveFile(const std::wstring& filename);
    std::vector<uint8_t> getArchiveFile(const std::wstring& filename);

private:
    static int BannedLevelDialogReturned(void* pParam, int iPad,
                                         const C4JStorage::EMessageResult);
    static int TexturePackDialogReturned(void* pParam, int iPad,
                                         C4JStorage::EMessageResult result);

    VBANNEDLIST* m_vBannedListA[XUSER_MAX_COUNT];

    void HandleButtonPresses(int iPad);

    bool m_bResourcesLoaded;

    
    

    

    

    

#if defined(_CONTENT_PACKAGE)
#if !defined(_FINAL_BUILD)
    bool m_bPartnernetPasswordRunning;
#endif
#endif

    eGameMode m_eGameMode;  

    static unsigned int m_uiLastSignInData;

    
    
    GAME_SETTINGS* GameSettingsA[XUSER_MAX_COUNT];

    
    bool m_bLoadSavesFromFolderEnabled;

    
    bool m_bWriteSavesToFolderEnabled;
    bool m_bMobsDontAttack;
    bool m_bUseDPadForDebug;
    bool m_bMobsDontTick;
    bool m_bFreezePlayers;

    
    
    

    DisconnectPacket::eDisconnectReason m_disconnectReason;
    std::wstring m_disconnectReasonText;

public:
    virtual void RunFrame() {};

    static constexpr unsigned int m_dwOfferID = 0x00000001;

    
    void InitTime();
    void UpdateTime();

    
    void SetTrialTimerStart(void);
    float getTrialTimer(void);

    
    VNOTIFICATIONS* GetNotifications() { return &m_vNotifications; }

private:
    
    
    
    eXuiAction m_eXuiAction[XUSER_MAX_COUNT];
    eTMSAction m_eTMSAction[XUSER_MAX_COUNT];
    void* m_eXuiActionParam[XUSER_MAX_COUNT];
    eXuiAction m_eGlobalXuiAction;
    eXuiServerAction m_eXuiServerAction[XUSER_MAX_COUNT];
    void* m_eXuiServerActionParam[XUSER_MAX_COUNT];
    eXuiServerAction m_eGlobalXuiServerAction;

    bool m_bLiveLinkRequired;

    static int UnlockFullExitReturned(void* pParam, int iPad,
                                      C4JStorage::EMessageResult result);
    static int UnlockFullSaveReturned(void* pParam, int iPad,
                                      C4JStorage::EMessageResult result);
    static int UnlockFullInviteReturned(void* pParam, int iPad,
                                        C4JStorage::EMessageResult result);
    static int TrialOverReturned(void* pParam, int iPad,
                                 C4JStorage::EMessageResult result);
    static int ExitAndJoinFromInvite(void* pParam, int iPad,
                                     C4JStorage::EMessageResult result);
    static int ExitAndJoinFromInviteSaveDialogReturned(
        void* pParam, int iPad, C4JStorage::EMessageResult result);
    static int ExitAndJoinFromInviteAndSaveReturned(
        void* pParam, int iPad, C4JStorage::EMessageResult result);
    static int ExitAndJoinFromInviteDeclineSaveReturned(
        void* pParam, int iPad, C4JStorage::EMessageResult result);
    static int FatalErrorDialogReturned(void* pParam, int iPad,
                                        C4JStorage::EMessageResult result);
    static int WarningTrialTexturePackReturned(
        void* pParam, int iPad, C4JStorage::EMessageResult result);

    JoinFromInviteData m_InviteData;
    bool m_bDebugOptions;  

    
    float m_fTrialTimerStart, mfTrialPausedTime;
    typedef struct TimeInfo {
        time_util::time_point qwTime;
        time_util::clock::duration qwAppTime{};

        float fAppTime;
        float fElapsedTime;
    } TIMEINFO;

    TimeInfo m_Time;

protected:
    static const int MAX_TIPS_GAMETIP = 50;
    static const int MAX_TIPS_TRIVIATIP = 20;
    static TIPSTRUCT m_GameTipA[MAX_TIPS_GAMETIP];
    static TIPSTRUCT m_TriviaTipA[MAX_TIPS_TRIVIATIP];
    static Random* TipRandom;

public:
    void InitialiseTips();
    int GetNextTip();
    int GetHTMLColour(eMinecraftColour colour);
    int GetHTMLColor(eMinecraftColour colour) { return GetHTMLColour(colour); }
    int GetHTMLFontSize(EHTMLFontSize size);
    std::wstring FormatHTMLString(int iPad, const std::wstring& desc,
                                  int shadowColour = 0xFFFFFFFF);
    std::wstring GetActionReplacement(int iPad, unsigned char ucAction);
    std::wstring GetVKReplacement(unsigned int uiVKey);
    std::wstring GetIconReplacement(unsigned int uiIcon);

    float getAppTime() { return m_Time.fAppTime; }
    void UpdateTrialPausedTimer() { mfTrialPausedTime += m_Time.fElapsedTime; }

    static int RemoteSaveThreadProc(void* lpParameter);
    static void ExitGameFromRemoteSave(void* lpParameter);
    static int ExitGameFromRemoteSaveDialogReturned(
        void* pParam, int iPad, C4JStorage::EMessageResult result);

private:
    int m_TipIDA[MAX_TIPS_GAMETIP + MAX_TIPS_TRIVIATIP];
    unsigned int m_uiCurrentTip;
    static int TipsSortFunction(const void* a, const void* b);

    
public:
    
    void AddTerrainFeaturePosition(_eTerrainFeatureType, int, int);
    void ClearTerrainFeaturePosition();
    _eTerrainFeatureType IsTerrainFeature(int x, int z);
    bool GetTerrainFeaturePosition(_eTerrainFeatureType eType, int* pX,
                                   int* pZ);
    std::vector<FEATURE_DATA*> m_vTerrainFeatures;

    static int32_t RegisterMojangData(wchar_t*, PlayerUID, wchar_t*, wchar_t*);
    MOJANG_DATA* GetMojangDataForXuid(PlayerUID xuid);
    static int32_t RegisterConfigValues(wchar_t* pType, int iValue);

    static int32_t RegisterDLCData(wchar_t*, wchar_t*, int, uint64_t, uint64_t,
                                   wchar_t*, unsigned int, int,
                                   wchar_t* pDataFile);
    bool GetDLCFullOfferIDForSkinID(const std::wstring& FirstSkin,
                                    uint64_t* pullVal);
    DLC_INFO* GetDLCInfoForTrialOfferID(uint64_t ullOfferID_Trial);
    DLC_INFO* GetDLCInfoForFullOfferID(uint64_t ullOfferID_Full);

    unsigned int GetDLCCreditsCount();
    SCreditTextItemDef* GetDLCCredits(int iIndex);

    
    void ReadDLCFileFromTMS(int iPad, eTMSAction action,
                            bool bCallback = false);
    void ReadXuidsFileFromTMS(int iPad, eTMSAction action,
                              bool bCallback = false);

    
    void CaptureSaveThumbnail() override = 0;
    void GetSaveThumbnail(std::uint8_t** thumbnailData,
                          unsigned int* thumbnailSize) override = 0;
    void ReleaseSaveThumbnail() override = 0;
    void GetScreenshot(int iPad, std::uint8_t** screenshotData,
                       unsigned int* screenshotSize) override = 0;

    void ReadBannedList(int iPad, eTMSAction action = (eTMSAction)0,
                        bool bCallback = false) override = 0;

private:
    std::vector<SCreditTextItemDef*> vDLCCredits;

    static std::unordered_map<PlayerUID, MOJANG_DATA*> MojangData;
    static std::unordered_map<int, uint64_t>
        DLCTextures_PackID;  
    static std::unordered_map<uint64_t, DLC_INFO*>
        DLCInfo_Trial;  
    static std::unordered_map<uint64_t, DLC_INFO*>
        DLCInfo_Full;  
    static std::unordered_map<std::wstring, uint64_t>
        DLCInfo_SkinName;  
    
    
    

    bool m_bDefaultCapeInstallAttempted;  
                                          

    
    
    bool m_bDLCInstallProcessCompleted;
    bool m_bDLCInstallPending;
    int m_iTotalDLC;
    int m_iTotalDLCInstalled;

public:
    
    
    
    
    
    XUSER_SIGNIN_INFO m_currentSigninInfo[XUSER_MAX_COUNT];

    
    
    
    BANNEDLIST BannedListA[XUSER_MAX_COUNT];

private:
    
    
    

    bool m_bRead_BannedListA[XUSER_MAX_COUNT];
    char m_pszUniqueMapName[14];
    bool m_BanListCheck[XUSER_MAX_COUNT];

public:
    void SetBanListCheck(int iPad, bool bVal) { m_BanListCheck[iPad] = bVal; }
    bool GetBanListCheck(int iPad) { return m_BanListCheck[iPad]; }
    
public:
    void SetAutosaveTimerTime(void);
    bool AutosaveDue(void);
    int64_t SecondsToAutosave();

private:
    time_util::time_point m_uiAutosaveTimer;
    unsigned int m_uiOpacityCountDown[XUSER_MAX_COUNT];

    
    bool m_bNewDLCAvailable;
    bool m_bSeenNewDLCTip;

    
private:
    unsigned int m_uiGameHostSettings;
    static unsigned char m_szPNG[8];

#if defined(_LARGE_WORLDS)
    unsigned int m_GameNewWorldSize;
    bool m_bGameNewWorldSizeUseMoat;
    unsigned int m_GameNewHellScale;
#endif
    unsigned int FromBigEndian(unsigned int uiValue);

public:
    void SetGameHostOption(eGameHostOption eVal, unsigned int uiVal);
    void SetGameHostOption(unsigned int& uiHostSettings, eGameHostOption eVal,
                           unsigned int uiVal);
    unsigned int GetGameHostOption(eGameHostOption eVal);
    unsigned int GetGameHostOption(unsigned int uiHostSettings,
                                   eGameHostOption eVal);

#if defined(_LARGE_WORLDS)
    void SetGameNewWorldSize(unsigned int newSize, bool useMoat) {
        m_GameNewWorldSize = newSize;
        m_bGameNewWorldSizeUseMoat = useMoat;
    }
    unsigned int GetGameNewWorldSize() { return m_GameNewWorldSize; }
    unsigned int GetGameNewWorldSizeUseMoat() {
        return m_bGameNewWorldSizeUseMoat;
    }
    void SetGameNewHellScale(unsigned int newScale) {
        m_GameNewHellScale = newScale;
    }
    unsigned int GetGameNewHellScale() { return m_GameNewHellScale; }
#endif
    void SetResetNether(bool bResetNether) { m_bResetNether = bResetNether; }
    bool GetResetNether() { return m_bResetNether; }
    bool CanRecordStatsAndAchievements();

    
    void GetImageTextData(std::uint8_t* imageData, unsigned int imageBytes,
                          unsigned char* seedText, unsigned int& uiHostOptions,
                          bool& bHostOptionsRead, std::uint32_t& uiTexturePack);
    unsigned int CreateImageTextData(std::uint8_t* textMetadata, int64_t seed,
                                     bool hasSeed, unsigned int uiHostOptions,
                                     unsigned int uiTexturePackId);

    
    GameRuleManager m_gameRules;

public:
    void processSchematics(LevelChunk* levelChunk);
    void processSchematicsLighting(LevelChunk* levelChunk);
    void loadDefaultGameRules();
    std::vector<LevelGenerationOptions*>* getLevelGenerators() {
        return m_gameRules.getLevelGenerators();
    }
    void setLevelGenerationOptions(LevelGenerationOptions* levelGen);
    LevelRuleset* getGameRuleDefinitions() {
        return m_gameRules.getGameRuleDefinitions();
    }
    LevelGenerationOptions* getLevelGenerationOptions() {
        return m_gameRules.getLevelGenerationOptions();
    }
    const wchar_t* GetGameRulesString(const std::wstring& key);

private:
    std::uint8_t m_playerColours[MINECRAFT_NET_MAX_PLAYERS];  
                                                              
    unsigned int m_playerGamePrivileges[MINECRAFT_NET_MAX_PLAYERS];

public:
    void UpdatePlayerInfo(std::uint8_t networkSmallId,
                          int16_t playerColourIndex,
                          unsigned int playerGamePrivileges);
    short GetPlayerColour(std::uint8_t networkSmallId);
    unsigned int GetPlayerPrivileges(std::uint8_t networkSmallId);

    std::wstring getEntityName(eINSTANCEOF type);

    unsigned int AddDLCRequest(eDLCMarketplaceType eContentType,
                               bool bPromote = false);
    bool RetrieveNextDLCContent();
    bool CheckTMSDLCCanStop();
    int dlcOffersReturned(int iOfferC, std::uint32_t dwType, int iPad);
    std::uint32_t GetDLCContentType(eDLCContentType eType) {
        return m_dwContentTypeA[eType];
    }
    eDLCContentType Find_eDLCContentType(std::uint32_t dwType);
    int GetDLCOffersCount() { return m_iDLCOfferC; }
    bool DLCContentRetrieved(eDLCMarketplaceType eType);
    void TickDLCOffersRetrieved();
    void ClearAndResetDLCDownloadQueue();
    bool RetrieveNextTMSPPContent();
    void TickTMSPPFilesRetrieved();
    void ClearTMSPPFilesRetrieved();
    unsigned int AddTMSPPFileTypeRequest(eDLCContentType eType,
                                         bool bPromote = false);
    int GetDLCInfoTexturesOffersCount();

    static int TMSPPFileReturned(void* pParam, int iPad, int iUserData,
                                 C4JStorage::PTMSPP_FILEDATA pFileData,
                                 const char* szFilename);
    DLC_INFO* GetDLCInfoTrialOffer(int iIndex);
    DLC_INFO* GetDLCInfoFullOffer(int iIndex);

    int GetDLCInfoTrialOffersCount();
    int GetDLCInfoFullOffersCount();
    bool GetDLCFullOfferIDForPackID(const int iPackID, uint64_t* pullVal);
    uint64_t GetDLCInfoTexturesFullOffer(int iIndex);

    void SetCorruptSaveDeleted(bool bVal) { m_bCorruptSaveDeleted = bVal; }
    bool GetCorruptSaveDeleted(void) { return m_bCorruptSaveDeleted; }

    void lockSaveNotification();
    void unlockSaveNotification();

private:
    std::mutex m_saveNotificationMutex;
    int m_saveNotificationDepth;
    

    
    std::vector<DLCRequest*> m_DLCDownloadQueue;
    std::vector<TMSPPRequest*> m_TMSPPDownloadQueue;
    static std::uint32_t m_dwContentTypeA[e_Marketplace_MAX];
    int m_iDLCOfferC;
    bool m_bAllDLCContentRetrieved;
    bool m_bAllTMSContentRetrieved;
    bool m_bTickTMSDLCFiles;
    std::mutex csDLCDownloadQueue;
    std::mutex csTMSPPDownloadQueue;
    std::mutex csAdditionalModelParts;
    std::mutex csAdditionalSkinBoxes;
    std::mutex csAnimOverrideBitmask;
    bool m_bCorruptSaveDeleted;

    std::uint32_t m_dwAdditionalModelParts[XUSER_MAX_COUNT];

    std::uint8_t* m_pBannedListFileBuffer;
    unsigned int m_dwBannedListFileSize;

public:
    unsigned int m_dwDLCFileSize;
    std::uint8_t* m_pDLCFileBuffer;

    
    
    
    
    
    

    
    void SetAdditionalSkinBoxes(std::uint32_t dwSkinID, SKIN_BOX* SkinBoxA,
                                unsigned int dwSkinBoxC);
    std::vector<ModelPart*>* SetAdditionalSkinBoxes(
        std::uint32_t dwSkinID, std::vector<SKIN_BOX*>* pvSkinBoxA);
    std::vector<ModelPart*>* GetAdditionalModelParts(std::uint32_t dwSkinID);
    std::vector<SKIN_BOX*>* GetAdditionalSkinBoxes(std::uint32_t dwSkinID);
    void SetAnimOverrideBitmask(std::uint32_t dwSkinID,
                                unsigned int uiAnimOverrideBitmask);
    unsigned int GetAnimOverrideBitmask(std::uint32_t dwSkinID);

    static std::uint32_t getSkinIdFromPath(const std::wstring& skin);
    static std::wstring getSkinPathFromId(std::uint32_t skinId);

    int LoadLocalTMSFile(wchar_t* wchTMSFile) override = 0;
    int LoadLocalTMSFile(wchar_t* wchTMSFile,
                         eFileExtensionType eExt) override = 0;
    void FreeLocalTMSFiles(eTMSFileType eType) override = 0;
    int GetLocalTMSFileIndex(wchar_t* wchTMSFile,
                             bool bFilenameIncludesExtension,
                             eFileExtensionType eEXT) override = 0;

    virtual bool GetTMSGlobalFileListRead() { return true; }
    virtual bool GetTMSDLCInfoRead() { return true; }
    virtual bool GetTMSXUIDsFileRead() { return true; }

    bool GetBanListRead(int iPad) { return m_bRead_BannedListA[iPad]; }
    void SetBanListRead(int iPad, bool bVal) {
        m_bRead_BannedListA[iPad] = bVal;
    }
    void ClearBanList(int iPad) {
        BannedListA[iPad].pBannedList = nullptr;
        BannedListA[iPad].byteCount = 0;
    }

    std::uint32_t GetRequiredTexturePackID() {
        return m_dwRequiredTexturePackID;
    }
    void SetRequiredTexturePackID(std::uint32_t texturePackId) {
        m_dwRequiredTexturePackID = texturePackId;
    }

    virtual void GetFileFromTPD(eTPDFileType eType, std::uint8_t* pbData,
                                unsigned int byteCount, std::uint8_t** ppbData,
                                unsigned int* pByteCount) {
        *ppbData = nullptr;
        *pByteCount = 0;
    }

    
    

private:
    
    std::unordered_map<std::uint32_t, std::vector<ModelPart*>*>
        m_AdditionalModelParts;
    std::unordered_map<std::uint32_t, std::vector<SKIN_BOX*>*>
        m_AdditionalSkinBoxes;
    std::unordered_map<std::uint32_t, unsigned int> m_AnimOverrides;

    bool m_bResetNether;
    std::uint32_t m_dwRequiredTexturePackID;

    
public:
    void LocaleAndLanguageInit();
    void getLocale(std::vector<std::wstring>& vecWstrLocales);
    int get_eMCLang(wchar_t* pwchLocale);
    int get_xcLang(wchar_t* pwchLocale);

    void SetTickTMSDLCFiles(bool bVal);

    std::wstring getFilePath(std::uint32_t packId, std::wstring filename,
                             bool bAddDataFolder,
                             std::wstring mountPoint = L"TPACK:");

private:
    std::unordered_map<int, std::wstring> m_localeA;
    std::unordered_map<std::wstring, int> m_eMCLangA;
    std::unordered_map<std::wstring, int> m_xcLangA;
    std::wstring getRootPath(std::uint32_t packId, bool allowOverride,
                             bool bAddDataFolder, std::wstring mountPoint);

public:
#if defined(_WINDOWS64)
    
#else

#endif
};



