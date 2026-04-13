// macOS ARM port of Linux_Minecraft.cpp

// #include <system_service.h>

#include <csignal>
#include "util/StringHelpers.h"

// ---------------------------------------------------------------------------
// Crash signal handler (macOS — execinfo is available on Apple platforms)
// ---------------------------------------------------------------------------
#if defined(__APPLE__)
#include <execinfo.h>
#include <unistd.h>

static void sigsegv_handler(int sig) {
    const char msg[] = "\n=== SIGNAL CAUGHT: ";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);

    char signum[8];
    int  len = 0, s = sig;
    if (s == 0) {
        signum[len++] = '0';
    } else {
        char tmp[8];
        int  tl = 0;
        while (s > 0) { tmp[tl++] = '0' + (s % 10); s /= 10; }
        for (int i = tl - 1; i >= 0; i--) signum[len++] = tmp[i];
    }
    write(STDERR_FILENO, signum, len);

    const char msg1b[] = " ===\n";
    write(STDERR_FILENO, msg1b, sizeof(msg1b) - 1);

    void* array[64];
    int   size = backtrace(array, 64);
    backtrace_symbols_fd(array, size, STDERR_FILENO);

    const char msg2[] = "=== END BACKTRACE ===\n";
    write(STDERR_FILENO, msg2, sizeof(msg2) - 1);
    _exit(139);
}
#endif // __APPLE__

// ---------------------------------------------------------------------------

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#include "minecraft/stats/StatsCounter.h"
#include "minecraft/world/level/Level.h"

#include "platform/PlatformTypes.h"
#include "platform/InputActions.h"
#include "platform/sdl2/Input.h"
#include "platform/sdl2/Profile.h"
#include "platform/sdl2/Render.h"
#include "platform/sdl2/Storage.h"

#include "app/common/App_Defines.h"
#include "app/common/src/Audio/SoundEngine.h"
#include "app/common/src/Network/GameNetworkManager.h"

#include "app/mac/MacGame.h"
#include "app/mac/Mac_UIController.h"

#include "minecraft/world/level/storage/ConsoleSaveFileIO/compression.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/renderer/Tesselator.h"
#include "minecraft/client/renderer/Textures.h"
#include "minecraft/world/level/chunk/storage/OldChunkStorage.h"
#include "minecraft/world/level/tile/Tile.h"
#include "strings.h"

// ---------------------------------------------------------------------------

#define THEME_NAME     "584111F70AAAAAAA"
#define THEME_FILESIZE 2797568
#define FIFTY_ONE_MB   (1000000 * 51)

#define NUM_PROFILE_VALUES   5
#define NUM_PROFILE_SETTINGS 4

uint32_t dwProfileSettingsA[NUM_PROFILE_VALUES] = {0, 0, 0, 0, 0};

// Rich-presence string helpers
uint8_t* AddRichPresenceString(int iID);
void     FreeRichPresenceStrings();

bool g_bWidescreen = true;

// ---------------------------------------------------------------------------
// Input action mappings (identical to Linux version)
// ---------------------------------------------------------------------------
void DefineActions(void) {
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_A,             _360_JOY_BUTTON_A);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_B,             _360_JOY_BUTTON_B);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_X,             _360_JOY_BUTTON_X);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_Y,             _360_JOY_BUTTON_Y);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_OK,            _360_JOY_BUTTON_A);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_CANCEL,        _360_JOY_BUTTON_B);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_UP,            _360_JOY_BUTTON_DPAD_UP    | _360_JOY_BUTTON_LSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_DOWN,          _360_JOY_BUTTON_DPAD_DOWN  | _360_JOY_BUTTON_LSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_LEFT,          _360_JOY_BUTTON_DPAD_LEFT  | _360_JOY_BUTTON_LSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_RIGHT,         _360_JOY_BUTTON_DPAD_RIGHT | _360_JOY_BUTTON_LSTICK_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_PAGEUP,        _360_JOY_BUTTON_LT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_PAGEDOWN,      _360_JOY_BUTTON_RT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_RIGHT_SCROLL,  _360_JOY_BUTTON_RB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_LEFT_SCROLL,   _360_JOY_BUTTON_LB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_PAUSEMENU,     _360_JOY_BUTTON_START);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_STICK_PRESS,         _360_JOY_BUTTON_LTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_OTHER_STICK_PRESS,   _360_JOY_BUTTON_RTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_OTHER_STICK_UP,      _360_JOY_BUTTON_RSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_OTHER_STICK_DOWN,    _360_JOY_BUTTON_RSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_OTHER_STICK_LEFT,    _360_JOY_BUTTON_RSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_OTHER_STICK_RIGHT,   _360_JOY_BUTTON_RSTICK_RIGHT);

    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_JUMP,                  _360_JOY_BUTTON_A);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_FORWARD,               _360_JOY_BUTTON_LSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_BACKWARD,              _360_JOY_BUTTON_LSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_LEFT,                  _360_JOY_BUTTON_LSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_RIGHT,                 _360_JOY_BUTTON_LSTICK_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_LOOK_LEFT,             _360_JOY_BUTTON_RSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_LOOK_RIGHT,            _360_JOY_BUTTON_RSTICK_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_LOOK_UP,               _360_JOY_BUTTON_RSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_LOOK_DOWN,             _360_JOY_BUTTON_RSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_USE,                   _360_JOY_BUTTON_LT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_ACTION,                _360_JOY_BUTTON_RT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_RIGHT_SCROLL,          _360_JOY_BUTTON_RB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_LEFT_SCROLL,           _360_JOY_BUTTON_LB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_INVENTORY,             _360_JOY_BUTTON_Y);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_PAUSEMENU,             _360_JOY_BUTTON_START);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_DROP,                  _360_JOY_BUTTON_B);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_SNEAK_TOGGLE,          _360_JOY_BUTTON_RTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_CRAFTING,              _360_JOY_BUTTON_X);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_RENDER_THIRD_PERSON,   _360_JOY_BUTTON_LTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_GAME_INFO,             _360_JOY_BUTTON_BACK);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_DPAD_LEFT,             _360_JOY_BUTTON_DPAD_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_DPAD_RIGHT,            _360_JOY_BUTTON_DPAD_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_DPAD_UP,               _360_JOY_BUTTON_DPAD_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_DPAD_DOWN,             _360_JOY_BUTTON_DPAD_DOWN);

    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_A,             _360_JOY_BUTTON_A);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_B,             _360_JOY_BUTTON_B);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_X,             _360_JOY_BUTTON_X);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_Y,             _360_JOY_BUTTON_Y);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_OK,            _360_JOY_BUTTON_A);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_CANCEL,        _360_JOY_BUTTON_B);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_UP,            _360_JOY_BUTTON_DPAD_UP    | _360_JOY_BUTTON_LSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_DOWN,          _360_JOY_BUTTON_DPAD_DOWN  | _360_JOY_BUTTON_LSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_LEFT,          _360_JOY_BUTTON_DPAD_LEFT  | _360_JOY_BUTTON_LSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_RIGHT,         _360_JOY_BUTTON_DPAD_RIGHT | _360_JOY_BUTTON_LSTICK_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_PAGEUP,        _360_JOY_BUTTON_LB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_PAGEDOWN,      _360_JOY_BUTTON_RT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_RIGHT_SCROLL,  _360_JOY_BUTTON_RB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_LEFT_SCROLL,   _360_JOY_BUTTON_LB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_PAUSEMENU,     _360_JOY_BUTTON_START);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_STICK_PRESS,         _360_JOY_BUTTON_LTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_OTHER_STICK_PRESS,   _360_JOY_BUTTON_RTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_OTHER_STICK_UP,      _360_JOY_BUTTON_RSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_OTHER_STICK_DOWN,    _360_JOY_BUTTON_RSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_OTHER_STICK_LEFT,    _360_JOY_BUTTON_RSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_OTHER_STICK_RIGHT,   _360_JOY_BUTTON_RSTICK_RIGHT);

    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_JUMP,                  _360_JOY_BUTTON_RB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_FORWARD,               _360_JOY_BUTTON_LSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_BACKWARD,              _360_JOY_BUTTON_LSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_LEFT,                  _360_JOY_BUTTON_LSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_RIGHT,                 _360_JOY_BUTTON_LSTICK_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_LOOK_LEFT,             _360_JOY_BUTTON_RSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_LOOK_RIGHT,            _360_JOY_BUTTON_RSTICK_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_LOOK_UP,               _360_JOY_BUTTON_RSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_LOOK_DOWN,             _360_JOY_BUTTON_RSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_USE,                   _360_JOY_BUTTON_RT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_ACTION,                _360_JOY_BUTTON_LT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_RIGHT_SCROLL,          _360_JOY_BUTTON_DPAD_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_LEFT_SCROLL,           _360_JOY_BUTTON_DPAD_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_INVENTORY,             _360_JOY_BUTTON_Y);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_PAUSEMENU,             _360_JOY_BUTTON_START);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_DROP,                  _360_JOY_BUTTON_B);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_SNEAK_TOGGLE,          _360_JOY_BUTTON_LTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_CRAFTING,              _360_JOY_BUTTON_X);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_RENDER_THIRD_PERSON,   _360_JOY_BUTTON_RTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_GAME_INFO,             _360_JOY_BUTTON_BACK);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_DPAD_LEFT,             _360_JOY_BUTTON_DPAD_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_DPAD_RIGHT,            _360_JOY_BUTTON_DPAD_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_DPAD_UP,               _360_JOY_BUTTON_DPAD_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_DPAD_DOWN,             _360_JOY_BUTTON_DPAD_DOWN);

    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_A,             _360_JOY_BUTTON_A);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_B,             _360_JOY_BUTTON_B);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_X,             _360_JOY_BUTTON_X);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_Y,             _360_JOY_BUTTON_Y);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_OK,            _360_JOY_BUTTON_A);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_CANCEL,        _360_JOY_BUTTON_B);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_UP,            _360_JOY_BUTTON_DPAD_UP    | _360_JOY_BUTTON_LSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_DOWN,          _360_JOY_BUTTON_DPAD_DOWN  | _360_JOY_BUTTON_LSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_LEFT,          _360_JOY_BUTTON_DPAD_LEFT  | _360_JOY_BUTTON_LSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_RIGHT,         _360_JOY_BUTTON_DPAD_RIGHT | _360_JOY_BUTTON_LSTICK_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_PAGEUP,        _360_JOY_BUTTON_DPAD_UP | _360_JOY_BUTTON_LB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_PAGEDOWN,      _360_JOY_BUTTON_RT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_RIGHT_SCROLL,  _360_JOY_BUTTON_RB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_LEFT_SCROLL,   _360_JOY_BUTTON_LB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_PAUSEMENU,     _360_JOY_BUTTON_START);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_STICK_PRESS,         _360_JOY_BUTTON_LTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_OTHER_STICK_PRESS,   _360_JOY_BUTTON_RTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_OTHER_STICK_UP,      _360_JOY_BUTTON_RSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_OTHER_STICK_DOWN,    _360_JOY_BUTTON_RSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_OTHER_STICK_LEFT,    _360_JOY_BUTTON_RSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_OTHER_STICK_RIGHT,   _360_JOY_BUTTON_RSTICK_RIGHT);

    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_JUMP,                  _360_JOY_BUTTON_LT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_FORWARD,               _360_JOY_BUTTON_LSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_BACKWARD,              _360_JOY_BUTTON_LSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_LEFT,                  _360_JOY_BUTTON_LSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_RIGHT,                 _360_JOY_BUTTON_LSTICK_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_LOOK_LEFT,             _360_JOY_BUTTON_RSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_LOOK_RIGHT,            _360_JOY_BUTTON_RSTICK_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_LOOK_UP,               _360_JOY_BUTTON_RSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_LOOK_DOWN,             _360_JOY_BUTTON_RSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_USE,                   _360_JOY_BUTTON_RT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_ACTION,                _360_JOY_BUTTON_A);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_RIGHT_SCROLL,          _360_JOY_BUTTON_DPAD_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_LEFT_SCROLL,           _360_JOY_BUTTON_DPAD_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_INVENTORY,             _360_JOY_BUTTON_Y);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_PAUSEMENU,             _360_JOY_BUTTON_START);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_DROP,                  _360_JOY_BUTTON_B);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_SNEAK_TOGGLE,          _360_JOY_BUTTON_LB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_CRAFTING,              _360_JOY_BUTTON_X);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_RENDER_THIRD_PERSON,   _360_JOY_BUTTON_LTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_GAME_INFO,             _360_JOY_BUTTON_BACK);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_DPAD_LEFT,             _360_JOY_BUTTON_DPAD_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_DPAD_RIGHT,            _360_JOY_BUTTON_DPAD_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_DPAD_UP,               _360_JOY_BUTTON_DPAD_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_DPAD_DOWN,             _360_JOY_BUTTON_DPAD_DOWN);
}

// ---------------------------------------------------------------------------
// main()
// ---------------------------------------------------------------------------

int main(int argc, const char* argv[]) {
#if defined(__APPLE__)
    // Install crash signal handlers (same approach as Linux / GLIBC version)
    struct sigaction sa;
    sa.sa_handler = sigsegv_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;
    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGBUS,  &sa, nullptr);
    sigaction(SIGTRAP, &sa, nullptr);
#endif

    app.DebugPrintf("---main()\n");

    // ---- Parse CLI arguments ----
    {
        int  reqW = 0, reqH = 0;
        bool fs   = false;
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--fullscreen") == 0) {
                fs = true;
            } else if (strcmp(argv[i], "--width") == 0 && i + 1 < argc) {
                reqW = atoi(argv[++i]);
            } else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc) {
                reqH = atoi(argv[++i]);
            }
        }
        if (reqW > 0 && reqH > 0) RenderManager.SetWindowSize(reqW, reqH);
        if (fs) RenderManager.SetFullscreen(true);
    }

    static bool bTrialTimerDisplayed = true;

    RenderManager.Initialise();

    app.DebugPrintf("---ReadProductCodes()\n");
    app.loadMediaArchive();
    app.loadStringTable();

    ui.init(1920, 1080);

    StorageManager.Init(
            0, app.GetString(IDS_DEFAULT_SAVENAME), (char*)"savegame.dat",
            FIFTY_ONE_MB,
            [](const C4JStorage::ESavingMessage eMsg, int iPad) {
                return app.displaySavingMessage(eMsg, iPad);
            },
            (char*)"");

    app.InitTime();

    InputManager.Initialise(1, 5, MINECRAFT_ACTION_MAX, ACTION_MAX_MENU);
    DefineActions();
    InputManager.SetJoypadMapVal(0, 0);
    InputManager.SetKeyRepeatRate(0.3f, 0.2f);

    ProfileManager.Initialise(
            TITLEID_MINECRAFT, app.m_dwOfferID, PROFILE_VERSION_10,
            NUM_PROFILE_VALUES, NUM_PROFILE_SETTINGS, dwProfileSettingsA,
            app.GAME_DEFINED_PROFILE_DATA_BYTES * XUSER_MAX_COUNT,
            &app.uiGameDefinedDataChangedBitmask);

    ProfileManager.SetSignInChangeCallback(
            [](bool bVal, unsigned int uiSignInData) {
                Game::SignInChangeCallback(&app, bVal, uiSignInData);
            });

    g_NetworkManager.Initialise();
    ProfileManager.SetDebugFullOverride(true);

    Tesselator::CreateNewThreadStorage(1024 * 1024);
    Compression::CreateNewThreadStorage();
    OldChunkStorage::CreateNewThreadStorage();
    Level::enableLightingCache();
    Tile::CreateNewThreadStorage();

    Minecraft::main();
    Minecraft* pMinecraft = Minecraft::GetInstance();

    app.InitGameSettings();
    app.InitialiseTips();

    // ---- Main game loop ----
    while (!RenderManager.ShouldClose()) {
        RenderManager.StartFrame();

        if (pMinecraft->pollResize()) {
            int fbw, fbh;
            RenderManager.GetFramebufferSize(fbw, fbh);
            ui.setScreenSize(fbw, fbh);
        }

        app.UpdateTime();
        InputManager.Tick();
        ProfileManager.Tick();
        StorageManager.Tick();
        RenderManager.Tick();
        g_NetworkManager.DoWork();

#if defined(ENABLE_JAVA_GUIS)
        pMinecraft->run_middle();
        if (app.GetGameStarted()) {
#else
        if (app.GetGameStarted()) {
            pMinecraft->run_middle();
#endif
            app.SetAppPaused(
                    g_NetworkManager.GetPlayerCount() == 1 &&
                    ui.IsPauseMenuDisplayed(ProfileManager.GetPrimaryPad()));
        } else {
            pMinecraft->soundEngine->tick(nullptr, 0.0f);
            pMinecraft->textures->tick(true, false);
            if (app.GetReallyChangingSessionType())
                pMinecraft->tickAllConnections();
        }

        pMinecraft->soundEngine->playMusicTick();

        static bool bInitnet = false;
        if (bInitnet) g_NetworkManager.Initialise();

        ui.tick();
        ui.render();

        RenderManager.Present();
        ui.CheckMenuDisplayed();

        // Apply game-settings changes triggered by profile loads
        if (app.uiGameDefinedDataChangedBitmask != 0) {
            void* pData;
            for (int i = 0; i < XUSER_MAX_COUNT; i++) {
                if (app.uiGameDefinedDataChangedBitmask & (1 << i)) {
                    app.ClearGameSettingsChangedFlag(i);
                    app.DebugPrintf("*** - APPLYING GAME SETTINGS CHANGE for pad %d\n", i);
                    app.ApplyGameSettingsChanged(i);

#if defined(_DEBUG_MENUS_ENABLED)
                    if (app.DebugSettingsOn())
                        app.ActionDebugMask(i);
                    else
                        app.ActionDebugMask(i, true);
#endif
                    pMinecraft->stats[i]->clear();
                    pMinecraft->stats[i]->parse(pData);
                }
            }
            app.uiGameDefinedDataChangedBitmask = 0;
        }

        g_NetworkManager.DoWork();
        app.HandleXuiActions();

        if (bTrialTimerDisplayed) {
            ui.ShowTrialTimer(false);
            bTrialTimerDisplayed = false;
        }
    } // end game loop

    // Graceful shutdown: destroy GL context and window before C++ dtors run.
    RenderManager.Shutdown();
    _exit(0);
} // end main

// ---------------------------------------------------------------------------
// Rich-presence string helpers
// ---------------------------------------------------------------------------

std::vector<uint8_t*> vRichPresenceStrings;

uint8_t* mallocAndCreateUTF8ArrayFromString(int iID) {
    const wchar_t* wchString = app.GetString(iID);
    std::wstring   srcString = wchString;
    std::u8string  dstString = wstring_to_u8string(srcString);
    int            dst_len   = dstString.size() + 1;
    uint8_t*       strUtf8   = (uint8_t*)malloc(dst_len);
    memcpy(strUtf8, dstString.c_str(), dst_len);
    return strUtf8;
}

uint8_t* AddRichPresenceString(int iID) {
    uint8_t* strUtf8 = mallocAndCreateUTF8ArrayFromString(iID);
    if (strUtf8 != nullptr) vRichPresenceStrings.push_back(strUtf8);
    return strUtf8;
}

void FreeRichPresenceStrings() {
    for (size_t i = 0; i < vRichPresenceStrings.size(); i++)
        free(vRichPresenceStrings[i]);
    vRichPresenceStrings.clear();
}

#pragma clang diagnostic pop