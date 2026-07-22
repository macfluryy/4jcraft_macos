#pragma once



#include <cstdint>
#include <cstring>
#include <functional>

#include "platform/sdl2/Storage.h"
#include "app/common/App_Defines.h"
#include "UIEnums.h"
#include "platform/C4JThread.h"

class Container;
class Inventory;
class BrewingStandTileEntity;
class DispenserTileEntity;
class FurnaceTileEntity;
class SignTileEntity;
class LevelGenerationOptions;
class LocalPlayer;
class Merchant;
class EntityHorse;
class BeaconTileEntity;
class Slot;
class AbstractContainerMenu;
class Level;
class FriendSessionInfo;


typedef struct _UIVec2D {
    float x;
    float y;

    _UIVec2D& operator+=(const _UIVec2D& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
} UIVec2D;


typedef struct _BrewingScreenInput {
    std::shared_ptr<Inventory> inventory;
    std::shared_ptr<BrewingStandTileEntity> brewingStand;
    int iPad;
    bool bSplitscreen;
} BrewingScreenInput;


typedef struct _ContainerScreenInput {
    std::shared_ptr<Container> inventory;
    std::shared_ptr<Container> container;
    int iPad;
    bool bSplitscreen;
} ContainerScreenInput;


typedef struct _TrapScreenInput {
    std::shared_ptr<Container> inventory;
    std::shared_ptr<DispenserTileEntity> trap;
    int iPad;
    bool bSplitscreen;
} TrapScreenInput;


typedef struct _InventoryScreenInput {
    std::shared_ptr<LocalPlayer> player;
    bool bNavigateBack;  
                         
    int iPad;
    bool bSplitscreen;
} InventoryScreenInput;


typedef struct _EnchantingScreenInput {
    std::shared_ptr<Inventory> inventory;
    Level* level;
    int x;
    int y;
    int z;
    int iPad;
    bool bSplitscreen;
    std::wstring name;
} EnchantingScreenInput;


typedef struct _FurnaceScreenInput {
    std::shared_ptr<Inventory> inventory;
    std::shared_ptr<FurnaceTileEntity> furnace;
    int iPad;
    bool bSplitscreen;
} FurnaceScreenInput;


typedef struct _CraftingPanelScreenInput {
    std::shared_ptr<LocalPlayer> player;
    int iContainerType;  
    bool bSplitscreen;
    int iPad;
    int x;
    int y;
    int z;
} CraftingPanelScreenInput;


typedef struct _FireworksScreenInput {
    std::shared_ptr<LocalPlayer> player;
    bool bSplitscreen;
    int iPad;
    int x;
    int y;
    int z;
} FireworksScreenInput;


typedef struct _TradingScreenInput {
    std::shared_ptr<Inventory> inventory;
    std::shared_ptr<Merchant> trader;
    Level* level;
    int iPad;
    bool bSplitscreen;
} TradingScreenInput;


typedef struct _AnvilScreenInput {
    std::shared_ptr<Inventory> inventory;
    Level* level;
    int x;
    int y;
    int z;
    int iPad;
    bool bSplitscreen;
} AnvilScreenInput;


typedef struct _HopperScreenInput {
    std::shared_ptr<Inventory> inventory;
    std::shared_ptr<Container> hopper;
    int iPad;
    bool bSplitscreen;
} HopperScreenInput;


typedef struct _HorseScreenInput {
    std::shared_ptr<Inventory> inventory;
    std::shared_ptr<Container> container;
    std::shared_ptr<EntityHorse> horse;
    int iPad;
    bool bSplitscreen;
} HorseScreenInput;


typedef struct _BeaconScreenInput {
    std::shared_ptr<Inventory> inventory;
    std::shared_ptr<BeaconTileEntity> beacon;
    int iPad;
    bool bSplitscreen;
} BeaconScreenInput;


typedef struct _SignEntryScreenInput {
    std::shared_ptr<SignTileEntity> sign;
    int iPad;
} SignEntryScreenInput;


typedef struct _ConnectionProgressParams {
    int iPad;
    int stringId;
    bool showTooltips;
    bool setFailTimer;
    int timerTime;
    void (*cancelFunc)(void* param);
    void* cancelFuncParam;

    _ConnectionProgressParams() {
        iPad = 0;
        stringId = -1;
        showTooltips = false;
        setFailTimer = false;
        timerTime = 0;
        cancelFunc = nullptr;
        cancelFuncParam = nullptr;
    }
} ConnectionProgressParams;


typedef struct _UIFullscreenProgressCompletionData {
    bool bRequiresUserAction;
    bool bShowBackground;
    bool bShowLogo;
    bool bShowTips;
    ProgressionCompletionType type;
    int iPad;
    EUIScene scene;

    _UIFullscreenProgressCompletionData() {
        bRequiresUserAction = false;
        bShowBackground = true;
        bShowLogo = true;
        bShowTips = true;
        type = e_ProgressCompletion_NoAction;
    }
} UIFullscreenProgressCompletionData;


typedef struct _CreateWorldMenuInitData {
    bool bOnline;
    bool bIsPrivate;
    int iPad;
} CreateWorldMenuInitData;


typedef struct _SaveListDetails {
    int saveId;
    std::uint8_t* pbThumbnailData;
    unsigned int dwThumbnailSize;
    char UTF8SaveName[128];
    char UTF8SaveFilename[MAX_SAVEFILENAME_LENGTH];

    _SaveListDetails() {
        saveId = 0;
        pbThumbnailData = nullptr;
        dwThumbnailSize = 0;
        std::memset(UTF8SaveName, 0, 128);
        std::memset(UTF8SaveFilename, 0, MAX_SAVEFILENAME_LENGTH);
    }

} SaveListDetails;


typedef struct _LoadMenuInitData {
    int iPad;
    int iSaveGameInfoIndex;
    LevelGenerationOptions* levelGen;
    SaveListDetails* saveDetails;
} LoadMenuInitData;


typedef struct _JoinMenuInitData {
    FriendSessionInfo* selectedSession;
    int iPad;
} JoinMenuInitData;


typedef struct _LaunchMoreOptionsMenuInitData {
    bool bOnlineGame;
    bool bInviteOnly;
    bool bAllowFriendsOfFriends;

    bool bGenerateOptions;
    bool bStructures;
    bool bFlatWorld;
    int iLevelType;
    bool bBonusChest;

    bool bPVP;
    bool bTrust;
    bool bFireSpreads;
    bool bTNT;

    bool bHostPrivileges;
    bool bResetNether;

    bool bMobGriefing;
    bool bKeepInventory;
    bool bDoMobSpawning;
    bool bDoMobLoot;
    bool bDoTileDrops;
    bool bNaturalRegeneration;
    bool bDoDaylightCycle;

    bool bOnlineSettingChangedBySystem;

    int iPad;

    uint32_t dwTexturePack;

    std::wstring seed;
    int worldSize;
    bool bDisableSaving;

    EGameHostOptionWorldSize currentWorldSize;
    EGameHostOptionWorldSize newWorldSize;
    bool newWorldSizeOverwriteEdges;

    _LaunchMoreOptionsMenuInitData() {
        bOnlineGame = true;
        bInviteOnly = false;
        bAllowFriendsOfFriends = true;
        bGenerateOptions = false;
        bStructures = false;
        bFlatWorld = false;
        iLevelType = e_levelType_Normal;
        bBonusChest = false;
        bPVP = true;
        bTrust = false;
        bFireSpreads = true;
        bTNT = false;
        bHostPrivileges = false;
        bResetNether = false;
        bMobGriefing = true;
        bKeepInventory = false;
        bDoMobSpawning = false;
        bDoMobLoot = true;
        bDoTileDrops = true;
        bNaturalRegeneration = true;
        bDoDaylightCycle = true;
        bOnlineSettingChangedBySystem = false;

        iPad = 0;

        dwTexturePack = 0;

        worldSize = 3;
        seed = L"";
        bDisableSaving = false;

        currentWorldSize = e_worldSize_Unknown;
        newWorldSize = e_worldSize_Unknown;
        newWorldSizeOverwriteEdges = false;
    }
} LaunchMoreOptionsMenuInitData;

typedef struct _LoadingInputParams {
    C4JThreadStartFunc* func;
    void* lpParam;
    UIFullscreenProgressCompletionData* completionData;

    int cancelText;
    void (*cancelFunc)(void* param);
    void (*completeFunc)(void* param);
    void* m_cancelFuncParam;
    void* m_completeFuncParam;
    bool waitForThreadToDelete;

    _LoadingInputParams() {
        func = nullptr;
        lpParam = nullptr;
        completionData = nullptr;

        cancelText = -1;
        cancelFunc = nullptr;
        completeFunc = nullptr;
        m_cancelFuncParam = nullptr;
        m_completeFuncParam = nullptr;
        waitForThreadToDelete = false;
    }
} LoadingInputParams;


class UIScene;
class Tutorial;
typedef struct _TutorialPopupInfo {
    UIScene* interactScene;
    const wchar_t* desc;
    const wchar_t* title;
    int icon;
    int iAuxVal ;
    bool isFoil ;
    bool allowFade ;
    bool isReminder ;
    Tutorial* tutorial;

    _TutorialPopupInfo() {
        interactScene = nullptr;
        desc = L"";
        title = L"";
        icon = -1;
        iAuxVal = 0;
        isFoil = false;
        allowFade = true;
        isReminder = false;
        tutorial = nullptr;
    }

} TutorialPopupInfo;


typedef struct _SignInInfo {
    std::function<int(bool, int)> Func;
    bool requireOnline;
} SignInInfo;


typedef struct {
    const wchar_t* m_Text;  
                            
    int m_iStringID[2];  
                         
    ECreditTextTypes m_eType;
} SCreditTextItemDef;


typedef struct _MessageBoxInfo {
    uint32_t uiTitle;
    uint32_t uiText;
    uint32_t* uiOptionA;
    uint32_t uiOptionC;
    uint32_t dwPad;
    int (*Func)(void*, int, const C4JStorage::EMessageResult);
    void* lpParam;
    
    
    wchar_t* pwchFormatString;
    unsigned int dwFocusButton;
} MessageBoxInfo;

typedef struct _DLCOffersParam {
    int iPad;
    int iOfferC;
    int iType;
} DLCOffersParam;

typedef struct _InGamePlayerOptionsInitData {
    int iPad;
    std::uint8_t networkSmallId;
    unsigned int playerPrivileges;
} InGamePlayerOptionsInitData;

typedef struct _DebugSetCameraPosition {
    int player;
    double m_camX, m_camY, m_camZ, m_yRot, m_elev;
} DebugSetCameraPosition;

typedef struct _TeleportMenuInitData {
    int iPad;
    bool teleportToPlayer;
} TeleportMenuInitData;

typedef struct _CustomDrawData {
    float x0, y0, x1,
        y1;  
    float mat[16];
} CustomDrawData;

typedef struct _ItemEditorInput {
    int iPad;
    Slot* slot;
    AbstractContainerMenu* menu;
} ItemEditorInput;