#pragma once

#include <cstdint>
#include <string>

#include "../4J.Common/4J_Compat.h"
#include "../platform/IPlatformProfile.h"

#define TITLEID_MINECRAFT 0x584111F7

#define CONTEXT_GAME_STATE 0
#define CONTEXT_GAME_STATE_BLANK 0
#define CONTEXT_GAME_STATE_RIDING_PIG 1
#define CONTEXT_GAME_STATE_RIDING_MINECART 2
#define CONTEXT_GAME_STATE_BOATING 3
#define CONTEXT_GAME_STATE_FISHING 4
#define CONTEXT_GAME_STATE_CRAFTING 5
#define CONTEXT_GAME_STATE_FORGING 6
#define CONTEXT_GAME_STATE_NETHER 7
#define CONTEXT_GAME_STATE_CD 8
#define CONTEXT_GAME_STATE_MAP 9
#define CONTEXT_GAME_STATE_ENCHANTING 5
#define CONTEXT_GAME_STATE_BREWING 5
#define CONTEXT_GAME_STATE_ANVIL 6
#define CONTEXT_GAME_STATE_TRADING 0

#define CONTEXT_PRESENCE_IDLE 0
#define CONTEXT_PRESENCE_MENUS 1
#define CONTEXT_PRESENCE_MULTIPLAYER 2
#define CONTEXT_PRESENCE_MULTIPLAYEROFFLINE 3
#define CONTEXT_PRESENCE_MULTIPLAYER_1P 4
#define CONTEXT_PRESENCE_MULTIPLAYER_1POFFLINE 5

class C_4JProfile : public IPlatformProfile {
public:
    // --- Methods with real logic (implemented in .cpp) ---

    void Initialise(std::uint32_t dwTitleID, std::uint32_t dwOfferID,
                    unsigned short usProfileVersion,
                    unsigned int uiProfileValuesC,
                    unsigned int uiProfileSettingsC,
                    std::uint32_t* pdwProfileSettingsA,
                    int iGameDefinedDataSizeX4,
                    unsigned int* puiGameDefinedDataChangedBitmask);

    int GetLockedProfile();
    void SetLockedProfile(int iProf);
    bool IsSignedIn(int iQuadrant);
    bool IsSignedInLive(int iProf);
    bool IsGuest(int iQuadrant);
    bool QuerySigninStatus();
    void GetXUID(int iPad, PlayerUID* pXuid, bool bOnlineXuid);
    bool AreXUIDSEqual(PlayerUID xuid1, PlayerUID xuid2);
    bool XUIDIsGuest(PlayerUID xuid);
    bool AllowedToPlayMultiplayer(int iProf);
    bool GetChatAndContentRestrictions(int iPad, bool* pbChatRestricted,
                                       bool* pbContentRestricted, int* piAge);
    char* GetGamertag(int iPad);
    std::wstring GetDisplayName(int iPad);
    int SetDefaultOptionsCallback(int (*Func)(void*, PROFILESETTINGS*,
                                              const int iPad),
                                  void* lpParam);
    PROFILESETTINGS* GetDashboardProfileSettings(int iPad);
    void* GetGameDefinedProfileData(int iQuadrant);
    void AllowedPlayerCreatedContent(int iPad, bool thisQuadrantOnly,
                                     bool* allAllowed, bool* friendsAllowed);
    bool CanViewPlayerCreatedContent(int iPad, bool thisQuadrantOnly,
                                     PlayerUID* pXuids, unsigned int xuidCount);

    // --- Dead stubs (inline no-ops, kept for call-site compat) ---

    void Tick() {}
    void SetTrialTextStringTable(CXuiStringTable*, int, int) {}
    void SetTrialAwardText(eAwardType, int, int) {}
    void StartTrialGame() {}
    unsigned int RequestSignInUI(bool, bool, bool, bool, bool,
                                 int (*)(void*, const bool, const int),
                                 void*, int = XUSER_INDEX_ANY) { return 0; }
    unsigned int DisplayOfflineProfile(int (*)(void*, const bool, const int),
                                       void*, int = XUSER_INDEX_ANY) { return 0; }
    unsigned int RequestConvertOfflineToGuestUI(
        int (*)(void*, const bool, const int), void*,
        int = XUSER_INDEX_ANY) { return 0; }
    void SetPrimaryPlayerChanged(bool) {}
    void ShowProfileCard(int, PlayerUID) {}
    bool GetProfileAvatar(int, int (*)(void*, std::uint8_t*, unsigned int),
                          void*) { return false; }
    void CancelProfileAvatarRequest() {}
    void SetSignInChangeCallback(void (*)(void*, bool, unsigned int), void*) {}
    void SetNotificationsCallback(void (*)(void*, std::uint32_t, unsigned int),
                                  void*) {}
    bool RegionIsNorthAmerica() { return false; }
    bool LocaleIsUSorCanada() { return false; }
    int GetLiveConnectionStatus() { return 0; }
    bool IsSystemUIDisplayed() { return false; }
    void SetProfileReadErrorCallback(void (*)(void*), void*) {}
    int SetOldProfileVersionCallback(int (*)(void*, unsigned char*,
                                             const unsigned short, const int),
                                     void*) { return 0; }
    void WriteToProfile(int, bool = false, bool = false) {}
    void ForceQueuedProfileWrites(int = XUSER_INDEX_ANY) {}
    void ResetProfileProcessState() {}
    void RegisterAward(int, int, eAwardType, bool = false,
                       CXuiStringTable* = nullptr, int = -1, int = -1,
                       int = -1, char* = nullptr, unsigned int = 0L) {}
    int GetAwardId(int) { return 0; }
    eAwardType GetAwardType(int) { return eAwardType_Achievement; }
    bool CanBeAwarded(int, int) { return false; }
    void Award(int, int, bool = false) {}
    bool IsAwardsFlagSet(int, int) { return false; }
    void RichPresenceInit(int, int) {}
    void RegisterRichPresenceContext(int) {}
    void SetRichPresenceContextValue(int, int, int) {}
    void SetCurrentGameActivity(int, int, bool = false) {}
    void DisplayFullVersionPurchase(bool, int, int = -1) {}
    void SetUpsellCallback(void (*)(void*, eUpsellType, eUpsellResponse, int),
                           void*) {}
    void SetDebugFullOverride(bool) {}

    // GetPrimaryPad/SetPrimaryPad moved to InputManager
    int GetPrimaryPad();
    void SetPrimaryPad(int iPad);
};

// Singleton
extern C_4JProfile ProfileManager;
