// Minecraft.cpp : Defines the entry point for the application.
//

// #include <system_service.h>
#include <csignal>

#if defined(__linux__) && defined(__GLIBC__)
#include <execinfo.h>
#include <unistd.h>

static void sigsegv_handler(int sig) {
    const char msg[] = "\n=== SIGNAL CAUGHT: ";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);
    char signum[8];
    int len = 0;
    int s = sig;
    if (s == 0) {
        signum[len++] = '0';
    } else {
        char tmp[8];
        int tl = 0;
        while (s > 0) {
            tmp[tl++] = '0' + (s % 10);
            s /= 10;
        }
        for (int i = tl - 1; i >= 0; i--) signum[len++] = tmp[i];
    }
    write(STDERR_FILENO, signum, len);
    const char msg1b[] = " ===\n";
    write(STDERR_FILENO, msg1b, sizeof(msg1b) - 1);
    void* array[64];
    int size = backtrace(array, 64);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    const char msg2[] = "=== END BACKTRACE ===\n";
    write(STDERR_FILENO, msg2, sizeof(msg2) - 1);
    _exit(139);
}
#endif
#include <features.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include "minecraft/stats/StatsCounter.h"
#include "minecraft/world/level/Level.h"
// #include "app/common/src/Leaderboards/LeaderboardManager.h"
// #include "../Common/XUI/XUI_Scene_Container.h"
// #include "NetworkManager.h"
#include "platform/PlatformTypes.h"
#include "platform/InputActions.h"
#include "platform/sdl2/Input.h"
#include "platform/sdl2/Profile.h"
#include "platform/sdl2/Render.h"
#include "platform/sdl2/Storage.h"
#include "app/common/App_Defines.h"
#include "app/common/src/Audio/SoundEngine.h"
#include "app/common/src/Network/GameNetworkManager.h"
#include "app/linux/Linux_App.h"
#include "app/linux/Linux_UIController.h"
#include "console_helpers/compression.h"
#include "console_helpers/StringHelpers.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/renderer/Tesselator.h"
#include "minecraft/client/renderer/Textures.h"
#include "minecraft/world/level/chunk/storage/OldChunkStorage.h"
#include "minecraft/world/level/tile/Tile.h"
#include "strings.h"

// #include "../Orbis/Leaderboards/OrbisLeaderboardManager.h"

// #include "../Orbis/Network/Orbis_NPToolkit.h"
// #include "../Orbis/Network/SonyVoiceChat_Orbis.h"

#define THEME_NAME "584111F70AAAAAAA"
#define THEME_FILESIZE 2797568

// #define THREE_MB 3145728 // minimum save size (checking for this on a
// selected device) #define FIVE_MB 5242880 // minimum save size (checking for
// this on a selected device) #define FIFTY_TWO_MB (1024*1024*52) // Maximum TCR
// space required for a save (checking for this on a selected device)
#define FIFTY_ONE_MB \
    (1000000 * 51)  // Maximum TCR space required for a save is 52MB (checking
                    // for this on a selected device)

// #define PROFILE_VERSION 3 // new version for the interim bug fix 166 TU
#define NUM_PROFILE_VALUES 5
#define NUM_PROFILE_SETTINGS 4
uint32_t dwProfileSettingsA[NUM_PROFILE_VALUES] = {0, 0, 0, 0, 0};

//-------------------------------------------------------------------------------------
// Time             Since fAppTime is a float, we need to keep the quadword app
// time
//                  as a LARGE_INTEGER so that we don't lose precision after
//                  running for a long time.
//-------------------------------------------------------------------------------------

// functions for storing and converting rich presence strings from wchar to utf8
uint8_t* AddRichPresenceString(int iID);
void FreeRichPresenceStrings();

bool g_bWidescreen = true;

void DefineActions(void) {
    // The app needs to define the actions required, and the possible mappings
    // for these

    // Split into Menu actions, and in-game actions

    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_A,
                                   _360_JOY_BUTTON_A);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_B,
                                   _360_JOY_BUTTON_B);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_X,
                                   _360_JOY_BUTTON_X);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_Y,
                                   _360_JOY_BUTTON_Y);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_OK,
                                   _360_JOY_BUTTON_A);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_CANCEL,
                                   _360_JOY_BUTTON_B);
    InputManager.SetGameJoypadMaps(
        MAP_STYLE_0, ACTION_MENU_UP,
        _360_JOY_BUTTON_DPAD_UP | _360_JOY_BUTTON_LSTICK_UP);
    InputManager.SetGameJoypadMaps(
        MAP_STYLE_0, ACTION_MENU_DOWN,
        _360_JOY_BUTTON_DPAD_DOWN | _360_JOY_BUTTON_LSTICK_DOWN);
    InputManager.SetGameJoypadMaps(
        MAP_STYLE_0, ACTION_MENU_LEFT,
        _360_JOY_BUTTON_DPAD_LEFT | _360_JOY_BUTTON_LSTICK_LEFT);
    InputManager.SetGameJoypadMaps(
        MAP_STYLE_0, ACTION_MENU_RIGHT,
        _360_JOY_BUTTON_DPAD_RIGHT | _360_JOY_BUTTON_LSTICK_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_PAGEUP,
                                   _360_JOY_BUTTON_LT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_PAGEDOWN,
                                   _360_JOY_BUTTON_RT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_RIGHT_SCROLL,
                                   _360_JOY_BUTTON_RB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_LEFT_SCROLL,
                                   _360_JOY_BUTTON_LB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_PAUSEMENU,
                                   _360_JOY_BUTTON_START);

    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_STICK_PRESS,
                                   _360_JOY_BUTTON_LTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_OTHER_STICK_PRESS,
                                   _360_JOY_BUTTON_RTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_OTHER_STICK_UP,
                                   _360_JOY_BUTTON_RSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_OTHER_STICK_DOWN,
                                   _360_JOY_BUTTON_RSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_OTHER_STICK_LEFT,
                                   _360_JOY_BUTTON_RSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, ACTION_MENU_OTHER_STICK_RIGHT,
                                   _360_JOY_BUTTON_RSTICK_RIGHT);

    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_JUMP,
                                   _360_JOY_BUTTON_A);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_FORWARD,
                                   _360_JOY_BUTTON_LSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_BACKWARD,
                                   _360_JOY_BUTTON_LSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_LEFT,
                                   _360_JOY_BUTTON_LSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_RIGHT,
                                   _360_JOY_BUTTON_LSTICK_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_LOOK_LEFT,
                                   _360_JOY_BUTTON_RSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_LOOK_RIGHT,
                                   _360_JOY_BUTTON_RSTICK_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_LOOK_UP,
                                   _360_JOY_BUTTON_RSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_LOOK_DOWN,
                                   _360_JOY_BUTTON_RSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_USE,
                                   _360_JOY_BUTTON_LT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_ACTION,
                                   _360_JOY_BUTTON_RT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_RIGHT_SCROLL,
                                   _360_JOY_BUTTON_RB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_LEFT_SCROLL,
                                   _360_JOY_BUTTON_LB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_INVENTORY,
                                   _360_JOY_BUTTON_Y);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_PAUSEMENU,
                                   _360_JOY_BUTTON_START);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_DROP,
                                   _360_JOY_BUTTON_B);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_SNEAK_TOGGLE,
                                   _360_JOY_BUTTON_RTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_CRAFTING,
                                   _360_JOY_BUTTON_X);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0,
                                   MINECRAFT_ACTION_RENDER_THIRD_PERSON,
                                   _360_JOY_BUTTON_LTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_GAME_INFO,
                                   _360_JOY_BUTTON_BACK);

    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_DPAD_LEFT,
                                   _360_JOY_BUTTON_DPAD_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_DPAD_RIGHT,
                                   _360_JOY_BUTTON_DPAD_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_DPAD_UP,
                                   _360_JOY_BUTTON_DPAD_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_0, MINECRAFT_ACTION_DPAD_DOWN,
                                   _360_JOY_BUTTON_DPAD_DOWN);

    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_A,
                                   _360_JOY_BUTTON_A);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_B,
                                   _360_JOY_BUTTON_B);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_X,
                                   _360_JOY_BUTTON_X);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_Y,
                                   _360_JOY_BUTTON_Y);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_OK,
                                   _360_JOY_BUTTON_A);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_CANCEL,
                                   _360_JOY_BUTTON_B);
    InputManager.SetGameJoypadMaps(
        MAP_STYLE_1, ACTION_MENU_UP,
        _360_JOY_BUTTON_DPAD_UP | _360_JOY_BUTTON_LSTICK_UP);
    InputManager.SetGameJoypadMaps(
        MAP_STYLE_1, ACTION_MENU_DOWN,
        _360_JOY_BUTTON_DPAD_DOWN | _360_JOY_BUTTON_LSTICK_DOWN);
    InputManager.SetGameJoypadMaps(
        MAP_STYLE_1, ACTION_MENU_LEFT,
        _360_JOY_BUTTON_DPAD_LEFT | _360_JOY_BUTTON_LSTICK_LEFT);
    InputManager.SetGameJoypadMaps(
        MAP_STYLE_1, ACTION_MENU_RIGHT,
        _360_JOY_BUTTON_DPAD_RIGHT | _360_JOY_BUTTON_LSTICK_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_PAGEUP,
                                   _360_JOY_BUTTON_LB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_PAGEDOWN,
                                   _360_JOY_BUTTON_RT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_RIGHT_SCROLL,
                                   _360_JOY_BUTTON_RB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_LEFT_SCROLL,
                                   _360_JOY_BUTTON_LB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_PAUSEMENU,
                                   _360_JOY_BUTTON_START);

    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_STICK_PRESS,
                                   _360_JOY_BUTTON_LTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_OTHER_STICK_PRESS,
                                   _360_JOY_BUTTON_RTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_OTHER_STICK_UP,
                                   _360_JOY_BUTTON_RSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_OTHER_STICK_DOWN,
                                   _360_JOY_BUTTON_RSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_OTHER_STICK_LEFT,
                                   _360_JOY_BUTTON_RSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, ACTION_MENU_OTHER_STICK_RIGHT,
                                   _360_JOY_BUTTON_RSTICK_RIGHT);

    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_JUMP,
                                   _360_JOY_BUTTON_RB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_FORWARD,
                                   _360_JOY_BUTTON_LSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_BACKWARD,
                                   _360_JOY_BUTTON_LSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_LEFT,
                                   _360_JOY_BUTTON_LSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_RIGHT,
                                   _360_JOY_BUTTON_LSTICK_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_LOOK_LEFT,
                                   _360_JOY_BUTTON_RSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_LOOK_RIGHT,
                                   _360_JOY_BUTTON_RSTICK_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_LOOK_UP,
                                   _360_JOY_BUTTON_RSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_LOOK_DOWN,
                                   _360_JOY_BUTTON_RSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_USE,
                                   _360_JOY_BUTTON_RT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_ACTION,
                                   _360_JOY_BUTTON_LT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_RIGHT_SCROLL,
                                   _360_JOY_BUTTON_DPAD_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_LEFT_SCROLL,
                                   _360_JOY_BUTTON_DPAD_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_INVENTORY,
                                   _360_JOY_BUTTON_Y);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_PAUSEMENU,
                                   _360_JOY_BUTTON_START);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_DROP,
                                   _360_JOY_BUTTON_B);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_SNEAK_TOGGLE,
                                   _360_JOY_BUTTON_LTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_CRAFTING,
                                   _360_JOY_BUTTON_X);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1,
                                   MINECRAFT_ACTION_RENDER_THIRD_PERSON,
                                   _360_JOY_BUTTON_RTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_GAME_INFO,
                                   _360_JOY_BUTTON_BACK);

    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_DPAD_LEFT,
                                   _360_JOY_BUTTON_DPAD_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_DPAD_RIGHT,
                                   _360_JOY_BUTTON_DPAD_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_DPAD_UP,
                                   _360_JOY_BUTTON_DPAD_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_1, MINECRAFT_ACTION_DPAD_DOWN,
                                   _360_JOY_BUTTON_DPAD_DOWN);

    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_A,
                                   _360_JOY_BUTTON_A);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_B,
                                   _360_JOY_BUTTON_B);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_X,
                                   _360_JOY_BUTTON_X);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_Y,
                                   _360_JOY_BUTTON_Y);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_OK,
                                   _360_JOY_BUTTON_A);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_CANCEL,
                                   _360_JOY_BUTTON_B);
    InputManager.SetGameJoypadMaps(
        MAP_STYLE_2, ACTION_MENU_UP,
        _360_JOY_BUTTON_DPAD_UP | _360_JOY_BUTTON_LSTICK_UP);
    InputManager.SetGameJoypadMaps(
        MAP_STYLE_2, ACTION_MENU_DOWN,
        _360_JOY_BUTTON_DPAD_DOWN | _360_JOY_BUTTON_LSTICK_DOWN);
    InputManager.SetGameJoypadMaps(
        MAP_STYLE_2, ACTION_MENU_LEFT,
        _360_JOY_BUTTON_DPAD_LEFT | _360_JOY_BUTTON_LSTICK_LEFT);
    InputManager.SetGameJoypadMaps(
        MAP_STYLE_2, ACTION_MENU_RIGHT,
        _360_JOY_BUTTON_DPAD_RIGHT | _360_JOY_BUTTON_LSTICK_RIGHT);
    InputManager.SetGameJoypadMaps(
        MAP_STYLE_2, ACTION_MENU_PAGEUP,
        _360_JOY_BUTTON_DPAD_UP | _360_JOY_BUTTON_LB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_PAGEDOWN,
                                   _360_JOY_BUTTON_RT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_RIGHT_SCROLL,
                                   _360_JOY_BUTTON_RB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_LEFT_SCROLL,
                                   _360_JOY_BUTTON_LB);

    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_JUMP,
                                   _360_JOY_BUTTON_LT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_FORWARD,
                                   _360_JOY_BUTTON_LSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_BACKWARD,
                                   _360_JOY_BUTTON_LSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_LEFT,
                                   _360_JOY_BUTTON_LSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_RIGHT,
                                   _360_JOY_BUTTON_LSTICK_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_LOOK_LEFT,
                                   _360_JOY_BUTTON_RSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_LOOK_RIGHT,
                                   _360_JOY_BUTTON_RSTICK_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_LOOK_UP,
                                   _360_JOY_BUTTON_RSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_LOOK_DOWN,
                                   _360_JOY_BUTTON_RSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_USE,
                                   _360_JOY_BUTTON_RT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_ACTION,
                                   _360_JOY_BUTTON_A);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_RIGHT_SCROLL,
                                   _360_JOY_BUTTON_DPAD_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_LEFT_SCROLL,
                                   _360_JOY_BUTTON_DPAD_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_INVENTORY,
                                   _360_JOY_BUTTON_Y);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_PAUSEMENU,
                                   _360_JOY_BUTTON_START);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_DROP,
                                   _360_JOY_BUTTON_B);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_SNEAK_TOGGLE,
                                   _360_JOY_BUTTON_LB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_CRAFTING,
                                   _360_JOY_BUTTON_X);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2,
                                   MINECRAFT_ACTION_RENDER_THIRD_PERSON,
                                   _360_JOY_BUTTON_LTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_GAME_INFO,
                                   _360_JOY_BUTTON_BACK);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_PAUSEMENU,
                                   _360_JOY_BUTTON_START);

    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_STICK_PRESS,
                                   _360_JOY_BUTTON_LTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_OTHER_STICK_PRESS,
                                   _360_JOY_BUTTON_RTHUMB);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_OTHER_STICK_UP,
                                   _360_JOY_BUTTON_RSTICK_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_OTHER_STICK_DOWN,
                                   _360_JOY_BUTTON_RSTICK_DOWN);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_OTHER_STICK_LEFT,
                                   _360_JOY_BUTTON_RSTICK_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, ACTION_MENU_OTHER_STICK_RIGHT,
                                   _360_JOY_BUTTON_RSTICK_RIGHT);

    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_DPAD_LEFT,
                                   _360_JOY_BUTTON_DPAD_LEFT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_DPAD_RIGHT,
                                   _360_JOY_BUTTON_DPAD_RIGHT);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_DPAD_UP,
                                   _360_JOY_BUTTON_DPAD_UP);
    InputManager.SetGameJoypadMaps(MAP_STYLE_2, MINECRAFT_ACTION_DPAD_DOWN,
                                   _360_JOY_BUTTON_DPAD_DOWN);
}

int main(int argc, const char* argv[]) {
#if defined(__linux__) && defined(__GLIBC__)
    struct sigaction sa;
    sa.sa_handler = sigsegv_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;
    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGBUS, &sa, nullptr);
    sigaction(SIGTRAP, &sa, nullptr);
#endif
    app.DebugPrintf("---main()\n");

    // ---- Parse CLI arguments ----
    // Usage: Minecraft.Client [--width W] [--height H] [--fullscreen]
    // If --width/--height are omitted the primary monitor's native resolution
    // is used automatically.
    {
        int reqW = 0, reqH = 0;
        bool fs = false;
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

    // Read the file containing the product codes
    app.DebugPrintf("---ReadProductCodes()\n");

    app.loadMediaArchive();
    app.loadStringTable();
    // fuck you
    ui.init(1920, 1080);
    // storage manager is needed for the trial key check
    StorageManager.Init(0, app.GetString(IDS_DEFAULT_SAVENAME),
                        (char*)"savegame.dat", FIFTY_ONE_MB,
                        &CConsoleMinecraftApp::DisplaySavingMessage,
                        (void*)&app, (char*)"");

    ////////////////
    // Initialise //
    ////////////////

    app.InitTime();

    // Set the number of possible joypad layouts that the user can switch
    // between, and the number of actions
    InputManager.Initialise(1, 5, MINECRAFT_ACTION_MAX, ACTION_MAX_MENU);

    // Set the default joypad action mappings for Minecraft
    DefineActions();
    InputManager.SetJoypadMapVal(0, 0);
    InputManager.SetKeyRepeatRate(0.3f, 0.2f);

    // Initialise the profile manager with the game Title ID, Offer ID, a
    // profile version number, and the number of profile values and settings

    ProfileManager.Initialise(
        TITLEID_MINECRAFT, app.m_dwOfferID, PROFILE_VERSION_10,
        NUM_PROFILE_VALUES, NUM_PROFILE_SETTINGS, dwProfileSettingsA,
        app.GAME_DEFINED_PROFILE_DATA_BYTES * XUSER_MAX_COUNT,
        &app.uiGameDefinedDataChangedBitmask);

    // set a function to be called when there's a sign in change, so we can exit
    // a level if the primary player signs out
    ProfileManager.SetSignInChangeCallback(
        &CConsoleMinecraftApp::SignInChangeCallback, &app);

    // Set a callback for when there is a read error on profile data
    // StorageManager.SetProfileReadErrorCallback(&CConsoleMinecraftApp::ProfileReadErrorCallback,
    // &app);

    // QNet needs to be setup after profile manager, as we do not want its
    // Notify listener to handle XN_SYS_SIGNINCHANGED notifications. This does
    // mean that we need to have a callback in the ProfileManager for
    // XN_LIVE_INVITE_ACCEPTED for QNet.

    g_NetworkManager.Initialise();

    // debug switch to trial version
    ProfileManager.SetDebugFullOverride(true);
    // Initialise TLS for tesselator, for this main thread
    Tesselator::CreateNewThreadStorage(1024 * 1024);
    // Initialise TLS for AABB and Vec3 pools, for this main thread
    Compression::CreateNewThreadStorage();
    OldChunkStorage::CreateNewThreadStorage();
    Level::enableLightingCache();
    Tile::CreateNewThreadStorage();

    Minecraft::main();
    Minecraft* pMinecraft = Minecraft::GetInstance();

    app.InitGameSettings();

    app.InitialiseTips();
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

        // Tick the social networking manager.
        //		CSocialManager::Instance()->Tick();

        // Tick sentient.
        //		SentientManager.Tick();

        g_NetworkManager.DoWork();

        // Render game graphics.
#if defined(ENABLE_JAVA_GUIS)
        pMinecraft->run_middle();
        if (app.GetGameStarted()) {
#else
        if (app.GetGameStarted()) {
            pMinecraft->run_middle();
#endif
            app.SetAppPaused(
                // TODO: proper fix for pausing
                // 4jcraft: IsLocalGame() doesn't seem to work properly on Iggy
                // UI, this should work even in multiplayer scenarios though
                // since it checks for the player count anyway
                //
                // g_NetworkManager.IsLocalGame() &&
                g_NetworkManager.GetPlayerCount() == 1 &&
                ui.IsPauseMenuDisplayed(ProfileManager.GetPrimaryPad()));
        } else {
            pMinecraft->soundEngine->tick(nullptr, 0.0f);
            pMinecraft->textures->tick(true, false);
            if (app.GetReallyChangingSessionType()) {
                pMinecraft
                    ->tickAllConnections();  // Added to stop timing out when we
                                             // are waiting after converting to
                                             // an offline game
            }
        }
        pMinecraft->soundEngine->playMusicTick();

        static bool bInitnet = false;

        if (bInitnet) {
            g_NetworkManager.Initialise();
        }

        ui.tick();
        ui.render();

        // Present the frame.
        RenderManager.Present();

        ui.CheckMenuDisplayed();
        // has the game defined profile data been changed (by a profile load)
        if (app.uiGameDefinedDataChangedBitmask != 0) {
            void* pData;
            for (int i = 0; i < XUSER_MAX_COUNT; i++) {
                if (app.uiGameDefinedDataChangedBitmask & (1 << i)) {
                    // reset the changed flag
                    app.ClearGameSettingsChangedFlag(i);
                    app.DebugPrintf(
                        "***  - APPLYING GAME SETTINGS CHANGE for pad %d\n", i);
                    app.ApplyGameSettingsChanged(i);

#if defined(_DEBUG_MENUS_ENABLED)
                    if (app.DebugSettingsOn()) {
                        app.ActionDebugMask(i);
                    } else {
                        // force debug mask off
                        app.ActionDebugMask(i, true);
                    }
#endif
                    // clear the stats first - there could have beena signout
                    // and sign back in in the menus need to clear the player
                    // stats - can't assume it'll be done in setlevel - we may
                    // not be in the game
                    pMinecraft->stats[i]->clear();
                    pMinecraft->stats[i]->parse(pData);
                }
            }

            // clear the flag
            app.uiGameDefinedDataChangedBitmask = 0;
        }

        g_NetworkManager.DoWork();

        // Any threading type things to deal with from the xui side?
        app.HandleXuiActions();

        // need to turn off the trial timer if it was on
        if (bTrialTimerDisplayed) {
            ui.ShowTrialTimer(false);
            bTrialTimerDisplayed = false;
        }

        // Fix for #7318 - Title crashes after short soak in the leaderboards
    }  // end game loop

    // Graceful shutdown: destroy GL context and GLFW before any C++ dtors run.
    // Without this, static/global destructors that touch GL objects cause
    // SIGSEGV.
    RenderManager.Shutdown();
    _exit(0);
}  // end main

// Free resources, unregister custom classes, and exit.
//	app.Uninit();
//	g_pd3dDevice->Release();

std::vector<uint8_t*> vRichPresenceStrings;

uint8_t* mallocAndCreateUTF8ArrayFromString(int iID) {
    const wchar_t* wchString = app.GetString(iID);

    std::wstring srcString = wchString;
    std::u8string dstString = wstring_to_u8string(srcString);

    int dst_len = dstString.size() + 1;
    uint8_t* strUtf8 = (uint8_t*)malloc(dst_len);
    memcpy(strUtf8, dstString.c_str(), dst_len);

    return strUtf8;
}

uint8_t* AddRichPresenceString(int iID) {
    uint8_t* strUtf8 = mallocAndCreateUTF8ArrayFromString(iID);
    if (strUtf8 != nullptr) {
        vRichPresenceStrings.push_back(strUtf8);
    }
    return strUtf8;
}

void FreeRichPresenceStrings() {
    uint8_t* strUtf8;
    for (int i = 0; i < vRichPresenceStrings.size(); i++) {
        strUtf8 = vRichPresenceStrings.at(i);
        free(strUtf8);
    }
    vRichPresenceStrings.clear();
}
