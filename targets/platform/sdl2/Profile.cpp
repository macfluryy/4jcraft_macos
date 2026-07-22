#include "Profile.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <functional>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>

#include "../ProfileConstants.h"
#include "../sdl2/Input.h"

C_4JProfile ProfileManager;

namespace {
















constexpr uint32_t kProfileMagic = 0x4D34504AU;  
constexpr uint32_t kProfileVersion = 1U;

std::string GetProfileDataPath() {
    const char* home = std::getenv("HOME");
    if (home == nullptr) {
        
        
        struct passwd* pw = getpwuid(getuid());
        if (pw != nullptr) home = pw->pw_dir;
    }
    if (home == nullptr) return "";
    std::string base = home;
    base += "/Library/Application Support/4jcraft";
    
    ::mkdir(base.c_str(), 0755);
    return base + "/profile.dat";
}















PlayerUID LoadOrCreatePersistentXuidBase() {
    const char* home = std::getenv("HOME");
    if (home == nullptr) {
        struct passwd* pw = getpwuid(getuid());
        if (pw != nullptr) home = pw->pw_dir;
    }

    std::string path;
    if (home != nullptr) {
        path = home;
        path += "/Library/Application Support/4jcraft";
        ::mkdir(path.c_str(), 0755);
        path += "/xuid.dat";
    }

    if (!path.empty()) {
        FILE* fp = std::fopen(path.c_str(), "rb");
        if (fp != nullptr) {
            PlayerUID stored = 0;
            size_t read = std::fread(&stored, 1, sizeof(stored), fp);
            std::fclose(fp);
            if (read == sizeof(stored) && stored != 0) {
                return stored;
            }
        }
    }

    
    PlayerUID base = 0xe000d45248242f2eULL;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t mix = ((uint64_t)getpid() << 32) ^ (uint64_t)ts.tv_nsec ^
                   ((uint64_t)ts.tv_sec << 16);
    PlayerUID generated =
        (base & 0xffff000000000000ULL) | (mix & 0x0000ffffffffffffULL);

    if (!path.empty()) {
        FILE* fp = std::fopen(path.c_str(), "wb");
        if (fp != nullptr) {
            std::fwrite(&generated, 1, sizeof(generated), fp);
            std::fclose(fp);
        }
    }
    return generated;
}

PlayerUID ComputePerProcessXuidBase() {
    return LoadOrCreatePersistentXuidBase();
}
PlayerUID kFakeXuidBase = ComputePerProcessXuidBase();

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
int s_perPadProfileSize = 0;  
std::function<int(C_4JProfile::PROFILESETTINGS*, int)>
    s_defaultOptionsCallback;

bool isValidPad(int iPad) { return iPad >= 0 && iPad < XUSER_MAX_COUNT; }





bool LoadProfileFromDisk(int perPadSize) {
    std::string path = GetProfileDataPath();
    if (path.empty()) {
        std::fprintf(stderr, "[profile] LoadProfileFromDisk: no path\n");
        return false;
    }

    FILE* fp = std::fopen(path.c_str(), "rb");
    if (fp == nullptr) {
        std::fprintf(stderr,
                     "[profile] LoadProfileFromDisk: %s not present "
                     "(first launch or file missing)\n",
                     path.c_str());
        return false;
    }

    uint32_t header[4] = {};
    if (std::fread(header, sizeof(uint32_t), 4, fp) != 4) {
        std::fprintf(stderr,
                     "[profile] LoadProfileFromDisk: short header read\n");
        std::fclose(fp);
        return false;
    }
    if (header[0] != kProfileMagic || header[1] != kProfileVersion ||
        header[2] != static_cast<uint32_t>(perPadSize) ||
        header[3] != static_cast<uint32_t>(XUSER_MAX_COUNT)) {
        std::fprintf(stderr,
                     "[profile] LoadProfileFromDisk: header mismatch "
                     "(magic=0x%X ver=%u perPad=%u count=%u expected "
                     "perPad=%d count=%d)\n",
                     header[0], header[1], header[2], header[3],
                     perPadSize, XUSER_MAX_COUNT);
        std::fclose(fp);
        return false;
    }
    bool ok = true;
    for (int i = 0; i < XUSER_MAX_COUNT && ok; ++i) {
        if (std::fread(s_profileData[i], 1, perPadSize, fp) !=
            (size_t)perPadSize) {
            ok = false;
        }
    }
    std::fclose(fp);
    std::fprintf(stderr,
                 "[profile] LoadProfileFromDisk: loaded %s "
                 "(perPad=%d count=%d) ok=%d\n",
                 path.c_str(), perPadSize, XUSER_MAX_COUNT, ok ? 1 : 0);
    return ok;
}







void SaveProfileToDisk(int perPadSize) {
    if (perPadSize <= 0) {
        std::fprintf(stderr,
                     "[profile] SaveProfileToDisk: perPadSize=%d, skipping\n",
                     perPadSize);
        return;
    }
    std::string path = GetProfileDataPath();
    if (path.empty()) {
        std::fprintf(stderr, "[profile] SaveProfileToDisk: no path\n");
        return;
    }
    std::string tmp = path + ".tmp";

    FILE* fp = std::fopen(tmp.c_str(), "wb");
    if (fp == nullptr) {
        std::fprintf(stderr,
                     "[profile] SaveProfileToDisk: open(%s) failed\n",
                     tmp.c_str());
        return;
    }

    uint32_t header[4] = {kProfileMagic, kProfileVersion,
                          static_cast<uint32_t>(perPadSize),
                          static_cast<uint32_t>(XUSER_MAX_COUNT)};
    if (std::fwrite(header, sizeof(uint32_t), 4, fp) != 4) {
        std::fclose(fp);
        ::unlink(tmp.c_str());
        std::fprintf(stderr,
                     "[profile] SaveProfileToDisk: header write failed\n");
        return;
    }
    bool ok = true;
    for (int i = 0; i < XUSER_MAX_COUNT && ok; ++i) {
        if (std::fwrite(s_profileData[i], 1, perPadSize, fp) !=
            (size_t)perPadSize) {
            ok = false;
        }
    }
    std::fflush(fp);
    std::fclose(fp);
    if (!ok) {
        ::unlink(tmp.c_str());
        std::fprintf(stderr,
                     "[profile] SaveProfileToDisk: body write failed\n");
        return;
    }
    if (::rename(tmp.c_str(), path.c_str()) != 0) {
        std::fprintf(stderr,
                     "[profile] SaveProfileToDisk: rename(%s -> %s) "
                     "failed\n",
                     tmp.c_str(), path.c_str());
        return;
    }
    std::fprintf(stderr,
                 "[profile] SaveProfileToDisk: wrote %s "
                 "(perPad=%d count=%d)\n",
                 path.c_str(), perPadSize, XUSER_MAX_COUNT);
}

void ensureFakeIdentity(int iPad) {
    if (!isValidPad(iPad) || s_gamertags[iPad][0] != '\0') {
        return;
    }

    
    
    
    
    
    
    
    unsigned int suffix =
        (unsigned int)((kFakeXuidBase ^ (kFakeXuidBase >> 16)) % 10000);
    std::snprintf(s_gamertags[iPad], sizeof(s_gamertags[iPad]), "Player%u",
                  suffix + (unsigned int)(iPad));
    s_displayNames[iPad] =
        std::wstring(L"Player") + std::to_wstring(suffix + (unsigned int)iPad);
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
}  









void OverrideXuidBaseForDirectConnect() {
    PlayerUID base = 0xe000d45248242f2eULL;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t mix = ((uint64_t)getpid() << 32) ^ (uint64_t)ts.tv_nsec ^
                   ((uint64_t)ts.tv_sec << 16) ^ (uint64_t)(uintptr_t)&ts;
    kFakeXuidBase =
        (base & 0xffff000000000000ULL) | (mix & 0x0000ffffffffffffULL);
    
    for (int i = 0; i < XUSER_MAX_COUNT; ++i) {
        s_gamertags[i][0] = '\0';
    }
    std::fprintf(stderr,
                 "[profile] OverrideXuidBaseForDirectConnect: new XUID base "
                 "0x%016llx\n",
                 (unsigned long long)kFakeXuidBase);
}







void SetUserGamertag(int iPad, const std::wstring& nick) {
    int p = (iPad >= 0 && iPad < XUSER_MAX_COUNT) ? iPad : 0;
    
    std::string ascii;
    ascii.reserve(nick.size());
    for (wchar_t wc : nick) {
        if (wc < 128 && wc > 0) ascii.push_back(static_cast<char>(wc));
    }
    if (ascii.length() >= sizeof(s_gamertags[0])) {
        ascii.resize(sizeof(s_gamertags[0]) - 1);
    }
    std::strncpy(s_gamertags[p], ascii.c_str(), sizeof(s_gamertags[p]) - 1);
    s_gamertags[p][sizeof(s_gamertags[p]) - 1] = '\0';
    s_displayNames[p] = nick;
}

void C_4JProfile::Initialise(std::uint32_t, std::uint32_t, unsigned short,
                             unsigned int, unsigned int, std::uint32_t*,
                             int iGameDefinedDataSizeX4, unsigned int*) {
    s_lockedProfile = 0;
    std::memset(s_dashboardSettings, 0, sizeof(s_dashboardSettings));

    const int perPad = iGameDefinedDataSizeX4 / 4;
    s_perPadProfileSize = perPad;

    for (int i = 0; i < XUSER_MAX_COUNT; ++i) {
        delete[] static_cast<unsigned char*>(s_profileData[i]);
        s_profileData[i] = new unsigned char[perPad];
        std::memset(s_profileData[i], 0, perPad);
        initialiseDefaultGameSettings(
            static_cast<ProfileGameSettings*>(s_profileData[i]));
        ensureFakeIdentity(i);
    }

    
    
    
    
    LoadProfileFromDisk(perPad);
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

int C_4JProfile::SetDefaultOptionsCallback(
    std::function<int(PROFILESETTINGS*, int)> callback) {
    s_defaultOptionsCallback = std::move(callback);
    return 0;
}

C_4JProfile::PROFILESETTINGS* C_4JProfile::GetDashboardProfileSettings(
    int iPad) {
    return &s_dashboardSettings[isValidPad(iPad) ? iPad : 0];
}

void* C_4JProfile::GetGameDefinedProfileData(int iQuadrant) {
    return isValidPad(iQuadrant) ? s_profileData[iQuadrant] : nullptr;
}








void C_4JProfile::WriteToProfile(int iPad,
                                 bool bGameDefinedDataChanged,
                                 bool bOverrideTimeLimit) {
    std::fprintf(stderr,
                 "[profile] WriteToProfile(iPad=%d, dirty=%d, force=%d)\n",
                 iPad, bGameDefinedDataChanged ? 1 : 0,
                 bOverrideTimeLimit ? 1 : 0);
    SaveProfileToDisk(s_perPadProfileSize);
}

void C_4JProfile::ForceQueuedProfileWrites(int iPad) {
    std::fprintf(stderr,
                 "[profile] ForceQueuedProfileWrites(iPad=%d)\n", iPad);
    SaveProfileToDisk(s_perPadProfileSize);
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




int C_4JProfile::GetPrimaryPad() {
    extern C_4JInput InputManager;
    return InputManager.GetPrimaryPad();
}
void C_4JProfile::SetPrimaryPad(int iPad) {
    extern C_4JInput InputManager;
    InputManager.SetPrimaryPad(iPad);
}
