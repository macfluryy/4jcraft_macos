#pragma once

// #pragma message("UIStructs.h")

#include <cstdint>

#include "UIEnums.h"

class Container;
class Inventory;
class BrewingStandTileEntity;
class DispenserTileEntity;
class FurnaceTileEntity;
class SignTileEntity;
class LevelGenerationOptions;
class LocalPlayer;
class Merchant;

// 4J Stu - Structs shared by Iggy and Xui scenes.
typedef struct _UIVec2D {
    float x;
    float y;

    _UIVec2D& operator+=(const _UIVec2D& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
} UIVec2D;

// Brewing
typedef struct _BrewingScreenInput {
    std::shared_ptr<Inventory> inventory;
    std::shared_ptr<BrewingStandTileEntity> brewingStand;
    int iPad;
    bool bSplitscreen;
} BrewingScreenInput;

// Chest
typedef struct _ContainerScreenInput {
    std::shared_ptr<Container> inventory;
    std::shared_ptr<Container> container;
    int iPad;
    bool bSplitscreen;
} ContainerScreenInput;

// Dispenser
typedef struct _TrapScreenInput {
    std::shared_ptr<Container> inventory;
    std::shared_ptr<DispenserTileEntity> trap;
    int iPad;
    bool bSplitscreen;
} TrapScreenInput;

// Inventory and creative inventory
typedef struct _InventoryScreenInput {
    std::shared_ptr<LocalPlayer> player;
    bool bNavigateBack;  // If we came here from the crafting screen, go back to
                         // it, rather than closing the xui menus
    int iPad;
    bool bSplitscreen;
} InventoryScreenInput;

// Enchanting
typedef struct _EnchantingScreenInput {
    std::shared_ptr<Inventory> inventory;
    Level* level;
    int x;
    int y;
    int z;
    int iPad;
    bool bSplitscreen;
} EnchantingScreenInput;

// Furnace
typedef struct _FurnaceScreenInput {
    std::shared_ptr<Inventory> inventory;
    std::shared_ptr<FurnaceTileEntity> furnace;
    int iPad;
    bool bSplitscreen;
} FurnaceScreenInput;

// Crafting
typedef struct _CraftingPanelScreenInput {
    std::shared_ptr<LocalPlayer> player;
    int iContainerType;  // RECIPE_TYPE_2x2 or RECIPE_TYPE_3x3
    bool bSplitscreen;
    int iPad;
    int x;
    int y;
    int z;
} CraftingPanelScreenInput;

// Trading
typedef struct _TradingScreenInput {
    std::shared_ptr<Inventory> inventory;
    std::shared_ptr<Merchant> trader;
    Level* level;
    int iPad;
    bool bSplitscreen;
} TradingScreenInput;

// Anvil
typedef struct _AnvilScreenInput {
    std::shared_ptr<Inventory> inventory;
    Level* level;
    int x;
    int y;
    int z;
    int iPad;
    bool bSplitscreen;
} AnvilScreenInput;

// Sign
typedef struct _SignEntryScreenInput {
    std::shared_ptr<SignTileEntity> sign;
    int iPad;
} SignEntryScreenInput;

// Connecting progress
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
        cancelFunc = NULL;
        cancelFuncParam = NULL;
    }
} ConnectionProgressParams;

// Fullscreen progress
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

// Create world
typedef struct _CreateWorldMenuInitData {
    bool bOnline;
    bool bIsPrivate;
    int iPad;
} CreateWorldMenuInitData;

// Join/Load saves list
typedef struct _SaveListDetails {
    int saveId;
    std::uint8_t* pbThumbnailData;
    unsigned int dwThumbnailSize;
#ifdef _DURANGO
    wchar_t UTF16SaveName[128];
    wchar_t UTF16SaveFilename[MAX_SAVEFILENAME_LENGTH];
#else
    char UTF8SaveName[128];
#ifndef _XBOX
    char UTF8SaveFilename[MAX_SAVEFILENAME_LENGTH];
#endif
#endif

    _SaveListDetails() {
        saveId = 0;
        pbThumbnailData = NULL;
        dwThumbnailSize = 0;
#ifdef _DURANGO
        ZeroMemory(UTF16SaveName, sizeof(wchar_t) * 128);
        ZeroMemory(UTF16SaveFilename,
                   sizeof(wchar_t) * MAX_SAVEFILENAME_LENGTH);
#else
        ZeroMemory(UTF8SaveName, 128);
#ifndef _XBOX
        ZeroMemory(UTF8SaveFilename, MAX_SAVEFILENAME_LENGTH);
#endif
#endif
    }

} SaveListDetails;

// Load world
typedef struct _LoadMenuInitData {
    int iPad;
    int iSaveGameInfoIndex;
    LevelGenerationOptions* levelGen;
    SaveListDetails* saveDetails;
} LoadMenuInitData;

// Join Games
typedef struct _JoinMenuInitData {
    FriendSessionInfo* selectedSession;
    int iPad;
} JoinMenuInitData;

// More Options
typedef struct _LaunchMoreOptionsMenuInitData {
    bool bOnlineGame = true;
    bool bInviteOnly = false;
    bool bAllowFriendsOfFriends = true;

    bool bGenerateOptions = false;
    bool bStructures = false;
    bool bFlatWorld = false;
    bool bBonusChest = false;

    bool bPVP = true;
    bool bTrust = false;
    bool bFireSpreads = true;
    bool bTNT = true;

    bool bHostPrivileges = false;
    bool bResetNether = false;

    bool bOnlineSettingChangedBySystem = false;

    int iPad = -1;

    std::uint32_t dwTexturePack = 0;

    std::wstring seed = L"";
    int worldSize = 3;
    bool bDisableSaving = false;
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
        func = NULL;
        lpParam = NULL;
        completionData = NULL;

        cancelText = -1;
        cancelFunc = NULL;
        completeFunc = NULL;
        m_cancelFuncParam = NULL;
        m_completeFuncParam = NULL;
        waitForThreadToDelete = false;
    }
} LoadingInputParams;

// Tutorial
#ifndef _XBOX
class UIScene;
#endif
class Tutorial;
typedef struct _TutorialPopupInfo {
#ifdef _XBOX
    CXuiScene* interactScene;
#else
    UIScene* interactScene;
#endif
    const wchar_t* desc;
    const wchar_t* title;
    int icon;
    int iAuxVal /* = 0 */;
    bool isFoil /* = false */;
    bool allowFade /* = true */;
    bool isReminder /*= false*/;
    Tutorial* tutorial;

    _TutorialPopupInfo() {
        interactScene = NULL;
        desc = L"";
        title = L"";
        icon = -1;
        iAuxVal = 0;
        isFoil = false;
        allowFade = true;
        isReminder = false;
        tutorial = NULL;
    }

} TutorialPopupInfo;

// Quadrant sign in
typedef struct _SignInInfo {
    int (*Func)(void*, const bool, const int iPad);
    void* lpParam;
    bool requireOnline;
} SignInInfo;

// Credits
typedef struct {
    const wchar_t* m_Text;  // Should contain string, optionally with %s to add
                            // in translated string ... e.g. "Andy West - %s"
    int m_iStringID[2];  // May be NO_TRANSLATED_STRING if we do not require to
                         // add any translated string.
    ECreditTextTypes m_eType;
} SCreditTextItemDef;

// Message box
typedef struct _MessageBoxInfo {
    unsigned int uiTitle;
    unsigned int uiText;
    unsigned int* uiOptionA;
    unsigned int uiOptionC;
    unsigned int dwPad;
    int (*Func)(void*, int, const C4JStorage::EMessageResult);
    void* lpParam;
    // C4JStringTable *pStringTable; // 4J Stu - We don't need this for our
    // internal message boxes
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
        y1;  // the bounding box of the original DisplayObject, in object space
    float mat[16];
} CustomDrawData;
