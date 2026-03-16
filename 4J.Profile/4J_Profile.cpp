#include "4J_Profile.h"
#include "../Minecraft.Client/Platform/Common/App_Defines.h"
#include "../Minecraft.Client/Platform/Common/Tutorial/TutorialEnum.h"
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
int s_primaryPad = 0;
int s_lockedProfile = 0;
bool s_profileIsFullVersion = true;
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

void C_4JProfile::Initialise(std::uint32_t dwTitleID,
                             std::uint32_t dwOfferID,
                             unsigned short usProfileVersion,
                             unsigned int uiProfileValuesC,
                             unsigned int uiProfileSettingsC,
                             std::uint32_t* pdwProfileSettingsA,
                             int iGameDefinedDataSizeX4,
                             unsigned int* puiGameDefinedDataChangedBitmask) {
    s_primaryPad = 0;
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
void C_4JProfile::SetTrialTextStringTable(CXuiStringTable* pStringTable,
                                          int iAccept, int iReject) {}
void C_4JProfile::SetTrialAwardText(eAwardType AwardType, int iTitle,
                                    int iText) {}
int C_4JProfile::GetLockedProfile() { return s_lockedProfile; }
void C_4JProfile::SetLockedProfile(int iProf) { s_lockedProfile = iProf; }
bool C_4JProfile::IsSignedIn(int iQuadrant) { return iQuadrant == 0; }
bool C_4JProfile::IsSignedInLive(int iProf) { return IsSignedIn(iProf); }
bool C_4JProfile::IsGuest(int iQuadrant) { return false; }
unsigned int C_4JProfile::RequestSignInUI(
    bool bFromInvite, bool bLocalGame, bool bNoGuestsAllowed,
    bool bMultiplayerSignIn, bool bAddUser,
    int (*Func)(void*, const bool, const int iPad), void* lpParam,
    int iQuadrant) {
    return 0;
}
unsigned int C_4JProfile::DisplayOfflineProfile(
    int (*Func)(void*, const bool, const int iPad), void* lpParam,
    int iQuadrant) {
    return 0;
}
unsigned int C_4JProfile::RequestConvertOfflineToGuestUI(
    int (*Func)(void*, const bool, const int iPad), void* lpParam,
    int iQuadrant) {
    return 0;
}
void C_4JProfile::SetPrimaryPlayerChanged(bool bVal) {}
bool C_4JProfile::QuerySigninStatus(void) { return true; }
void C_4JProfile::GetXUID(int iPad, PlayerUID* pXuid, bool bOnlineXuid) {
    if (pXuid) *pXuid = kFakeXuidBase + static_cast<PlayerUID>(isValidPad(iPad) ? iPad : 0);
}
bool C_4JProfile::AreXUIDSEqual(PlayerUID xuid1, PlayerUID xuid2) {
    return xuid1 == xuid2;
}
bool C_4JProfile::XUIDIsGuest(PlayerUID xuid) { return false; }
bool C_4JProfile::AllowedToPlayMultiplayer(int iProf) { return true; }
bool C_4JProfile::GetChatAndContentRestrictions(int iPad,
                                                bool* pbChatRestricted,
                                                bool* pbContentRestricted,
                                                int* piAge) {
    if (pbChatRestricted) *pbChatRestricted = false;
    if (pbContentRestricted) *pbContentRestricted = false;
    if (piAge) *piAge = 18;
    return true;
}
void C_4JProfile::StartTrialGame() {}
void C_4JProfile::AllowedPlayerCreatedContent(int iPad, bool thisQuadrantOnly,
                                              bool* allAllowed,
                                              bool* friendsAllowed) {
    if (allAllowed) *allAllowed = true;
    if (friendsAllowed) *friendsAllowed = true;
}
bool C_4JProfile::CanViewPlayerCreatedContent(int iPad, bool thisQuadrantOnly,
                                              PPlayerUID pXuids,
                                              unsigned int xuidCount) {
    return true;
}
void C_4JProfile::ShowProfileCard(int iPad, PlayerUID targetUid) {}
bool C_4JProfile::GetProfileAvatar(int iPad,
                                   int (*Func)(void* lpParam,
                                               std::uint8_t* thumbnailData,
                                               unsigned int thumbnailBytes),
                                   void* lpParam) {
    return false;
}
void C_4JProfile::CancelProfileAvatarRequest() {}
int C_4JProfile::GetPrimaryPad() { return s_primaryPad; }
void C_4JProfile::SetPrimaryPad(int iPad) { s_primaryPad = iPad; }

char* C_4JProfile::GetGamertag(int iPad) {
    const int resolvedPad = isValidPad(iPad) ? iPad : 0;
    ensureFakeIdentity(resolvedPad);
    return s_gamertags[resolvedPad];
}
std::wstring C_4JProfile::GetDisplayName(int iPad) {
    const int resolvedPad = isValidPad(iPad) ? iPad : 0;
    ensureFakeIdentity(resolvedPad);
    return s_displayNames[resolvedPad];
}
bool C_4JProfile::IsFullVersion() { return s_profileIsFullVersion; }
void C_4JProfile::SetSignInChangeCallback(void (*Func)(void*, bool,
                                                       unsigned int),
                                          void* lpParam) {}
void C_4JProfile::SetNotificationsCallback(void (*Func)(void*, std::uint32_t,
                                                        unsigned int),
                                           void* lpParam) {}
bool C_4JProfile::RegionIsNorthAmerica(void) { return false; }
bool C_4JProfile::LocaleIsUSorCanada(void) { return false; }
HRESULT C_4JProfile::GetLiveConnectionStatus() { return S_OK; }
bool C_4JProfile::IsSystemUIDisplayed() { return false; }
void C_4JProfile::SetProfileReadErrorCallback(void (*Func)(void*),
                                              void* lpParam) {}
int C_4JProfile::SetDefaultOptionsCallback(int (*Func)(void*, PROFILESETTINGS*,
                                                       const int iPad),
                                           void* lpParam) {
    s_defaultOptionsCallback = Func;
    s_defaultOptionsCallbackParam = lpParam;
    return 0;
}
int C_4JProfile::SetOldProfileVersionCallback(int (*Func)(void*, unsigned char*,
                                                          const unsigned short,
                                                          const int),
                                              void* lpParam) {
    return 0;
}
C_4JProfile::PROFILESETTINGS* C_4JProfile::GetDashboardProfileSettings(
    int iPad) {
    return &s_dashboardSettings[isValidPad(iPad) ? iPad : 0];
}
void C_4JProfile::WriteToProfile(int iQuadrant, bool bGameDefinedDataChanged,
                                 bool bOverride5MinuteLimitOnProfileWrites) {}
void C_4JProfile::ForceQueuedProfileWrites(int iPad) {}
void* C_4JProfile::GetGameDefinedProfileData(int iQuadrant) {
    return isValidPad(iQuadrant) ? s_profileData[iQuadrant] : nullptr;
}
void C_4JProfile::ResetProfileProcessState() {}
void C_4JProfile::Tick(void) {}
void C_4JProfile::RegisterAward(int iAwardNumber, int iGamerconfigID,
                                eAwardType eType, bool bLeaderboardAffected,
                                CXuiStringTable* pStringTable, int iTitleStr,
                                int iTextStr, int iAcceptStr,
                                char* pszThemeName, unsigned int uiThemeSize) {}
int C_4JProfile::GetAwardId(int iAwardNumber) { return 0; }
eAwardType C_4JProfile::GetAwardType(int iAwardNumber) {
    return eAwardType_Achievement;
}
bool C_4JProfile::CanBeAwarded(int iQuadrant, int iAwardNumber) {
    return false;
}
void C_4JProfile::Award(int iQuadrant, int iAwardNumber, bool bForce) {}
bool C_4JProfile::IsAwardsFlagSet(int iQuadrant, int iAward) { return false; }
void C_4JProfile::RichPresenceInit(int iPresenceCount, int iContextCount) {}
void C_4JProfile::RegisterRichPresenceContext(int iGameConfigContextID) {}
void C_4JProfile::SetRichPresenceContextValue(int iPad, int iContextID,
                                              int iVal) {}
void C_4JProfile::SetCurrentGameActivity(int iPad, int iNewPresence,
                                         bool bSetOthersToIdle) {}
void C_4JProfile::DisplayFullVersionPurchase(bool bRequired, int iQuadrant,
                                             int iUpsellParam) {}
void C_4JProfile::SetUpsellCallback(void (*Func)(void* lpParam,
                                                 eUpsellType type,
                                                 eUpsellResponse response,
                                                 int iUserData),
                                    void* lpParam) {}
void C_4JProfile::SetDebugFullOverride(bool bVal) { s_profileIsFullVersion = bVal; }
