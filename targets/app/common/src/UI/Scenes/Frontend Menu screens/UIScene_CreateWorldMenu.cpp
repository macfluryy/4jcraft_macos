
#include "UIScene_CreateWorldMenu.h"

#include <wchar.h>

#include <cstdint>
#include <utility>

#include "platform/PlatformTypes.h"
#include "platform/InputActions.h"
#include "platform/sdl2/Input.h"
#include "platform/sdl2/Profile.h"
#include "app/common/App_Defines.h"
#include "app/common/App_enums.h"
#include "app/common/src/DLC/DLCManager.h"
#include "app/common/src/DLC/DLCPack.h"
#include "app/common/src/Network/GameNetworkManager.h"
#include "app/common/src/UI/All Platforms/UIStructs.h"
#include "app/common/src/UI/Controls/UIControl_Button.h"
#include "app/common/src/UI/Controls/UIControl_CheckBox.h"
#include "app/common/src/UI/Controls/UIControl_Label.h"
#include "app/common/src/UI/Controls/UIControl_Slider.h"
#include "app/common/src/UI/Controls/UIControl_TextInput.h"
#include "app/common/src/UI/Scenes/Frontend Menu screens/IUIScene_StartGame.h"
#include "app/common/src/UI/UILayer.h"
#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"
#include "app/include/NetTypes.h"
#include "util/StringHelpers.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/Options.h"
#include "minecraft/client/skins/DLCTexturePack.h"
#include "minecraft/client/skins/TexturePack.h"
#include "minecraft/client/skins/TexturePackRepository.h"
#include "minecraft/server/MinecraftServer.h"
#include "minecraft/sounds/SoundTypes.h"
#include "minecraft/world/level/LevelSettings.h"
#include "minecraft/world/level/chunk/ChunkSource.h"
#include "strings.h"

#if defined(_WINDOWS64)

#include <windows.h>

#include "../../../../../Windows64/Resource.h"
#endif

#define GAME_CREATE_ONLINE_TIMER_ID 0
#define GAME_CREATE_ONLINE_TIMER_TIME 100

int UIScene_CreateWorldMenu::m_iDifficultyTitleSettingA[4] = {
    IDS_DIFFICULTY_TITLE_PEACEFUL, IDS_DIFFICULTY_TITLE_EASY,
    IDS_DIFFICULTY_TITLE_NORMAL, IDS_DIFFICULTY_TITLE_HARD};

UIScene_CreateWorldMenu::UIScene_CreateWorldMenu(int iPad, void* initData,
                                                 UILayer* parentLayer)
    : IUIScene_StartGame(iPad, parentLayer) {
    
    initialiseMovie();

    m_worldName = app.GetString(IDS_DEFAULT_WORLD_NAME);
    m_seed = L"";

    m_iPad = iPad;

    m_labelWorldName.init(app.GetString(IDS_WORLD_NAME));

    m_editWorldName.init(m_worldName, eControl_EditWorldName);

    m_buttonGamemode.init(app.GetString(IDS_GAMEMODE_SURVIVAL),
                          eControl_GameModeToggle);
    m_buttonMoreOptions.init(app.GetString(IDS_MORE_OPTIONS),
                             eControl_MoreOptions);
    m_buttonCreateWorld.init(app.GetString(IDS_CREATE_NEW_WORLD),
                             eControl_NewWorld);

    m_texturePackList.init(app.GetString(IDS_DLC_MENU_TEXTUREPACKS),
                           eControl_TexturePackList);

    m_labelTexturePackName.init(L"");
    m_labelTexturePackDescription.init(L"");

    wchar_t TempString[256];
    swprintf(TempString, 256, L"%ls: %ls", app.GetString(IDS_SLIDER_DIFFICULTY),
             app.GetString(m_iDifficultyTitleSettingA[app.GetGameSettings(
                 m_iPad, eGameSetting_Difficulty)]));
    m_sliderDifficulty.init(
        TempString, eControl_Difficulty, 0, 3,
        app.GetGameSettings(m_iPad, eGameSetting_Difficulty));

    m_MoreOptionsParams.bGenerateOptions = true;
    m_MoreOptionsParams.bStructures = true;
    m_MoreOptionsParams.bFlatWorld = false;
    m_MoreOptionsParams.iLevelType = e_levelType_Normal;
    m_MoreOptionsParams.bBonusChest = false;
    m_MoreOptionsParams.bPVP = true;
    m_MoreOptionsParams.bTrust = true;
    m_MoreOptionsParams.bFireSpreads = true;
    m_MoreOptionsParams.bHostPrivileges = false;
    m_MoreOptionsParams.bTNT = true;
    m_MoreOptionsParams.iPad = iPad;

    m_MoreOptionsParams.bMobGriefing = true;
    m_MoreOptionsParams.bKeepInventory = false;
    m_MoreOptionsParams.bDoMobSpawning = true;
    m_MoreOptionsParams.bDoMobLoot = true;
    m_MoreOptionsParams.bDoTileDrops = true;
    m_MoreOptionsParams.bNaturalRegeneration = true;
    m_MoreOptionsParams.bDoDaylightCycle = true;

    m_bGameModeCreative = false;
    m_iGameModeId = GameType::SURVIVAL->getId();
    m_pDLCPack = nullptr;
    m_bRebuildTouchBoxes = false;

    m_bMultiplayerAllowed = ProfileManager.IsSignedInLive(m_iPad) &&
                            ProfileManager.AllowedToPlayMultiplayer(m_iPad);
    
    
    bool bGameSetting_Online =
        (app.GetGameSettings(m_iPad, eGameSetting_Online) != 0);
    m_MoreOptionsParams.bOnlineSettingChangedBySystem = false;

    
    
    
    
    
    
    
    
    
    

    
    if (m_bMultiplayerAllowed) {
        m_MoreOptionsParams.bOnlineGame = bGameSetting_Online;
        if (bGameSetting_Online) {
            m_MoreOptionsParams.bInviteOnly =
                app.GetGameSettings(m_iPad, eGameSetting_InviteOnly) != 0;
            m_MoreOptionsParams.bAllowFriendsOfFriends =
                app.GetGameSettings(m_iPad, eGameSetting_FriendsOfFriends) != 0;
        } else {
            m_MoreOptionsParams.bInviteOnly = false;
            m_MoreOptionsParams.bAllowFriendsOfFriends = false;
        }
    } else {
        m_MoreOptionsParams.bOnlineGame = false;
        m_MoreOptionsParams.bInviteOnly = false;
        m_MoreOptionsParams.bAllowFriendsOfFriends = false;
        if (bGameSetting_Online) {
            
            
            m_MoreOptionsParams.bOnlineSettingChangedBySystem = true;
        }
    }

    
    bool bOnlineGame = m_MoreOptionsParams.bOnlineGame;
    m_checkboxOnline.SetEnable(true);

    
    if (ProfileManager.IsSignedInLive(m_iPad) == false) {
        m_checkboxOnline.SetEnable(false);
    }

    if (m_MoreOptionsParams.bOnlineSettingChangedBySystem) {
        m_checkboxOnline.SetEnable(false);
        bOnlineGame = false;
    }

    m_checkboxOnline.init(app.GetString(IDS_ONLINE_GAME), eControl_OnlineGame,
                          bOnlineGame);

    addTimer(GAME_CREATE_ONLINE_TIMER_ID, GAME_CREATE_ONLINE_TIMER_TIME);
#if TO_BE_IMPLEMENTED
    XuiSetTimer(m_hObj, CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID,
                CHECKFORAVAILABLETEXTUREPACKS_TIMER_TIME);
#endif

    
    
    if (app.StartInstallDLCProcess(m_iPad) == true) {
        
        m_bIgnoreInput = true;
    } else {
        m_bIgnoreInput = false;

        Minecraft* pMinecraft = Minecraft::GetInstance();
        int texturePacksCount = pMinecraft->skins->getTexturePackCount();
        for (unsigned int i = 0; i < texturePacksCount; ++i) {
            TexturePack* tp = pMinecraft->skins->getTexturePackByIndex(i);

            std::uint32_t imageBytes = 0;
            std::uint8_t* imageData = tp->getPackIcon(imageBytes);

            if (imageBytes > 0 && imageData) {
                wchar_t imageName[64];
                swprintf(imageName, 64, L"tpack%08x", tp->getId());
                registerSubstitutionTexture(imageName, imageData, imageBytes);
                m_texturePackList.addPack(i, imageName);
                app.DebugPrintf("Adding texture pack %ls at %d\n", imageName,
                                i);
            }
        }

#if TO_BE_IMPLEMENTED
        
        

        DLC_INFO* pDLCInfo = nullptr;

        
        bool bTexturePackAlreadyListed;
        bool bNeedToGetTPD = false;

        for (unsigned int i = 0; i < app.GetDLCInfoTexturesOffersCount(); ++i) {
            bTexturePackAlreadyListed = false;
            uint64_t ull = app.GetDLCInfoTexturesFullOffer(i);
            pDLCInfo = app.GetDLCInfoForFullOfferID(ull);
            for (unsigned int i = 0; i < texturePacksCount; ++i) {
                TexturePack* tp = pMinecraft->skins->getTexturePackByIndex(i);
                if (pDLCInfo->iConfig == tp->getDLCParentPackId()) {
                    bTexturePackAlreadyListed = true;
                }
            }
            if (bTexturePackAlreadyListed == false) {
                
                bNeedToGetTPD = true;

                m_iTexturePacksNotInstalled++;
            }
        }

        if (bNeedToGetTPD == true) {
            
            app.DebugPrintf("+++ Adding TMSPP request for texture pack data\n");
            app.AddTMSPPFileTypeRequest(e_DLC_TexturePackData);
            m_iConfigA = new int[m_iTexturePacksNotInstalled];
            m_iTexturePacksNotInstalled = 0;

            for (unsigned int i = 0; i < app.GetDLCInfoTexturesOffersCount();
                 ++i) {
                bTexturePackAlreadyListed = false;
                uint64_t ull = app.GetDLCInfoTexturesFullOffer(i);
                pDLCInfo = app.GetDLCInfoForFullOfferID(ull);
                for (unsigned int i = 0; i < texturePacksCount; ++i) {
                    TexturePack* tp =
                        pMinecraft->skins->getTexturePackByIndex(i);
                    if (pDLCInfo->iConfig == tp->getDLCParentPackId()) {
                        bTexturePackAlreadyListed = true;
                    }
                }
                if (bTexturePackAlreadyListed == false) {
                    m_iConfigA[m_iTexturePacksNotInstalled++] =
                        pDLCInfo->iConfig;
                }
            }
        }
#endif

        UpdateTexturePackDescription(m_currentTexturePackIndex);

        m_texturePackList.selectSlot(m_currentTexturePackIndex);
    }
}

UIScene_CreateWorldMenu::~UIScene_CreateWorldMenu() {}

void UIScene_CreateWorldMenu::updateTooltips() {
    ui.SetTooltips(DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,
                   IDS_TOOLTIPS_BACK);
}

void UIScene_CreateWorldMenu::updateComponents() {
    m_parentLayer->showComponent(m_iPad, eUIComponent_Panorama, true);
    m_parentLayer->showComponent(m_iPad, eUIComponent_Logo, false);
}

std::wstring UIScene_CreateWorldMenu::getMoviePath() {
    return L"CreateWorldMenu";
}

UIControl* UIScene_CreateWorldMenu::GetMainPanel() {
    return &m_controlMainPanel;
}

void UIScene_CreateWorldMenu::handleDestroy() {
    
}

void UIScene_CreateWorldMenu::tick() {
    UIScene::tick();

    if (m_iSetTexturePackDescription >= 0) {
        UpdateTexturePackDescription(m_iSetTexturePackDescription);
        m_iSetTexturePackDescription = -1;
    }
    if (m_bShowTexturePackDescription) {
        slideLeft();
        m_texturePackDescDisplayed = true;

        m_bShowTexturePackDescription = false;
    }
}

void UIScene_CreateWorldMenu::handleInput(int iPad, int key, bool repeat,
                                          bool pressed, bool released,
                                          bool& handled) {
    if (m_bIgnoreInput) return;

    ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

    switch (key) {
        case ACTION_MENU_CANCEL:
            if (pressed) {
                navigateBack();
                handled = true;
            }
            break;
        case ACTION_MENU_OK:

            

        case ACTION_MENU_UP:
        case ACTION_MENU_DOWN:
        case ACTION_MENU_LEFT:
        case ACTION_MENU_RIGHT:
        case ACTION_MENU_OTHER_STICK_UP:
        case ACTION_MENU_OTHER_STICK_DOWN:
            sendInputToMovie(key, repeat, pressed, released);

            bool bOnlineGame = m_checkboxOnline.IsChecked();
            if (m_MoreOptionsParams.bOnlineGame != bOnlineGame) {
                m_MoreOptionsParams.bOnlineGame = bOnlineGame;

                if (!m_MoreOptionsParams.bOnlineGame) {
                    m_MoreOptionsParams.bInviteOnly = false;
                    m_MoreOptionsParams.bAllowFriendsOfFriends = false;
                }
            }

            handled = true;
            break;
    }
}

void UIScene_CreateWorldMenu::handlePress(F64 controlId, F64 childId) {
    if (m_bIgnoreInput) return;

    
    ui.PlayUISFX(eSFX_Press);

    switch ((int)controlId) {
        case eControl_EditWorldName: {
            m_bIgnoreInput = true;
            InputManager.RequestKeyboard(
                app.GetString(IDS_CREATE_NEW_WORLD), m_editWorldName.getLabel(),
                0, 25,
                [this](bool bRes) -> int {
                    m_bIgnoreInput = false;
                    
                    if (bRes) {
                        std::wstring str =
                            convStringToWstring(InputManager.GetText());
                        if (!str.empty()) {
                            m_editWorldName.setLabel(str);
                            m_worldName = std::move(str);
                        }
                        m_buttonCreateWorld.setEnable(!m_worldName.empty());
                    }
                    return 0;
                },
                C_4JInput::EKeyboardMode_Default);
        } break;
        case eControl_GameModeToggle:
            switch (m_iGameModeId) {
                case 0:  
                    m_buttonGamemode.setLabel(
                        app.GetString(IDS_GAMEMODE_CREATIVE));
                    m_iGameModeId = GameType::CREATIVE->getId();
                    m_bGameModeCreative = true;
                    break;
                case 1:  
                    m_buttonGamemode.setLabel(
                        app.GetString(IDS_GAMEMODE_SURVIVAL));
                    m_iGameModeId = GameType::SURVIVAL->getId();
                    m_bGameModeCreative = false;
                    break;
            };
            break;
        case eControl_MoreOptions:
            ui.NavigateToScene(m_iPad, eUIScene_LaunchMoreOptionsMenu,
                               &m_MoreOptionsParams);
            break;
        case eControl_TexturePackList: {
            UpdateCurrentTexturePack((int)childId);
        } break;
        case eControl_NewWorld: {
            {
                StartSharedLaunchFlow();
            }
            break;
        }
    }
}

void UIScene_CreateWorldMenu::StartSharedLaunchFlow() {
    Minecraft* pMinecraft = Minecraft::GetInstance();
    
    if (m_MoreOptionsParams.dwTexturePack != 0) {
        
        TexturePack* pTexturePack = pMinecraft->skins->getTexturePackById(
            m_MoreOptionsParams.dwTexturePack);

        if (pTexturePack == nullptr) {
#if TO_BE_IMPLEMENTED
            
            
            CXuiCtrl4JList::LIST_ITEM_INFO ListItem;
            
            ListItem = m_pTexturePacksList->GetData(m_currentTexturePackIndex);

            
            
            
            uint64_t ullOfferID_Full;
            app.GetDLCFullOfferIDForPackID(m_MoreOptionsParams.dwTexturePack,
                                           &ullOfferID_Full);

#endif

            unsigned int uiIDA[2];

            uiIDA[0] = IDS_TEXTUREPACK_FULLVERSION;
            
            uiIDA[1] = IDS_CONFIRM_CANCEL;

            
            ui.RequestAlertMessage(IDS_DLC_TEXTUREPACK_NOT_PRESENT_TITLE,
                                   IDS_DLC_TEXTUREPACK_NOT_PRESENT, uiIDA, 2,
                                   ProfileManager.GetPrimaryPad(),
                                   &TexturePackDialogReturned, this);
            return;
        }
    }
    m_bIgnoreInput = true;

    
    
    
    

    
    
    if (!m_MoreOptionsParams.bOnlineSettingChangedBySystem) {
        app.SetGameSettings(m_iPad, eGameSetting_Online,
                            m_MoreOptionsParams.bOnlineGame ? 1 : 0);
    }
    app.SetGameSettings(m_iPad, eGameSetting_InviteOnly,
                        m_MoreOptionsParams.bInviteOnly ? 1 : 0);
    app.SetGameSettings(m_iPad, eGameSetting_FriendsOfFriends,
                        m_MoreOptionsParams.bAllowFriendsOfFriends ? 1 : 0);

    app.CheckGameSettingsChanged(true, m_iPad);

    
    if (m_MoreOptionsParams.dwTexturePack != 0) {
        
        TexturePack* pTexturePack = pMinecraft->skins->getTexturePackById(
            m_MoreOptionsParams.dwTexturePack);
        DLCTexturePack* pDLCTexPack = (DLCTexturePack*)pTexturePack;
        m_pDLCPack = pDLCTexPack->getDLCInfoParentPack();

        
        if (m_pDLCPack &&
            !m_pDLCPack->hasPurchasedFile(DLCManager::e_DLCType_Texture, L"")) {
            

            
            

            











            {
                

#if defined(_WINDOWS64)
                
                unsigned int uiIDA[1];
                uiIDA[0] = IDS_CONFIRM_OK;
                ui.RequestAlertMessage(IDS_WARNING_DLC_TRIALTEXTUREPACK_TITLE,
                                       IDS_USING_TRIAL_TEXUREPACK_WARNING,
                                       uiIDA, 1, m_iPad,
                                       &TrialTexturePackWarningReturned, this);
#endif

                return;
            }
        }
    }
    checkStateAndStartGame();
}

void UIScene_CreateWorldMenu::handleSliderMove(F64 sliderId, F64 currentValue) {
    wchar_t TempString[256];
    int value = (int)currentValue;
    switch ((int)sliderId) {
        case eControl_Difficulty:
            m_sliderDifficulty.handleSliderMove(value);

            app.SetGameSettings(m_iPad, eGameSetting_Difficulty, value);
            swprintf(TempString, 256, L"%ls: %ls",
                     app.GetString(IDS_SLIDER_DIFFICULTY),
                     app.GetString(m_iDifficultyTitleSettingA[value]));
            m_sliderDifficulty.setLabel(TempString);
            break;
    }
}

void UIScene_CreateWorldMenu::handleTimerComplete(int id) {
    switch (id) {
        case GAME_CREATE_ONLINE_TIMER_ID: {
            bool bMultiplayerAllowed =
                ProfileManager.IsSignedInLive(m_iPad) &&
                ProfileManager.AllowedToPlayMultiplayer(m_iPad);

            if (bMultiplayerAllowed != m_bMultiplayerAllowed) {
                if (bMultiplayerAllowed) {
                    bool bGameSetting_Online =
                        (app.GetGameSettings(m_iPad, eGameSetting_Online) != 0);
                    m_MoreOptionsParams.bOnlineGame = bGameSetting_Online;
                    if (bGameSetting_Online) {
                        m_MoreOptionsParams.bInviteOnly =
                            app.GetGameSettings(m_iPad,
                                                eGameSetting_InviteOnly) != 0;
                        m_MoreOptionsParams.bAllowFriendsOfFriends =
                            app.GetGameSettings(
                                m_iPad, eGameSetting_FriendsOfFriends) != 0;
                    } else {
                        m_MoreOptionsParams.bInviteOnly = false;
                        m_MoreOptionsParams.bAllowFriendsOfFriends = false;
                    }
                } else {
                    m_MoreOptionsParams.bOnlineGame = false;
                    m_MoreOptionsParams.bInviteOnly = false;
                    m_MoreOptionsParams.bAllowFriendsOfFriends = false;
                }

                m_checkboxOnline.SetEnable(bMultiplayerAllowed);
                m_checkboxOnline.setChecked(m_MoreOptionsParams.bOnlineGame);

                m_bMultiplayerAllowed = bMultiplayerAllowed;
            }
        } break;
            
    };
}

void UIScene_CreateWorldMenu::handleGainFocus(bool navBack) {
    if (navBack) {
        m_checkboxOnline.setChecked(m_MoreOptionsParams.bOnlineGame);
    }
}


void UIScene_CreateWorldMenu::checkStateAndStartGame() {
    int primaryPad = ProfileManager.GetPrimaryPad();
    bool isSignedInLive = true;
    bool isOnlineGame = m_MoreOptionsParams.bOnlineGame;
    int iPadNotSignedInLive = -1;
    bool isLocalMultiplayerAvailable = app.IsLocalMultiplayerAvailable();

    for (unsigned int i = 0; i < XUSER_MAX_COUNT; i++) {
        if (ProfileManager.IsSignedIn(i) &&
            (i == primaryPad || isLocalMultiplayerAvailable)) {
            if (isSignedInLive && !ProfileManager.IsSignedInLive(i)) {
                
                iPadNotSignedInLive = i;
            }

            isSignedInLive = isSignedInLive && ProfileManager.IsSignedInLive(i);
        }
    }

    
    
    if (isOnlineGame && !isSignedInLive) {
        m_bIgnoreInput = false;
        unsigned int uiIDA[1];
        uiIDA[0] = IDS_CONFIRM_OK;
        ui.RequestAlertMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT,
                               uiIDA, 1, ProfileManager.GetPrimaryPad());
        return;
    }

    unsigned int uiIDA[2];
    if (m_bGameModeCreative == true ||
        m_MoreOptionsParams.bHostPrivileges == true) {
        uiIDA[0] = IDS_CONFIRM_OK;
        uiIDA[1] = IDS_CONFIRM_CANCEL;
        if (m_bGameModeCreative == true) {
            ui.RequestAlertMessage(
                IDS_TITLE_START_GAME, IDS_CONFIRM_START_CREATIVE, uiIDA, 2,
                m_iPad, &UIScene_CreateWorldMenu::ConfirmCreateReturned, this);
        } else {
            ui.RequestAlertMessage(
                IDS_TITLE_START_GAME, IDS_CONFIRM_START_HOST_PRIVILEGES, uiIDA,
                2, m_iPad, &UIScene_CreateWorldMenu::ConfirmCreateReturned,
                this);
        }
    } else {
        
        
        int connectedControllers = 0;
        for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i) {
            if (InputManager.IsPadConnected(i) || ProfileManager.IsSignedIn(i))
                ++connectedControllers;
        }

        
        
        
        
        
        bool noUGC = false;
        bool pccAllowed = true;
        bool pccFriendsAllowed = true;
        bool bContentRestricted = false;

        ProfileManager.AllowedPlayerCreatedContent(
            ProfileManager.GetPrimaryPad(), false, &pccAllowed,
            &pccFriendsAllowed);

        noUGC = !pccAllowed && !pccFriendsAllowed;

        if (isOnlineGame && isSignedInLive &&
            app.IsLocalMultiplayerAvailable()) {
            
            
            
            if (noUGC) {
                m_bIgnoreInput = false;
                ui.RequestUGCMessageBox();
            } else if (bContentRestricted) {
                m_bIgnoreInput = false;
                ui.RequestContentRestrictedMessageBox();
            } else {
                
                
                
                SignInInfo info;
                info.Func = [this](bool bContinue, int pad) {
                    return StartGame_SignInReturned(this, bContinue, pad);
                };
                info.requireOnline = m_MoreOptionsParams.bOnlineGame;
                ui.NavigateToScene(ProfileManager.GetPrimaryPad(),
                                   eUIScene_QuadrantSignin, &info);
            }
        } else {
            if (!pccAllowed && !pccFriendsAllowed) noUGC = true;

            if (isOnlineGame && isSignedInLive && noUGC) {
                m_bIgnoreInput = false;
                ui.RequestUGCMessageBox();
            } else if (isOnlineGame && isSignedInLive && bContentRestricted) {
                m_bIgnoreInput = false;
                ui.RequestContentRestrictedMessageBox();
            } else {
                CreateGame(this, 0);
            }
        }
    }
}



void UIScene_CreateWorldMenu::CreateGame(UIScene_CreateWorldMenu* pClass,
                                         int localUsersMask) {
#if TO_BE_IMPLEMENTED
    
    
    
    XuiKillTimer(pClass->m_hObj, CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID);
#endif

    bool isClientSide =
        ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) &&
        pClass->m_MoreOptionsParams.bOnlineGame;

    bool isPrivate = pClass->m_MoreOptionsParams.bInviteOnly ? true : false;

    
    app.ClearTerrainFeaturePosition();

    
    std::wstring wWorldName = pClass->m_worldName;

    StorageManager.ResetSaveData();
    
    StorageManager.SetSaveTitle((wchar_t*)wWorldName.c_str());

    std::wstring wSeed;
    if (!pClass->m_MoreOptionsParams.seed.empty()) {
        wSeed = pClass->m_MoreOptionsParams.seed;
    } else {
        
        wSeed = L"";
    }

    
    int64_t seedValue = 0;

    NetworkGameInitData* param = new NetworkGameInitData();

    if (wSeed.length() != 0) {
        int64_t value = 0;
        unsigned int len = (unsigned int)wSeed.length();

        
        bool isNumber = true;
        for (unsigned int i = 0; i < len; ++i) {
            if (wSeed.at(i) < L'0' || wSeed.at(i) > L'9') {
                if (!(i == 0 && wSeed.at(i) == L'-')) {
                    isNumber = false;
                    break;
                }
            }
        }

        
        if (isNumber) value = fromWString<int64_t>(wSeed);

        
        
        if (value != 0)
            seedValue = value;
        else {
            int hashValue = 0;
            for (unsigned int i = 0; i < len; ++i)
                hashValue = 31 * hashValue + wSeed.at(i);
            seedValue = hashValue;
        }
    } else {
        param->findSeed =
            true;  
                   
                   
                   
    }

    param->seed = seedValue;
    param->saveData = nullptr;
    param->texturePackId = pClass->m_MoreOptionsParams.dwTexturePack;

    Minecraft* pMinecraft = Minecraft::GetInstance();
    pMinecraft->skins->selectTexturePackById(
        pClass->m_MoreOptionsParams.dwTexturePack);

    app.SetGameHostOption(eGameHostOption_Difficulty,
                          Minecraft::GetInstance()->options->difficulty);
    app.SetGameHostOption(eGameHostOption_FriendsOfFriends,
                          pClass->m_MoreOptionsParams.bAllowFriendsOfFriends);
    app.SetGameHostOption(
        eGameHostOption_Gamertags,
        app.GetGameSettings(pClass->m_iPad, eGameSetting_GamertagsVisible) ? 1
                                                                           : 0);

    app.SetGameHostOption(
        eGameHostOption_BedrockFog,
        app.GetGameSettings(pClass->m_iPad, eGameSetting_BedrockFog) ? 1 : 0);

    app.SetGameHostOption(eGameHostOption_GameType, pClass->m_iGameModeId);
    app.SetGameHostOption(eGameHostOption_LevelType,
                          pClass->m_MoreOptionsParams.iLevelType);
    app.SetGameHostOption(eGameHostOption_Structures,
                          pClass->m_MoreOptionsParams.bStructures);
    app.SetGameHostOption(eGameHostOption_BonusChest,
                          pClass->m_MoreOptionsParams.bBonusChest);

    app.SetGameHostOption(eGameHostOption_PvP,
                          pClass->m_MoreOptionsParams.bPVP);
    app.SetGameHostOption(eGameHostOption_TrustPlayers,
                          pClass->m_MoreOptionsParams.bTrust);
    app.SetGameHostOption(eGameHostOption_FireSpreads,
                          pClass->m_MoreOptionsParams.bFireSpreads);
    app.SetGameHostOption(eGameHostOption_TNT,
                          pClass->m_MoreOptionsParams.bTNT);
    app.SetGameHostOption(eGameHostOption_HostCanFly,
                          pClass->m_MoreOptionsParams.bHostPrivileges);
    app.SetGameHostOption(eGameHostOption_HostCanChangeHunger,
                          pClass->m_MoreOptionsParams.bHostPrivileges);
    app.SetGameHostOption(eGameHostOption_HostCanBeInvisible,
                          pClass->m_MoreOptionsParams.bHostPrivileges);

    app.SetGameHostOption(eGameHostOption_MobGriefing,
                          pClass->m_MoreOptionsParams.bMobGriefing);
    app.SetGameHostOption(eGameHostOption_KeepInventory,
                          pClass->m_MoreOptionsParams.bKeepInventory);
    app.SetGameHostOption(eGameHostOption_DoMobSpawning,
                          pClass->m_MoreOptionsParams.bDoMobSpawning);
    app.SetGameHostOption(eGameHostOption_DoMobLoot,
                          pClass->m_MoreOptionsParams.bDoMobLoot);
    app.SetGameHostOption(eGameHostOption_DoTileDrops,
                          pClass->m_MoreOptionsParams.bDoTileDrops);
    app.SetGameHostOption(eGameHostOption_NaturalRegeneration,
                          pClass->m_MoreOptionsParams.bNaturalRegeneration);
    app.SetGameHostOption(eGameHostOption_DoDaylightCycle,
                          pClass->m_MoreOptionsParams.bDoDaylightCycle);

    app.SetGameHostOption(eGameHostOption_WasntSaveOwner, false);
#if defined(_LARGE_WORLDS)
    app.SetGameHostOption(eGameHostOption_WorldSize,
                          pClass->m_MoreOptionsParams.worldSize +
                              1);  
    pClass->m_MoreOptionsParams.currentWorldSize =
        (EGameHostOptionWorldSize)(pClass->m_MoreOptionsParams.worldSize + 1);
    pClass->m_MoreOptionsParams.newWorldSize =
        (EGameHostOptionWorldSize)(pClass->m_MoreOptionsParams.worldSize + 1);
#endif

    g_NetworkManager.HostGame(localUsersMask, isClientSide, isPrivate,
                              MINECRAFT_NET_MAX_PLAYERS, 0);

    param->settings = app.GetGameHostOption(eGameHostOption_All);

#if defined(_LARGE_WORLDS)
    switch (pClass->m_MoreOptionsParams.worldSize) {
        case 0:
            
            param->xzSize = LEVEL_WIDTH_CLASSIC;
            param->hellScale =
                HELL_LEVEL_SCALE_CLASSIC;  
            break;
        case 1:
            
            param->xzSize = LEVEL_WIDTH_SMALL;
            param->hellScale =
                HELL_LEVEL_SCALE_SMALL;  
            break;
        case 2:
            
            param->xzSize = LEVEL_WIDTH_MEDIUM;
            param->hellScale =
                HELL_LEVEL_SCALE_MEDIUM;  
            break;
        case 3:
            
            param->xzSize = LEVEL_WIDTH_LARGE;
            param->hellScale =
                HELL_LEVEL_SCALE_LARGE;  
            break;
    };
#else
    param->xzSize = LEVEL_MAX_WIDTH;
    param->hellScale = HELL_LEVEL_MAX_SCALE;
#endif

    g_NetworkManager.FakeLocalPlayerJoined();

    LoadingInputParams* loadingParams = new LoadingInputParams();
    loadingParams->func = &CGameNetworkManager::RunNetworkGameThreadProc;
    loadingParams->lpParam = param;

    
    app.SetAutosaveTimerTime();

    UIFullscreenProgressCompletionData* completionData =
        new UIFullscreenProgressCompletionData();
    completionData->bShowBackground = true;
    completionData->bShowLogo = true;
    completionData->type = e_ProgressCompletion_CloseAllPlayersUIScenes;
    completionData->iPad = DEFAULT_XUI_MENU_USER;
    loadingParams->completionData = completionData;

    ui.NavigateToScene(pClass->m_iPad, eUIScene_FullscreenProgress,
                       loadingParams);
}

int UIScene_CreateWorldMenu::StartGame_SignInReturned(void* pParam,
                                                      bool bContinue,
                                                      int iPad) {
    UIScene_CreateWorldMenu* pClass = (UIScene_CreateWorldMenu*)pParam;

    if (bContinue == true) {
        
        if (ProfileManager.IsSignedIn(pClass->m_iPad)) {
            bool isOnlineGame =
                ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) &&
                pClass->m_MoreOptionsParams.bOnlineGame;
            
            int primaryPad = ProfileManager.GetPrimaryPad();
            bool noPrivileges = false;
            int localUsersMask = 0;
            bool isSignedInLive = ProfileManager.IsSignedInLive(primaryPad);
            int iPadNotSignedInLive = -1;
            bool isLocalMultiplayerAvailable =
                app.IsLocalMultiplayerAvailable();

            for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i) {
                if (ProfileManager.IsSignedIn(i) &&
                    ((i == primaryPad) || isLocalMultiplayerAvailable)) {
                    if (isSignedInLive && !ProfileManager.IsSignedInLive(i)) {
                        
                        iPadNotSignedInLive = i;
                    }

                    if (!ProfileManager.AllowedToPlayMultiplayer(i))
                        noPrivileges = true;
                    localUsersMask |=
                        CGameNetworkManager::GetLocalPlayerMask(i);
                    isSignedInLive =
                        isSignedInLive && ProfileManager.IsSignedInLive(i);
                }
            }

            
            
            if (isOnlineGame && !isSignedInLive) {
                pClass->m_bIgnoreInput = false;
                unsigned int uiIDA[1];
                uiIDA[0] = IDS_CONFIRM_OK;
                ui.RequestAlertMessage(IDS_PRO_NOTONLINE_TITLE,
                                       IDS_PRO_NOTONLINE_TEXT, uiIDA, 1,
                                       ProfileManager.GetPrimaryPad());
                return 0;
            }

            
            
            bool noUGC = false;
            bool pccAllowed = true;
            bool pccFriendsAllowed = true;

            ProfileManager.AllowedPlayerCreatedContent(
                ProfileManager.GetPrimaryPad(), false, &pccAllowed,
                &pccFriendsAllowed);
            if (!pccAllowed && !pccFriendsAllowed) noUGC = true;

            if (isOnlineGame && (noPrivileges || noUGC)) {
                if (noUGC) {
                    pClass->m_bIgnoreInput = false;
                    unsigned int uiIDA[1];
                    uiIDA[0] = IDS_CONFIRM_OK;
                    ui.RequestAlertMessage(
                        IDS_FAILED_TO_CREATE_GAME_TITLE,
                        IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_CREATE, uiIDA, 1,
                        ProfileManager.GetPrimaryPad());
                } else {
                    pClass->m_bIgnoreInput = false;
                    unsigned int uiIDA[1];
                    uiIDA[0] = IDS_CONFIRM_OK;
                    ui.RequestAlertMessage(
                        IDS_NO_MULTIPLAYER_PRIVILEGE_TITLE,
                        IDS_NO_MULTIPLAYER_PRIVILEGE_HOST_TEXT, uiIDA, 1,
                        ProfileManager.GetPrimaryPad());
                }
            } else {
                
                
                CreateGame(pClass, localUsersMask);
            }
        }
    } else {
        pClass->m_bIgnoreInput = false;
    }
    return 0;
}

int UIScene_CreateWorldMenu::ConfirmCreateReturned(
    void* pParam, int iPad, C4JStorage::EMessageResult result) {
    UIScene_CreateWorldMenu* pClass = (UIScene_CreateWorldMenu*)pParam;

    if (result == C4JStorage::EMessage_ResultAccept) {
        bool isClientSide =
            ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) &&
            pClass->m_MoreOptionsParams.bOnlineGame;

        
        
        int connectedControllers = 0;
        for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i) {
            if (InputManager.IsPadConnected(i) || ProfileManager.IsSignedIn(i))
                ++connectedControllers;
        }

        if (isClientSide && app.IsLocalMultiplayerAvailable()) {
            
            
            
            SignInInfo info;
            info.Func = [pClass](bool bContinue, int pad) {
                return StartGame_SignInReturned(pClass, bContinue, pad);
            };
            info.requireOnline = pClass->m_MoreOptionsParams.bOnlineGame;
            ui.NavigateToScene(ProfileManager.GetPrimaryPad(),
                               eUIScene_QuadrantSignin, &info);
        } else {
            
            
            bool isClientSide =
                ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) &&
                pClass->m_MoreOptionsParams.bOnlineGame;
            bool noUGC = false;
            bool pccAllowed = true;
            bool pccFriendsAllowed = true;

            ProfileManager.AllowedPlayerCreatedContent(
                ProfileManager.GetPrimaryPad(), false, &pccAllowed,
                &pccFriendsAllowed);
            if (!pccAllowed && !pccFriendsAllowed) noUGC = true;

            if (isClientSide && noUGC) {
                pClass->m_bIgnoreInput = false;
                unsigned int uiIDA[1];
                uiIDA[0] = IDS_CONFIRM_OK;
                ui.RequestAlertMessage(
                    IDS_FAILED_TO_CREATE_GAME_TITLE,
                    IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_CREATE, uiIDA, 1,
                    ProfileManager.GetPrimaryPad());
            } else {
                CreateGame(pClass, 0);
            }
        }
    } else {
        pClass->m_bIgnoreInput = false;
    }
    return 0;
}

void UIScene_CreateWorldMenu::handleTouchBoxRebuild() {
    m_bRebuildTouchBoxes = true;
}
