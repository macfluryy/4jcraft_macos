#include "../../Minecraft.World/Platform/stdafx.h"
#include "../../Minecraft.World/Util/Mth.h"
#include "../../Minecraft.World/Util/StringHelpers.h"
#include "../../Minecraft.World/Util/Random.h"
#include "../../Minecraft.Client/Player/User.h"
#include "../../Minecraft.Client/MinecraftServer.h"
#include "UI.h"
#include "UIScene_MainMenu.h"

Random* UIScene_MainMenu::random = new Random();

int UIScene_MainMenu::eNavigateWhenReady = -1;

UIScene_MainMenu::UIScene_MainMenu(int iPad, void* initData,
                                   UILayer* parentLayer)
    : UIScene(iPad, parentLayer) {
    m_bRunGameChosen = false;
    m_bErrorDialogRunning = false;

    // Setup all the Iggy references we need for this scene
    initialiseMovie();

    parentLayer->addComponent(iPad, eUIComponent_Panorama);
    parentLayer->addComponent(iPad, eUIComponent_Logo);

    m_eAction = eAction_None;
    m_bIgnorePress = false;

    m_buttons[(int)eControl_PlayGame].init(IDS_PLAY_GAME, eControl_PlayGame);

    m_buttons[(int)eControl_Leaderboards].init(IDS_LEADERBOARDS,
                                               eControl_Leaderboards);
    m_buttons[(int)eControl_Achievements].init((UIString)IDS_ACHIEVEMENTS,
                                               eControl_Achievements);
    m_buttons[(int)eControl_HelpAndOptions].init(IDS_HELP_AND_OPTIONS,
                                                 eControl_HelpAndOptions);
    if (ProfileManager.IsFullVersion()) {
        m_bTrialVersion = false;
        m_buttons[(int)eControl_UnlockOrDLC].init(IDS_DOWNLOADABLECONTENT,
                                                  eControl_UnlockOrDLC);
    } else {
        m_bTrialVersion = true;
        m_buttons[(int)eControl_UnlockOrDLC].init(IDS_UNLOCK_FULL_GAME,
                                                  eControl_UnlockOrDLC);
    }

    m_buttons[(int)eControl_Exit].init(app.GetString(IDS_EXIT_GAME),
                                       eControl_Exit);

    doHorizontalResizeCheck();

    m_splash = L"";

    std::wstring filename = L"splashes.txt";
    if (app.hasArchiveFile(filename)) {
        byteArray splashesArray = app.getArchiveFile(filename);
        ByteArrayInputStream bais(splashesArray);
        InputStreamReader isr(&bais);
        BufferedReader br(&isr);

        std::wstring line = L"";
        while (!(line = br.readLine()).empty()) {
            line = trimString(line);
            if (line.length() > 0) {
                m_splashes.push_back(line);
            }
        }

        br.close();
    }

    m_bIgnorePress = false;
    m_bLoadTrialOnNetworkManagerReady = false;

    // 4J Stu - Clear out any loaded game rules
    app.setLevelGenerationOptions(nullptr);

    // 4J Stu - Reset the leaving game flag so that we correctly handle signouts
    // while in the menus
    g_NetworkManager.ResetLeavingGame();

#if TO_BE_IMPLEMENTED
    // Fix for #45154 - Frontend: DLC: Content can only be downloaded from the
    // frontend if you have not joined/exited multiplayer
    XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW);
#endif
}

UIScene_MainMenu::~UIScene_MainMenu() {
    m_parentLayer->removeComponent(eUIComponent_Panorama);
    m_parentLayer->removeComponent(eUIComponent_Logo);
}

void UIScene_MainMenu::updateTooltips() {
    int iX = -1;
    int iA = -1;
    if (!m_bIgnorePress) {
        iA = IDS_TOOLTIPS_SELECT;
    }
    ui.SetTooltips(DEFAULT_XUI_MENU_USER, iA, -1, iX);
}

void UIScene_MainMenu::updateComponents() {
    m_parentLayer->showComponent(m_iPad, eUIComponent_Panorama, true);
    m_parentLayer->showComponent(m_iPad, eUIComponent_Logo, true);
}

void UIScene_MainMenu::handleGainFocus(bool navBack) {
    UIScene::handleGainFocus(navBack);
    ui.ShowPlayerDisplayname(false);
    m_bIgnorePress = false;

    if (eNavigateWhenReady >= 0) {
        return;
    }

    // 4J-JEV: This needs to come before SetLockedProfile(-1) as it wipes the
    // XbLive contexts.
    if (!navBack) {
        for (int iPad = 0; iPad < MAX_LOCAL_PLAYERS; iPad++) {
            // For returning to menus after exiting a game.
            if (ProfileManager.IsSignedIn(iPad)) {
                ProfileManager.SetCurrentGameActivity(
                    iPad, CONTEXT_PRESENCE_MENUS, false);
            }
        }
    }
    ProfileManager.SetLockedProfile(-1);

    m_bIgnorePress = false;
    updateTooltips();

    if (navBack && ProfileManager.IsFullVersion()) {
        // Replace the Unlock Full Game with Downloadable Content
        m_buttons[(int)eControl_UnlockOrDLC].setLabel(IDS_DOWNLOADABLECONTENT);
    }

#if TO_BE_IMPLEMENTED
    // Fix for #45154 - Frontend: DLC: Content can only be downloaded from the
    // frontend if you have not joined/exited multiplayer
    XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW);
    m_Timer.SetShow(false);
#endif
    m_controlTimer.setVisible(false);

    // 4J-PB - remove the "hobo humping" message legal say we can't have, and
    // the 1080p one for Vita
    int splashIndex =
        eSplashRandomStart + 1 +
        random->nextInt((int)m_splashes.size() - (eSplashRandomStart + 1));

    // Override splash text on certain dates
    SYSTEMTIME LocalSysTime;
    GetLocalTime(&LocalSysTime);
    if (LocalSysTime.wMonth == 11 && LocalSysTime.wDay == 9) {
        splashIndex = eSplashHappyBirthdayEx;
    } else if (LocalSysTime.wMonth == 6 && LocalSysTime.wDay == 1) {
        splashIndex = eSplashHappyBirthdayNotch;
    } else if (LocalSysTime.wMonth == 12 &&
               LocalSysTime.wDay == 24)  // the Java game shows this on
                                         // Christmas Eve, so we will too
    {
        splashIndex = eSplashMerryXmas;
    } else if (LocalSysTime.wMonth == 1 && LocalSysTime.wDay == 1) {
        splashIndex = eSplashHappyNewYear;
    }
    // splashIndex = 47; // Very short string
    // splashIndex = 194; // Very long string
    // splashIndex = 295; // Coloured
    // splashIndex = 296; // Noise
    m_splash = m_splashes.at(splashIndex);
}

std::wstring UIScene_MainMenu::getMoviePath() { return L"MainMenu"; }

void UIScene_MainMenu::handleReload() {}

void UIScene_MainMenu::handleInput(int iPad, int key, bool repeat, bool pressed,
                                   bool released, bool& handled) {
    // app.DebugPrintf("UIScene_DebugOverlay handling input for pad %d, key %d,
    // down- %s, pressed- %s, released- %s\n", iPad, key, down?"true":"false",
    // pressed?"true":"false", released?"true":"false");

    if (m_bIgnorePress || (eNavigateWhenReady >= 0)) return;

    ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

    switch (key) {
        case ACTION_MENU_OK:
            if (pressed) {
                ProfileManager.SetPrimaryPad(iPad);
                ProfileManager.SetLockedProfile(-1);
                sendInputToMovie(key, repeat, pressed, released);
            }
            break;

        case ACTION_MENU_UP:
        case ACTION_MENU_DOWN:
            sendInputToMovie(key, repeat, pressed, released);
            break;
    }
}

void UIScene_MainMenu::handlePress(F64 controlId, F64 childId) {
    int primaryPad = ProfileManager.GetPrimaryPad();

    int (*signInReturnedFunc)(void*, const bool, const int iPad) = nullptr;

    switch ((int)controlId) {
        case eControl_PlayGame:
            m_eAction = eAction_RunGame;
            // CD - Added for audio
            ui.PlayUISFX(eSFX_Press);

            signInReturnedFunc = &UIScene_MainMenu::CreateLoad_SignInReturned;
            break;
        case eControl_Leaderboards:
            // CD - Added for audio
            ui.PlayUISFX(eSFX_Press);
            m_eAction = eAction_RunLeaderboards;
            signInReturnedFunc = &UIScene_MainMenu::Leaderboards_SignInReturned;
            break;
        case eControl_Achievements:
            // CD - Added for audio
            ui.PlayUISFX(eSFX_Press);

            m_eAction = eAction_RunAchievements;
            signInReturnedFunc = &UIScene_MainMenu::Achievements_SignInReturned;
            break;
        case eControl_HelpAndOptions:
            // CD - Added for audio
            ui.PlayUISFX(eSFX_Press);

            m_eAction = eAction_RunHelpAndOptions;
            signInReturnedFunc =
                &UIScene_MainMenu::HelpAndOptions_SignInReturned;
            break;
        case eControl_UnlockOrDLC:
            // CD - Added for audio
            ui.PlayUISFX(eSFX_Press);

            m_eAction = eAction_RunUnlockOrDLC;
            signInReturnedFunc =
                &UIScene_MainMenu::UnlockFullGame_SignInReturned;
            break;
        case eControl_Exit:
            if (ProfileManager.IsFullVersion()) {
                unsigned int uiIDA[2];
                uiIDA[0] = IDS_CONFIRM_CANCEL;
                uiIDA[1] = IDS_CONFIRM_OK;
                ui.RequestErrorMessage(
                    IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 2,
                    XUSER_INDEX_ANY, &UIScene_MainMenu::ExitGameReturned, this);
            } else {
                ui.NavigateToScene(primaryPad, eUIScene_TrialExitUpsell);
            }
            break;

        default:
            __debugbreak();
    }

    bool confirmUser = false;

    // Note: if no sign in returned func, assume this isn't required
    if (signInReturnedFunc != nullptr) {
        if (ProfileManager.IsSignedIn(primaryPad)) {
            if (confirmUser) {
                ProfileManager.RequestSignInUI(false, false, true, false, true,
                                               signInReturnedFunc, this,
                                               primaryPad);
            } else {
                RunAction(primaryPad);
            }
        } else {
            // Ask user to sign in
            unsigned int uiIDA[2];
            uiIDA[0] = IDS_CONFIRM_OK;
            uiIDA[1] = IDS_CONFIRM_CANCEL;
            ui.RequestErrorMessage(IDS_MUST_SIGN_IN_TITLE,
                                   IDS_MUST_SIGN_IN_TEXT, uiIDA, 2, primaryPad,
                                   &UIScene_MainMenu::MustSignInReturned, this);
        }
    }
}

// Run current action
void UIScene_MainMenu::RunAction(int iPad) {
    switch (m_eAction) {
        case eAction_RunGame:
            RunPlayGame(iPad);
            break;
        case eAction_RunLeaderboards:
            RunLeaderboards(iPad);
            break;
        case eAction_RunAchievements:
            RunAchievements(iPad);
            break;
        case eAction_RunHelpAndOptions:
            RunHelpAndOptions(iPad);
            break;
        case eAction_RunUnlockOrDLC:
            RunUnlockOrDLC(iPad);
            break;
        default:
            break;
    }
}

void UIScene_MainMenu::customDraw(IggyCustomDrawCallbackRegion* region) {
    if (std::char_traits<char16_t>::compare(region->name, u"Splash", 6) == 0) {
        PIXBeginNamedEvent(0, "Custom draw splash");
        customDrawSplash(region);
        PIXEndNamedEvent();
    }
}

void UIScene_MainMenu::customDrawSplash(IggyCustomDrawCallbackRegion* region) {
    Minecraft* pMinecraft = Minecraft::GetInstance();

    // 4J Stu - Move this to the ctor when the main menu is not the first scene
    // we navigate to
    ScreenSizeCalculator ssc(pMinecraft->options, pMinecraft->width_phys,
                             pMinecraft->height_phys);
    m_fScreenWidth = (float)pMinecraft->width_phys;
    m_fRawWidth = (float)ssc.rawWidth;
    m_fScreenHeight = (float)pMinecraft->height_phys;
    m_fRawHeight = (float)ssc.rawHeight;

    // Setup GDraw, normal game render states and matrices
    CustomDrawData* customDrawRegion = ui.setupCustomDraw(this, region);
    delete customDrawRegion;

    Font* font = pMinecraft->font;

    // build and render with the game call
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glPushMatrix();

    float width = region->x1 - region->x0;
    float height = region->y1 - region->y0;
    float xo = width / 2;
    float yo = height;

    glTranslatef(xo, yo, 0);

    glRotatef(-17, 0, 0, 1);
    float sss = 1.8f - Mth::abs(Mth::sin(System::currentTimeMillis() % 1000 /
                                         1000.0f * PI * 2) *
                                0.1f);
    sss *= (m_fScreenWidth / m_fRawWidth);

    sss = sss * 100 / (font->width(m_splash) + 8 * 4);
    glScalef(sss, sss, sss);
    // drawCenteredString(font, splash, 0, -8, 0xffff00);
    font->drawShadow(m_splash, 0 - (font->width(m_splash)) / 2, -8, 0xffff00);
    glPopMatrix();

    glDisable(GL_RESCALE_NORMAL);

    glEnable(GL_DEPTH_TEST);

    // Finish GDraw and anything else that needs to be finalised
    ui.endCustomDraw(region);
}

int UIScene_MainMenu::MustSignInReturned(void* pParam, int iPad,
                                         C4JStorage::EMessageResult result) {
    UIScene_MainMenu* pClass = (UIScene_MainMenu*)pParam;

    if (result == C4JStorage::EMessage_ResultAccept) {
        // we need to specify local game here to display local and LIVE profiles
        // in the list
        switch (pClass->m_eAction) {
            case eAction_RunGame:
                ProfileManager.RequestSignInUI(
                    false, true, false, false, true,
                    &UIScene_MainMenu::CreateLoad_SignInReturned, pClass, iPad);
                break;
            case eAction_RunHelpAndOptions:
                ProfileManager.RequestSignInUI(
                    false, false, true, false, true,
                    &UIScene_MainMenu::HelpAndOptions_SignInReturned, pClass,
                    iPad);
                break;
            case eAction_RunLeaderboards:
                ProfileManager.RequestSignInUI(
                    false, false, true, false, true,
                    &UIScene_MainMenu::Leaderboards_SignInReturned, pClass,
                    iPad);
                break;
            case eAction_RunAchievements:
                ProfileManager.RequestSignInUI(
                    false, false, true, false, true,
                    &UIScene_MainMenu::Achievements_SignInReturned, pClass,
                    iPad);
                break;
            case eAction_RunUnlockOrDLC:
                ProfileManager.RequestSignInUI(
                    false, false, true, false, true,
                    &UIScene_MainMenu::UnlockFullGame_SignInReturned, pClass,
                    iPad);
                break;
            default:
                break;
        }
    } else {
        pClass->m_bIgnorePress = false;
        // unlock the profile
        ProfileManager.SetLockedProfile(-1);
        for (int i = 0; i < XUSER_MAX_COUNT; i++) {
            // if the user is valid, we should set the presence
            if (ProfileManager.IsSignedIn(i)) {
                ProfileManager.SetCurrentGameActivity(i, CONTEXT_PRESENCE_MENUS,
                                                      false);
            }
        }
    }

    return 0;
}

int UIScene_MainMenu::HelpAndOptions_SignInReturned(void* pParam,
                                                    bool bContinue, int iPad) {
    UIScene_MainMenu* pClass = (UIScene_MainMenu*)pParam;

    if (bContinue) {
        // 4J-JEV: Don't we only need to update rich-presence if the sign-in
        // status changes.
        ProfileManager.SetCurrentGameActivity(iPad, CONTEXT_PRESENCE_MENUS,
                                              false);

#if TO_BE_IMPLEMENTED
        if (app.GetTMSDLCInfoRead())
#endif
        {
            ProfileManager.SetLockedProfile(ProfileManager.GetPrimaryPad());
            proceedToScene(iPad, eUIScene_HelpAndOptionsMenu);
        }
#if TO_BE_IMPLEMENTED
        else {
            // Changing to async TMS calls
            app.SetTMSAction(iPad,
                             eTMSAction_TMSPP_RetrieveFiles_HelpAndOptions);

            // block all input
            pClass->m_bIgnorePress = true;
            // We want to hide everything in this scene and display a timer
            // until we get a completion for the TMS files
            for (int i = 0; i < BUTTONS_MAX; i++) {
                pClass->m_Buttons[i].SetShow(false);
            }

            pClass->updateTooltips();

            pClass->m_Timer.SetShow(true);
        }
#endif
    } else {
        pClass->m_bIgnorePress = false;
        // unlock the profile
        ProfileManager.SetLockedProfile(-1);
        for (int i = 0; i < XUSER_MAX_COUNT; i++) {
            // if the user is valid, we should set the presence
            if (ProfileManager.IsSignedIn(i)) {
                ProfileManager.SetCurrentGameActivity(i, CONTEXT_PRESENCE_MENUS,
                                                      false);
            }
        }
    }

    return 0;
}

int UIScene_MainMenu::CreateLoad_SignInReturned(void* pParam, bool bContinue,
                                                int iPad) {
    UIScene_MainMenu* pClass = (UIScene_MainMenu*)pParam;

    if (bContinue) {
        // 4J-JEV: We only need to update rich-presence if the sign-in status
        // changes.
        ProfileManager.SetCurrentGameActivity(iPad, CONTEXT_PRESENCE_MENUS,
                                              false);

        unsigned int uiIDA[1] = {IDS_OK};

        if (ProfileManager.IsGuest(ProfileManager.GetPrimaryPad())) {
            pClass->m_bIgnorePress = false;
            ui.RequestErrorMessage(IDS_PRO_GUESTPROFILE_TITLE,
                                   IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1);
        } else {
            ProfileManager.SetLockedProfile(ProfileManager.GetPrimaryPad());

            // change the minecraft player name
            Minecraft::GetInstance()->user->name = convStringToWstring(
                ProfileManager.GetGamertag(ProfileManager.GetPrimaryPad()));

            if (ProfileManager.IsFullVersion()) {
                bool bSignedInLive = ProfileManager.IsSignedInLive(iPad);

                // Check if we're signed in to LIVE
                if (bSignedInLive) {
                    // 4J-PB - Need to check for installed DLC
                    if (!app.DLCInstallProcessCompleted())
                        app.StartInstallDLCProcess(iPad);

                    if (ProfileManager.IsGuest(iPad)) {
                        pClass->m_bIgnorePress = false;
                        ui.RequestErrorMessage(IDS_PRO_GUESTPROFILE_TITLE,
                                               IDS_PRO_GUESTPROFILE_TEXT, uiIDA,
                                               1);
                    } else {
                        // 4J Stu - Not relevant to PS3
#if TO_BE_IMPLEMENTED
                        // check if all the TMS files are loaded
                        if (app.GetTMSDLCInfoRead() &&
                            app.GetTMSXUIDsFileRead() &&
                            app.GetBanListRead(iPad)) {
                            if (StorageManager.SetSaveDevice(
                                    &UIScene_MainMenu::DeviceSelectReturned,
                                    pClass) == true) {
                                // save device already selected

                                // ensure we've applied this player's settings
                                app.ApplyGameSettingsChanged(
                                    ProfileManager.GetPrimaryPad());
                                // check for DLC
                                // start timer to track DLC check finished
                                pClass->m_Timer.SetShow(true);
                                XuiSetTimer(pClass->m_hObj,
                                            DLC_INSTALLED_TIMER_ID,
                                            DLC_INSTALLED_TIMER_TIME);
                                // app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_MultiGameJoinLoad);
                            }
                        } else {
                            // Changing to async TMS calls
                            app.SetTMSAction(
                                iPad,
                                eTMSAction_TMSPP_RetrieveFiles_RunPlayGame);

                            // block all input
                            pClass->m_bIgnorePress = true;
                            // We want to hide everything in this scene and
                            // display a timer until we get a completion for the
                            // TMS files
                            for (int i = 0; i < BUTTONS_MAX; i++) {
                                pClass->m_Buttons[i].SetShow(false);
                            }

                            updateTooltips();

                            pClass->m_Timer.SetShow(true);
                        }
#else
                        Minecraft* pMinecraft = Minecraft::GetInstance();
                        pMinecraft->user->name =
                            convStringToWstring(ProfileManager.GetGamertag(
                                ProfileManager.GetPrimaryPad()));

                        // ensure we've applied this player's settings
                        app.ApplyGameSettingsChanged(iPad);

                        proceedToScene(ProfileManager.GetPrimaryPad(),
                                       eUIScene_LoadOrJoinMenu);
#endif
                    }
                } else {
#if TO_BE_IMPLEMENTED
                    // offline
                    ProfileManager.DisplayOfflineProfile(
                        &CScene_Main::CreateLoad_OfflineProfileReturned, pClass,
                        ProfileManager.GetPrimaryPad());
#else
                    app.DebugPrintf(
                        "Offline Profile returned not implemented\n");
                    proceedToScene(ProfileManager.GetPrimaryPad(),
                                   eUIScene_LoadOrJoinMenu);
#endif
                }
            } else {
                // 4J-PB - if this is the trial game, we can't have any
                // networking Can't apply the player's settings here - they
                // haven't come back from the QuerySignInStatud call above yet.
                // Need to let them action in the main loop when they come in
                // ensure we've applied this player's settings
                // app.ApplyGameSettingsChanged(iPad);

                {
                    // go straight in to the trial level
                    LoadTrial();
                }
            }
        }
    } else {
        pClass->m_bIgnorePress = false;

        // unlock the profile
        ProfileManager.SetLockedProfile(-1);
        for (int i = 0; i < XUSER_MAX_COUNT; i++) {
            // if the user is valid, we should set the presence
            if (ProfileManager.IsSignedIn(i)) {
                ProfileManager.SetCurrentGameActivity(i, CONTEXT_PRESENCE_MENUS,
                                                      false);
            }
        }
    }
    return 0;
}

int UIScene_MainMenu::Leaderboards_SignInReturned(void* pParam, bool bContinue,
                                                  int iPad) {
    UIScene_MainMenu* pClass = (UIScene_MainMenu*)pParam;

    if (bContinue) {
        // 4J-JEV: We only need to update rich-presence if the sign-in status
        // changes.
        ProfileManager.SetCurrentGameActivity(iPad, CONTEXT_PRESENCE_MENUS,
                                              false);

        unsigned int uiIDA[1] = {IDS_OK};

        // guests can't look at leaderboards
        if (ProfileManager.IsGuest(ProfileManager.GetPrimaryPad())) {
            pClass->m_bIgnorePress = false;
            ui.RequestErrorMessage(IDS_PRO_GUESTPROFILE_TITLE,
                                   IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1);
        } else if (!ProfileManager.IsSignedInLive(
                       ProfileManager.GetPrimaryPad())) {
            pClass->m_bIgnorePress = false;
            ui.RequestErrorMessage(IDS_PRO_NOTONLINE_TITLE,
                                   IDS_PRO_NOTONLINE_TEXT, uiIDA, 1);
        } else {
            bool bContentRestricted = false;
            if (bContentRestricted) {
                pClass->m_bIgnorePress = false;
#if !defined(_WINDOWS64)
                // we check this for other platforms
                // you can't see leaderboards
                unsigned int uiIDA[1];
                uiIDA[0] = IDS_CONFIRM_OK;
                ui.RequestErrorMessage(IDS_ONLINE_SERVICE_TITLE,
                                       IDS_CONTENT_RESTRICTION, uiIDA, 1,
                                       ProfileManager.GetPrimaryPad());
#endif
            } else {
                ProfileManager.SetLockedProfile(ProfileManager.GetPrimaryPad());
                proceedToScene(ProfileManager.GetPrimaryPad(),
                               eUIScene_LeaderboardsMenu);
            }
        }
    } else {
        pClass->m_bIgnorePress = false;
        // unlock the profile
        ProfileManager.SetLockedProfile(-1);
        for (int i = 0; i < XUSER_MAX_COUNT; i++) {
            // if the user is valid, we should set the presence
            if (ProfileManager.IsSignedIn(i)) {
                ProfileManager.SetCurrentGameActivity(i, CONTEXT_PRESENCE_MENUS,
                                                      false);
            }
        }
    }
    return 0;
}

int UIScene_MainMenu::Achievements_SignInReturned(void* pParam, bool bContinue,
                                                  int iPad) {
    UIScene_MainMenu* pClass = (UIScene_MainMenu*)pParam;

    if (bContinue) {
        pClass->m_bIgnorePress = false;
        // 4J-JEV: We only need to update rich-presence if the sign-in status
        // changes.
        ProfileManager.SetCurrentGameActivity(iPad, CONTEXT_PRESENCE_MENUS,
                                              false);

        XShowAchievementsUI(ProfileManager.GetPrimaryPad());
    } else {
        pClass->m_bIgnorePress = false;
        // unlock the profile
        ProfileManager.SetLockedProfile(-1);
        for (int i = 0; i < XUSER_MAX_COUNT; i++) {
            // if the user is valid, we should set the presence
            if (ProfileManager.IsSignedIn(i)) {
                ProfileManager.SetCurrentGameActivity(i, CONTEXT_PRESENCE_MENUS,
                                                      false);
            }
        }
    }
    return 0;
}

int UIScene_MainMenu::UnlockFullGame_SignInReturned(void* pParam,
                                                    bool bContinue, int iPad) {
    UIScene_MainMenu* pClass = (UIScene_MainMenu*)pParam;

    if (bContinue) {
        // 4J-JEV: We only need to update rich-presence if the sign-in status
        // changes.
        ProfileManager.SetCurrentGameActivity(iPad, CONTEXT_PRESENCE_MENUS,
                                              false);

        pClass->RunUnlockOrDLC(iPad);
    } else {
        pClass->m_bIgnorePress = false;
        // unlock the profile
        ProfileManager.SetLockedProfile(-1);
        for (int i = 0; i < XUSER_MAX_COUNT; i++) {
            // if the user is valid, we should set the presence
            if (ProfileManager.IsSignedIn(i)) {
                ProfileManager.SetCurrentGameActivity(i, CONTEXT_PRESENCE_MENUS,
                                                      false);
            }
        }
    }

    return 0;
}

int UIScene_MainMenu::ExitGameReturned(void* pParam, int iPad,
                                       C4JStorage::EMessageResult result) {
    // UIScene_MainMenu* pClass = (UIScene_MainMenu*)pParam;

    // buttons reversed on this
    if (result == C4JStorage::EMessage_ResultDecline) {
        // XLaunchNewImage(XLAUNCH_KEYWORD_DASH_ARCADE, 0);
        app.ExitGame();
    }

    return 0;
}

void UIScene_MainMenu::RunPlayGame(int iPad) {
    Minecraft* pMinecraft = Minecraft::GetInstance();

    // clear the remembered signed in users so their profiles get read again
    app.ClearSignInChangeUsersMask();

    app.ReleaseSaveThumbnail();

    if (ProfileManager.IsGuest(iPad)) {
        unsigned int uiIDA[1];
        uiIDA[0] = IDS_OK;

        m_bIgnorePress = false;
        ui.RequestErrorMessage(IDS_PRO_GUESTPROFILE_TITLE,
                               IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1);
    } else {
        ProfileManager.SetLockedProfile(iPad);

        // If the player was signed in before selecting play, we'll not have
        // read the profile yet, so query the sign-in status to get this to
        // happen
        ProfileManager.QuerySigninStatus();

        // 4J-PB - Need to check for installed DLC
        if (!app.DLCInstallProcessCompleted()) app.StartInstallDLCProcess(iPad);

        if (ProfileManager.IsFullVersion()) {
            // are we offline?
            bool bSignedInLive = ProfileManager.IsSignedInLive(iPad);

            if (!bSignedInLive) {
                ProfileManager.SetLockedProfile(iPad);
                proceedToScene(ProfileManager.GetPrimaryPad(),
                               eUIScene_LoadOrJoinMenu);
            } else {
#if TO_BE_IMPLEMENTED
                // Check if there is any new DLC
                app.ClearNewDLCAvailable();
                StorageManager.GetAvailableDLCCount(iPad);

                // check if all the TMS files are loaded
                if (app.GetTMSDLCInfoRead() && app.GetTMSXUIDsFileRead() &&
                    app.GetBanListRead(iPad)) {
                    if (StorageManager.SetSaveDevice(
                            &CScene_Main::DeviceSelectReturned, this) == true) {
                        // change the minecraft player name
                        pMinecraft->user->name =
                            convStringToWstring(ProfileManager.GetGamertag(
                                ProfileManager.GetPrimaryPad()));
                        // save device already selected

                        // ensure we've applied this player's settings
                        app.ApplyGameSettingsChanged(iPad);
                        // check for DLC
                        // start timer to track DLC check finished
                        m_Timer.SetShow(true);
                        XuiSetTimer(m_hObj, DLC_INSTALLED_TIMER_ID,
                                    DLC_INSTALLED_TIMER_TIME);
                        // app.NavigateToScene(iPad,eUIScene_MultiGameJoinLoad);
                    }
                } else {
                    // Changing to async TMS calls
                    app.SetTMSAction(
                        iPad, eTMSAction_TMSPP_RetrieveFiles_RunPlayGame);

                    // block all input
                    m_bIgnorePress = true;
                    // We want to hide everything in this scene and display a
                    // timer until we get a completion for the TMS files
                    for (int i = 0; i < BUTTONS_MAX; i++) {
                        m_Buttons[i].SetShow(false);
                    }

                    updateTooltips();

                    m_Timer.SetShow(true);
                }
#else
                pMinecraft->user->name = convStringToWstring(
                    ProfileManager.GetGamertag(ProfileManager.GetPrimaryPad()));

                // ensure we've applied this player's settings
                app.ApplyGameSettingsChanged(iPad);

                proceedToScene(ProfileManager.GetPrimaryPad(),
                               eUIScene_LoadOrJoinMenu);
#endif
            }
        } else {
            // 4J-PB - if this is the trial game, we can't have any networking
            // go straight in to the trial level
            // change the minecraft player name
            Minecraft::GetInstance()->user->name = convStringToWstring(
                ProfileManager.GetGamertag(ProfileManager.GetPrimaryPad()));

            // Can't apply the player's settings here - they haven't come back
            // from the QuerySignInStatud call above yet. Need to let them
            // action in the main loop when they come in ensure we've applied
            // this player's settings
            // app.ApplyGameSettingsChanged(iPad);

            {
                LoadTrial();
            }
        }
    }
}

void UIScene_MainMenu::RunLeaderboards(int iPad) {
    unsigned int uiIDA[1];
    uiIDA[0] = IDS_OK;

    // guests can't look at leaderboards
    if (ProfileManager.IsGuest(iPad)) {
        ui.RequestErrorMessage(IDS_PRO_GUESTPROFILE_TITLE,
                               IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1);
    } else if (!ProfileManager.IsSignedInLive(iPad)) {
        ui.RequestErrorMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT,
                               uiIDA, 1);
    } else {
        // we're supposed to check for parental control restrictions before
        // showing leaderboards The title enforces the user's NP parental
        // control setting for age-based content
        // restriction in network communications.
        // If age restrictions are in place and the user's age does not meet
        // the age restriction of the title's online service content rating
        // (CERO, ESRB, PEGI, etc.), then the title must
        // display a message such as the following and disallow online service
        // for this user.

        bool bContentRestricted = false;
        if (bContentRestricted) {
#if !defined(_WINDOWS64)
            // we check this for other platforms
            // you can't see leaderboards
            unsigned int uiIDA[1];
            uiIDA[0] = IDS_CONFIRM_OK;
            ui.RequestErrorMessage(
                IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1,
                ProfileManager.GetPrimaryPad(), nullptr, this);
#endif
        } else {
            ProfileManager.SetLockedProfile(iPad);
            // If the player was signed in before selecting play, we'll not have
            // read the profile yet, so query the sign-in status to get this to
            // happen
            ProfileManager.QuerySigninStatus();

            proceedToScene(iPad, eUIScene_LeaderboardsMenu);
        }
    }
}
void UIScene_MainMenu::RunUnlockOrDLC(int iPad) {
    unsigned int uiIDA[1];
    uiIDA[0] = IDS_OK;

    // Check if this means downloadable content
    if (ProfileManager.IsFullVersion()) {
        // downloadable content
        if (ProfileManager.IsSignedInLive(iPad)) {
            if (ProfileManager.IsGuest(iPad)) {
                m_bIgnorePress = false;
                ui.RequestErrorMessage(IDS_PRO_GUESTPROFILE_TITLE,
                                       IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1);
            } else {
                // If the player was signed in before selecting play, we'll not
                // have read the profile yet, so query the sign-in status to get
                // this to happen
                ProfileManager.QuerySigninStatus();

                {
                    bool bContentRestricted = false;
                    if (bContentRestricted) {
                        m_bIgnorePress = false;
#if !defined(_WINDOWS64)
                        // we check this for other platforms
                        // you can't see the store
                        unsigned int uiIDA[1];
                        uiIDA[0] = IDS_CONFIRM_OK;
                        ui.RequestErrorMessage(
                            IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION,
                            uiIDA, 1, ProfileManager.GetPrimaryPad(), nullptr,
                            this);
#endif
                    } else {
                        ProfileManager.SetLockedProfile(iPad);
                        proceedToScene(ProfileManager.GetPrimaryPad(),
                                       eUIScene_DLCMainMenu);
                    }
                }

                // read the DLC info from TMS
                /*app.ReadDLCFileFromTMS(iPad);*/

                // We want to navigate to the DLC scene, but block input until
                // we get the DLC file in from TMS Don't navigate - we might
                // have an uplink disconnect
                // app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_DLCMainMenu);
            }
        } else {
            unsigned int uiIDA[1];
            uiIDA[0] = IDS_OK;
            ui.RequestErrorMessage(IDS_PRO_NOTONLINE_TITLE,
                                   IDS_PRO_NOTONLINE_TEXT, uiIDA, 1);
        }
    } else {
        // guests can't buy the game
        if (ProfileManager.IsGuest(iPad)) {
            m_bIgnorePress = false;
            ui.RequestErrorMessage(IDS_UNLOCK_TITLE, IDS_UNLOCK_GUEST_TEXT,
                                   uiIDA, 1, iPad);
        } else if (!ProfileManager.IsSignedInLive(iPad)) {
            unsigned int uiIDA[1];
            uiIDA[0] = IDS_OK;
            ui.RequestErrorMessage(IDS_PRO_NOTONLINE_TITLE,
                                   IDS_PRO_NOTONLINE_TEXT, uiIDA, 1);

        } else {
            // If the player was signed in before selecting play, we'll not have
            // read the profile yet, so query the sign-in status to get this to
            // happen
            ProfileManager.QuerySigninStatus();

            // check that the commerce is in the right state to be able to
            // display the full version purchase - if the user is fast with the
            // trial version, it can still be retrieving the product list
            TelemetryManager->RecordUpsellPresented(
                iPad, eSen_UpsellID_Full_Version_Of_Game, app.m_dwOfferID);
            ProfileManager.DisplayFullVersionPurchase(
                false, iPad, eSen_UpsellID_Full_Version_Of_Game);
        }
    }
}

void UIScene_MainMenu::tick() {
    UIScene::tick();

#if !defined(_ENABLEIGGY) && !defined(ENABLE_JAVA_GUIS)
    // 4jcraft
    {
        static int s_mainMenuTickCount = 0;
        s_mainMenuTickCount++;
        if (s_mainMenuTickCount % 60 == 1) {
            fprintf(stderr, "[MM] tick %d\n", s_mainMenuTickCount);
            fflush(stderr);
        }
        // ~3 seconds at 30fps
        if (s_mainMenuTickCount == 90) {
            fprintf(stderr,
                    "[Linux] Auto-starting trial world from MainMenu after %d "
                    "ticks\n",
                    s_mainMenuTickCount);
            LoadTrial();
            return;
        }
    }
#endif

    if ((eNavigateWhenReady >= 0)) {
        int lockedProfile = ProfileManager.GetLockedProfile();

        {
            app.DebugPrintf("[MainMenu] Navigating away from MainMenu.\n");
            ui.NavigateToScene(lockedProfile, (EUIScene)eNavigateWhenReady);
            eNavigateWhenReady = -1;
        }
    }
}

void UIScene_MainMenu::RunAchievements(int iPad) {
#if TO_BE_IMPLEMENTED
    unsigned int uiIDA[1];
    uiIDA[0] = IDS_OK;

    // guests can't look at achievements
    if (ProfileManager.IsGuest(iPad)) {
        ui.RequestErrorMessage(IDS_PRO_GUESTPROFILE_TITLE,
                               IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1);
    } else {
        XShowAchievementsUI(iPad);
    }
#endif
}

void UIScene_MainMenu::RunHelpAndOptions(int iPad) {
    if (ProfileManager.IsGuest(iPad)) {
        unsigned int uiIDA[1];
        uiIDA[0] = IDS_OK;
        ui.RequestErrorMessage(IDS_PRO_GUESTPROFILE_TITLE,
                               IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1);
    } else {
        // If the player was signed in before selecting play, we'll not have
        // read the profile yet, so query the sign-in status to get this to
        // happen
        ProfileManager.QuerySigninStatus();

#if TO_BE_IMPLEMENTED
        // 4J-PB - You can be offline and still can go into help and options
        if (app.GetTMSDLCInfoRead() || !ProfileManager.IsSignedInLive(iPad))
#endif
        {
            ProfileManager.SetLockedProfile(iPad);
            proceedToScene(iPad, eUIScene_HelpAndOptionsMenu);
        }
#if TO_BE_IMPLEMENTED
        else {
            // Changing to async TMS calls
            app.SetTMSAction(iPad,
                             eTMSAction_TMSPP_RetrieveFiles_HelpAndOptions);

            // block all input
            m_bIgnorePress = true;
            // We want to hide everything in this scene and display a timer
            // until we get a completion for the TMS files
            for (int i = 0; i < BUTTONS_MAX; i++) {
                m_Buttons[i].SetShow(false);
            }

            updateTooltips();

            m_Timer.SetShow(true);
        }
#endif
    }
}

void UIScene_MainMenu::LoadTrial(void) {
    app.SetTutorialMode(true);

    // clear out the app's terrain features list
    app.ClearTerrainFeaturePosition();

    StorageManager.ResetSaveData();

    // Need to set the mode as trial
    ProfileManager.StartTrialGame();

    // No saving in the trial
    StorageManager.SetSaveDisabled(true);
    app.SetGameHostOption(eGameHostOption_WasntSaveOwner, false);

    // Set the global flag, so that we don't disable saving again once the save
    // is complete
    app.SetGameHostOption(eGameHostOption_DisableSaving, 1);

    StorageManager.SetSaveTitle(L"Tutorial");

    // Reset the autosave time
    app.SetAutosaveTimerTime();

    // not online for the trial game
    g_NetworkManager.HostGame(0, false, true, MINECRAFT_NET_MAX_PLAYERS, 0);

    g_NetworkManager.FakeLocalPlayerJoined();

    NetworkGameInitData* param = new NetworkGameInitData();
    param->seed = 0;
    param->saveData = nullptr;
    param->settings = app.GetGameHostOption(eGameHostOption_Tutorial) |
                      app.GetGameHostOption(eGameHostOption_DisableSaving);

    std::vector<LevelGenerationOptions*>* generators = app.getLevelGenerators();
    param->levelGen = generators->at(0);

    LoadingInputParams* loadingParams = new LoadingInputParams();
    loadingParams->func = &CGameNetworkManager::RunNetworkGameThreadProc;
    loadingParams->lpParam = (void*)param;

    UIFullscreenProgressCompletionData* completionData =
        new UIFullscreenProgressCompletionData();
    completionData->bShowBackground = true;
    completionData->bShowLogo = true;
    completionData->type = e_ProgressCompletion_CloseAllPlayersUIScenes;
    completionData->iPad = ProfileManager.GetPrimaryPad();
    loadingParams->completionData = completionData;

    ui.ShowTrialTimer(true);

    ui.NavigateToScene(ProfileManager.GetPrimaryPad(),
                       eUIScene_FullscreenProgress, loadingParams);
}

void UIScene_MainMenu::handleUnlockFullVersion() {
    m_buttons[(int)eControl_UnlockOrDLC].setLabel(IDS_DOWNLOADABLECONTENT,
                                                  true);
}
