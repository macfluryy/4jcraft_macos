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
// 4J macOS - profile persistence path. We dump the per-pad
// GAME_DEFINED_PROFILE_DATA blocks into a single file under the
// app support folder so settings (volume, sensitivity, gamma, HUD,
// FOV, view bobbing, control scheme, language, etc.) survive
// across launches. Original Xbox builds rely on the platform
// profile system which is a no-op on macOS.
//
// Format (versioned, append-only safe):
//   bytes 0..3   : magic 'M4JP'
//   bytes 4..7   : little-endian uint32 version
//   bytes 8..11  : little-endian uint32 perPadSize
//   bytes 12..15 : little-endian uint32 padCount
//   bytes 16..   : padCount * perPadSize bytes of game-defined data
//
// On a size mismatch (game updated, struct grew) we fall back to the
// defaults rather than corrupting memory.
constexpr uint32_t kProfileMagic = 0x4D34504AU;  // 'M4JP'
constexpr uint32_t kProfileVersion = 1U;

std::string GetProfileDataPath() {
    const char* home = std::getenv("HOME");
    if (home == nullptr) {
        // Fall back to the user database if HOME is unset (e.g.
        // double-clicked .app launched without a shell environment).
        struct passwd* pw = getpwuid(getuid());
        if (pw != nullptr) home = pw->pw_dir;
    }
    if (home == nullptr) return "";
    std::string base = home;
    base += "/Library/Application Support/4jcraft";
    // mkdir -p (best effort, ignore EEXIST).
    ::mkdir(base.c_str(), 0755);
    return base + "/profile.dat";
}

// 4J macOS - base XUID used to be a constexpr constant, which meant every
// Minecraft.Client instance on the same machine had the same XUID. That
// caused ClientConnection::handleAddPlayer to reject remote players as
// "local" due to XUID equality. Randomise a per-process offset at startup
// so host and each remote client get distinct XUIDs.
//
// 4J macOS - but a fully random per-process base meant `players/<xuid>.dat`
// changed every launch, so single-player saves never recovered the
// player's position / inventory: each new launch wrote a fresh .dat
// alongside the old ones. We now persist the chosen XUID base in
// xuid.dat in the app support folder. On first launch we randomise
// once and write it; on subsequent launches we read the same value
// back. Different physical machines still get different bases, so
// multiplayer XUID disambiguation still works.
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

    // First launch: randomise a fresh base and persist it.
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
int s_perPadProfileSize = 0;  // 4J macOS - cached from Initialise
std::function<int(C_4JProfile::PROFILESETTINGS*, int)>
    s_defaultOptionsCallback;

bool isValidPad(int iPad) { return iPad >= 0 && iPad < XUSER_MAX_COUNT; }

// 4J macOS - load persisted game-defined profile data from
// ~/Library/Application Support/4jcraft/profile.dat into the in-memory
// per-pad buffers. Returns true if the file was present and matches
// the expected layout; false otherwise (caller leaves defaults intact).
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

// 4J macOS - persist all per-pad profile blocks to disk. Atomic write
// via temp file + rename so a crash mid-write can't truncate the
// real file. Best-effort: any IO error is silently swallowed because
// settings persistence is a soft requirement (we don't want to spam
// stderr on every shutdown of an app launched without a writable
// home directory).
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

    // 4J macOS - For direct-connect multiplayer two clients on the same
    // machine would otherwise both call themselves "Player1" because the
    // gamertag was deterministic per-pad. We derive a stable per-machine
    // suffix from the persisted XUID base (see kFakeXuidBase /
    // LoadOrCreatePersistentXuidBase) so the gamertag stays the same
    // across launches and the host's chat / tab list / players/<xuid>
    // .dat file all line up.
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
}  // namespace

// 4J macOS - direct-connect MP override. When two Minecraft.Client processes
// run on the same machine and connect to one another, both read the same
// xuid.dat and end up with identical XUIDs. handleAddPlayer on each side
// then treats the remote peer as "actually a local player" and drops the
// packet, so neither shows up in the other's tab list or world. Calling
// this from TemporaryDirectConnectStart re-randomises the in-memory XUID
// for the joining client only - we do NOT rewrite xuid.dat, so singleplayer
// saves keyed by the original XUID still recover correctly.
void OverrideXuidBaseForDirectConnect() {
    PlayerUID base = 0xe000d45248242f2eULL;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t mix = ((uint64_t)getpid() << 32) ^ (uint64_t)ts.tv_nsec ^
                   ((uint64_t)ts.tv_sec << 16) ^ (uint64_t)(uintptr_t)&ts;
    kFakeXuidBase =
        (base & 0xffff000000000000ULL) | (mix & 0x0000ffffffffffffULL);
    // Reset cached gamertags so they regenerate against the new base.
    for (int i = 0; i < XUSER_MAX_COUNT; ++i) {
        s_gamertags[i][0] = '\0';
    }
    std::fprintf(stderr,
                 "[profile] OverrideXuidBaseForDirectConnect: new XUID base "
                 "0x%016llx\n",
                 (unsigned long long)kFakeXuidBase);
}

// 4J macOS - persistent user-chosen gamertag override. When the player
// picks a name on the Title -> Username screen we mirror it into
// s_gamertags / s_displayNames so subsequent ProfileManager.GetGamertag()
// / GetDisplayName() calls return that name. This is what overrides the
// Player%random% string the rest of the engine assembles from the XUID
// when no human-readable identity is set.
void SetUserGamertag(int iPad, const std::wstring& nick) {
    int p = (iPad >= 0 && iPad < XUSER_MAX_COUNT) ? iPad : 0;
    // Clip to s_gamertags fixed buffer (16 bytes incl. nul).
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

    // 4J macOS - try to overlay persisted user settings on top of the
    // defaults. If the file is missing / version-mismatched / size-
    // mismatched the defaults stay in place and the next save will
    // create a fresh file with the current layout.
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

// 4J macOS - real persistence hooks. Game.cpp calls these after the
// player tweaks any setting (volume, sensitivity, gamma, FOV, ...).
// The originals were no-ops on macOS so settings only lived in
// memory. Now they atomically write the per-pad GAME_DEFINED data to
// ~/Library/Application Support/4jcraft/profile.dat. The original
// signature has parameters we ignore (Xbox queue/timer flags) -
// every call just flushes immediately.
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
