#include "4J_Profile.h"
#include <cstring>

C_4JProfile ProfileManager;

static void *s_profileData[4] = {};

void C_4JProfile::Initialise(DWORD dwTitleID, DWORD dwOfferID, unsigned short usProfileVersion,
    UINT uiProfileValuesC, UINT uiProfileSettingsC, DWORD *pdwProfileSettingsA,
    int iGameDefinedDataSizeX4, unsigned int *puiGameDefinedDataChangedBitmask)
{
    for (int i = 0; i < 4; i++)
    {
        s_profileData[i] = new unsigned char[iGameDefinedDataSizeX4 / 4];
        memset(s_profileData[i], 0, iGameDefinedDataSizeX4 / 4);
    }
}
void C_4JProfile::SetTrialTextStringTable(CXuiStringTable *pStringTable, int iAccept, int iReject) {}
void C_4JProfile::SetTrialAwardText(eAwardType AwardType, int iTitle, int iText) {}
int C_4JProfile::GetLockedProfile() { return -1; }
void C_4JProfile::SetLockedProfile(int iProf) {}
bool C_4JProfile::IsSignedIn(int iQuadrant) { return iQuadrant == 0; }
bool C_4JProfile::IsSignedInLive(int iProf) { return false; }
bool C_4JProfile::IsGuest(int iQuadrant) { return false; }
UINT C_4JProfile::RequestSignInUI(bool bFromInvite, bool bLocalGame, bool bNoGuestsAllowed, bool bMultiplayerSignIn, bool bAddUser, int(*Func)(void *, const bool, const int iPad), void *lpParam, int iQuadrant) { return 0; }
UINT C_4JProfile::DisplayOfflineProfile(int(*Func)(void *, const bool, const int iPad), void *lpParam, int iQuadrant) { return 0; }
UINT C_4JProfile::RequestConvertOfflineToGuestUI(int(*Func)(void *, const bool, const int iPad), void *lpParam, int iQuadrant) { return 0; }
void C_4JProfile::SetPrimaryPlayerChanged(bool bVal) {}
bool C_4JProfile::QuerySigninStatus(void) { return true; }
void C_4JProfile::GetXUID(int iPad, PlayerUID *pXuid, bool bOnlineXuid) { if (pXuid) *pXuid = 0; }
BOOL C_4JProfile::AreXUIDSEqual(PlayerUID xuid1, PlayerUID xuid2) { return xuid1 == xuid2; }
BOOL C_4JProfile::XUIDIsGuest(PlayerUID xuid) { return FALSE; }
bool C_4JProfile::AllowedToPlayMultiplayer(int iProf) { return true; }
bool C_4JProfile::GetChatAndContentRestrictions(int iPad, bool *pbChatRestricted, bool *pbContentRestricted, int *piAge) {
    if (pbChatRestricted) *pbChatRestricted = false;
    if (pbContentRestricted) *pbContentRestricted = false;
    if (piAge) *piAge = 18;
    return true;
}
void C_4JProfile::StartTrialGame() {}
void C_4JProfile::AllowedPlayerCreatedContent(int iPad, bool thisQuadrantOnly, BOOL *allAllowed, BOOL *friendsAllowed) {
    if (allAllowed) *allAllowed = TRUE;
    if (friendsAllowed) *friendsAllowed = TRUE;
}
BOOL C_4JProfile::CanViewPlayerCreatedContent(int iPad, bool thisQuadrantOnly, PPlayerUID pXuids, DWORD dwXuidCount) { return TRUE; }
void C_4JProfile::ShowProfileCard(int iPad, PlayerUID targetUid) {}
bool C_4JProfile::GetProfileAvatar(int iPad, int(*Func)(LPVOID lpParam, PBYTE pbThumbnail, DWORD dwThumbnailBytes), LPVOID lpParam) { return false; }
void C_4JProfile::CancelProfileAvatarRequest() {}
int C_4JProfile::GetPrimaryPad() { return 0; }
void C_4JProfile::SetPrimaryPad(int iPad) {}

static char s_gamertag[64] = "Player";
char* C_4JProfile::GetGamertag(int iPad) { return s_gamertag; }
std::wstring C_4JProfile::GetDisplayName(int iPad) { return L"Player"; }
bool C_4JProfile::IsFullVersion() { return true; }
void C_4JProfile::SetSignInChangeCallback(void(*Func)(void *, bool, unsigned int), void *lpParam) {}
void C_4JProfile::SetNotificationsCallback(void(*Func)(void *, std::uint32_t, unsigned int), void *lpParam) {}
bool C_4JProfile::RegionIsNorthAmerica(void) { return true; }
bool C_4JProfile::LocaleIsUSorCanada(void) { return true; }
HRESULT C_4JProfile::GetLiveConnectionStatus() { return S_OK; }
bool C_4JProfile::IsSystemUIDisplayed() { return false; }
void C_4JProfile::SetProfileReadErrorCallback(void(*Func)(void *), void *lpParam) {}
int C_4JProfile::SetDefaultOptionsCallback(int(*Func)(void *, PROFILESETTINGS *, const int iPad), void *lpParam) { return 0; }
int C_4JProfile::SetOldProfileVersionCallback(int(*Func)(void *, unsigned char *, const unsigned short, const int), void *lpParam) { return 0; }

static C_4JProfile::PROFILESETTINGS s_defaultSettings = {};
C_4JProfile::PROFILESETTINGS* C_4JProfile::GetDashboardProfileSettings(int iPad) { return &s_defaultSettings; }
void C_4JProfile::WriteToProfile(int iQuadrant, bool bGameDefinedDataChanged, bool bOverride5MinuteLimitOnProfileWrites) {}
void C_4JProfile::ForceQueuedProfileWrites(int iPad) {}
void* C_4JProfile::GetGameDefinedProfileData(int iQuadrant) { return s_profileData[iQuadrant]; }
void C_4JProfile::ResetProfileProcessState() {}
void C_4JProfile::Tick(void) {}
void C_4JProfile::RegisterAward(int iAwardNumber, int iGamerconfigID, eAwardType eType, bool bLeaderboardAffected,
    CXuiStringTable *pStringTable, int iTitleStr, int iTextStr, int iAcceptStr, char *pszThemeName, unsigned int uiThemeSize) {}
int C_4JProfile::GetAwardId(int iAwardNumber) { return 0; }
eAwardType C_4JProfile::GetAwardType(int iAwardNumber) { return eAwardType_Achievement; }
bool C_4JProfile::CanBeAwarded(int iQuadrant, int iAwardNumber) { return false; }
void C_4JProfile::Award(int iQuadrant, int iAwardNumber, bool bForce) {}
bool C_4JProfile::IsAwardsFlagSet(int iQuadrant, int iAward) { return false; }
void C_4JProfile::RichPresenceInit(int iPresenceCount, int iContextCount) {}
void C_4JProfile::RegisterRichPresenceContext(int iGameConfigContextID) {}
void C_4JProfile::SetRichPresenceContextValue(int iPad, int iContextID, int iVal) {}
void C_4JProfile::SetCurrentGameActivity(int iPad, int iNewPresence, bool bSetOthersToIdle) {}
void C_4JProfile::DisplayFullVersionPurchase(bool bRequired, int iQuadrant, int iUpsellParam) {}
void C_4JProfile::SetUpsellCallback(void(*Func)(void *lpParam, eUpsellType type, eUpsellResponse response, int iUserData), void *lpParam) {}
void C_4JProfile::SetDebugFullOverride(bool bVal) {}
