#include "4J_Profile.h"
#include "../4J.Common/4J_ProfileConstants.h"
#include "../4J.Input/4J_Input.h"
#include <cstdio>
#include <cstring>

C_4JProfile ProfileManager;

namespace {
constexpr PlayerUID kFakeXuidBase = 0xe000d45248242f2eULL;

struct ProfileGameSettings {
    bool bSettingsChanged;
    unsigned char ucMusicVolume;
    unsigned char ucSoundFXVolume;
    unsigned char ucSensitivity;
    unsigned char ucGamma;
    unsigned char ucPad01;
    unsigned short usBitmaskValues;
    unsigned int uiDebugBitmask;
    union {
        struct {
            unsigned char ucTutorialCompletion[TUTORIAL_PROFILE_STORAGE_BYTES];
            std::uint32_t dwSelectedSkin;
            unsigned char ucMenuSensitivity;
            unsigned char ucInterfaceOpacity;
            unsigned char ucPad02;
            unsigned char usPad03;
            unsigned int uiBitmaskValues;
            unsigned int uiSpecialTutorialBitmask;
            std::uint32_t dwSelectedCape;
            unsigned int uiFavoriteSkinA[MAX_FAVORITE_SKINS];
            unsigned char ucCurrentFavoriteSkinPos;
            unsigned int uiMashUpPackWorldsDisplay;
            unsigned char ucLanguage;
        };

        unsigned char ucReservedSpace[192];
    };
};

static_assert(sizeof(ProfileGameSettings) == 204,
              "ProfileGameSettings must match GAME_SETTINGS profile storage");

void* s_profileData[XUSER_MAX_COUNT] = {};
C_4JProfile::PROFILESETTINGS s_dashboardSettings[XUSER_MAX_COUNT] = {};
char s_gamertags[XUSER_MAX_COUNT][16] = {};
std::wstring s_displayNames[XUSER_MAX_COUNT];
int s_lockedProfile = 0;
int (*s_defaultOptionsCallback)(void*, C_4JProfile::PROFILESETTINGS*,
                                const int iPad) = nullptr;
void* s_defaultOptionsCallbackParam = nullptr;

bool isValidPad(int iPad) { return iPad >= 0 && iPad < XUSER_MAX_COUNT; }

void ensureFakeIdentity(int iPad) {
    if (!isValidPad(iPad) || s_gamertags[iPad][0] != '\0') {
        return;
    }

    std::snprintf(s_gamertags[iPad], sizeof(s_gamertags[iPad]), "Player%d",
                  iPad + 1);
    s_displayNames[iPad] = std::wstring(L"Player") + std::to_wstring(iPad + 1);
}

void initialiseDefaultGameSettings(ProfileGameSettings* gameSettings) {
    gameSettings->ucMenuSensitivity = 100;
    gameSettings->ucInterfaceOpacity = 80;
    gameSettings->usBitmaskValues |= 0x0200;
    gameSettings->usBitmaskValues |= 0x0400;
    gameSettings->usBitmaskValues |= 0x1000;
    gameSettings->usBitmaskValues |= 0x8000;
    gameSettings->uiBitmaskValues = 0L;
    gameSettings->uiBitmaskValues |= GAMESETTING_CLOUDS;
    gameSettings->uiBitmaskValues |= GAMESETTING_ONLINE;
    gameSettings->uiBitmaskValues |= GAMESETTING_FRIENDSOFFRIENDS;
    gameSettings->uiBitmaskValues |= GAMESETTING_DISPLAYUPDATEMSG;
    gameSettings->uiBitmaskValues &= ~GAMESETTING_BEDROCKFOG;
    gameSettings->uiBitmaskValues |= GAMESETTING_DISPLAYHUD;
    gameSettings->uiBitmaskValues |= GAMESETTING_DISPLAYHAND;
    gameSettings->uiBitmaskValues |= GAMESETTING_CUSTOMSKINANIM;
    gameSettings->uiBitmaskValues |= GAMESETTING_DEATHMESSAGES;
    gameSettings->uiBitmaskValues |= (GAMESETTING_UISIZE & 0x00000800);
    gameSettings->uiBitmaskValues |=
        (GAMESETTING_UISIZE_SPLITSCREEN & 0x00004000);
    gameSettings->uiBitmaskValues |= GAMESETTING_ANIMATEDCHARACTER;

    for (int i = 0; i < MAX_FAVORITE_SKINS; ++i) {
        gameSettings->uiFavoriteSkinA[i] = 0xFFFFFFFF;
    }

    gameSettings->ucCurrentFavoriteSkinPos = 0;
    gameSettings->uiMashUpPackWorldsDisplay = 0xFFFFFFFF;
    gameSettings->uiBitmaskValues &= ~GAMESETTING_PS3EULAREAD;
    gameSettings->ucLanguage = MINECRAFT_LANGUAGE_DEFAULT;
    gameSettings->uiBitmaskValues &= ~GAMESETTING_PSVITANETWORKMODEADHOC;
    gameSettings->ucTutorialCompletion[0] = 0xFF;
    gameSettings->ucTutorialCompletion[1] = 0xFF;
    gameSettings->ucTutorialCompletion[2] = 0x0F;
    gameSettings->ucTutorialCompletion[28] |= 1 << 0;
}
}  // namespace

void C_4JProfile::Initialise(std::uint32_t, std::uint32_t, unsigned short,
                             unsigned int, unsigned int, std::uint32_t*,
                             int iGameDefinedDataSizeX4, unsigned int*) {
    s_lockedProfile = 0;
    std::memset(s_dashboardSettings, 0, sizeof(s_dashboardSettings));

    for (int i = 0; i < XUSER_MAX_COUNT; ++i) {
        delete[] static_cast<unsigned char*>(s_profileData[i]);
        s_profileData[i] = new unsigned char[iGameDefinedDataSizeX4 / 4];
        std::memset(s_profileData[i], 0, iGameDefinedDataSizeX4 / 4);
        initialiseDefaultGameSettings(
            static_cast<ProfileGameSettings*>(s_profileData[i]));
        ensureFakeIdentity(i);
    }
}

int C_4JProfile::GetLockedProfile() { return s_lockedProfile; }
void C_4JProfile::SetLockedProfile(int iProf) { s_lockedProfile = iProf; }
bool C_4JProfile::IsSignedIn(int iQuadrant) { return iQuadrant == 0; }
bool C_4JProfile::IsSignedInLive(int iProf) { return IsSignedIn(iProf); }
bool C_4JProfile::IsGuest(int) { return false; }
bool C_4JProfile::QuerySigninStatus() { return true; }

void C_4JProfile::GetXUID(int iPad, PlayerUID* pXuid, bool) {
    if (pXuid)
        *pXuid =
            kFakeXuidBase + static_cast<PlayerUID>(isValidPad(iPad) ? iPad : 0);
}

bool C_4JProfile::AreXUIDSEqual(PlayerUID xuid1, PlayerUID xuid2) {
    return xuid1 == xuid2;
}

bool C_4JProfile::XUIDIsGuest(PlayerUID) { return false; }
bool C_4JProfile::AllowedToPlayMultiplayer(int) { return true; }

bool C_4JProfile::GetChatAndContentRestrictions(int, bool* pbChatRestricted,
                                                bool* pbContentRestricted,
                                                int* piAge) {
    if (pbChatRestricted) *pbChatRestricted = false;
    if (pbContentRestricted) *pbContentRestricted = false;
    if (piAge) *piAge = 18;
    return true;
}

char* C_4JProfile::GetGamertag(int iPad) {
    const int p = isValidPad(iPad) ? iPad : 0;
    ensureFakeIdentity(p);
    return s_gamertags[p];
}

std::wstring C_4JProfile::GetDisplayName(int iPad) {
    const int p = isValidPad(iPad) ? iPad : 0;
    ensureFakeIdentity(p);
    return s_displayNames[p];
}

int C_4JProfile::SetDefaultOptionsCallback(int (*Func)(void*, PROFILESETTINGS*,
                                                       const int iPad),
                                           void* lpParam) {
    s_defaultOptionsCallback = Func;
    s_defaultOptionsCallbackParam = lpParam;
    return 0;
}

C_4JProfile::PROFILESETTINGS* C_4JProfile::GetDashboardProfileSettings(
    int iPad) {
    return &s_dashboardSettings[isValidPad(iPad) ? iPad : 0];
}

void* C_4JProfile::GetGameDefinedProfileData(int iQuadrant) {
    return isValidPad(iQuadrant) ? s_profileData[iQuadrant] : nullptr;
}

void C_4JProfile::AllowedPlayerCreatedContent(int, bool, bool* allAllowed,
                                              bool* friendsAllowed) {
    if (allAllowed) *allAllowed = true;
    if (friendsAllowed) *friendsAllowed = true;
}

bool C_4JProfile::CanViewPlayerCreatedContent(int, bool, PlayerUID*,
                                              unsigned int) {
    return true;
}

// GetPrimaryPad/SetPrimaryPad — delegates to InputManager.
// Kept here temporarily for call sites that still use ProfileManager.
// These forward to the canonical copies in C_4JInput.
int C_4JProfile::GetPrimaryPad() {
    extern C_4JInput InputManager;
    return InputManager.GetPrimaryPad();
}
void C_4JProfile::SetPrimaryPad(int iPad) {
    extern C_4JInput InputManager;
    InputManager.SetPrimaryPad(iPad);
}
