// Minecraft.cpp : Defines the entry point for the application.
//

#include "../../../Minecraft.World/Platform/stdafx.h"

#include <assert.h>
// #include <system_service.h>
#include <codecvt>
#if defined(__linux__) && defined(__GLIBC__)
#include <signal.h>
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
#include "../Windows64/GameConfig/Minecraft.spa.h"
#include "../../MinecraftServer.h"
#include "../../Player/LocalPlayer.h"
#include "../../../Minecraft.World/Items/ItemInstance.h"
#include "../../../Minecraft.World/Items/MapItem.h"
#include "../../../Minecraft.World/Recipes/Recipes.h"
#include "../../../Minecraft.World/Recipes/Recipy.h"
#include "../../../Minecraft.World/Util/Language.h"
#include "../../../Minecraft.World/Util/StringHelpers.h"
#include "../../../Minecraft.World/Util/AABB.h"
#include "../../../Minecraft.World/Util/Vec3.h"
#include "../../../Minecraft.World/Level/Level.h"
#include "../../../Minecraft.World/Headers/net.minecraft.world.level.tile.h"

#include "../../Network/ClientConnection.h"
#include "../../Player/User.h"
#include "../../../Minecraft.World/Network/Socket.h"
#include "../../../Minecraft.World/Util/ThreadName.h"
#include "../../GameState/StatsCounter.h"
#include "../../UI/Screens/ConnectScreen.h"
// #include "../Durango/Social/SocialManager.h"
// #include "../Common/Leaderboards/LeaderboardManager.h"
// #include "../Common/XUI/XUI_Scene_Container.h"
// #include "NetworkManager.h"
#include "../../Rendering/Tesselator.h"
#include "../../GameState/Options.h"
#include "../Linux/Sentient/SentientManager.h"
#include "../../../Minecraft.World/Util/IntCache.h"
#include "../../Textures/Textures.h"
#include "../../../Minecraft.World/IO/Streams/Compression.h"
#include "../../../Minecraft.World/Level/Storage/OldChunkStorage.h"
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
DWORD dwProfileSettingsA[NUM_PROFILE_VALUES] = {
#ifdef _XBOX
    XPROFILE_OPTION_CONTROLLER_VIBRATION,
    XPROFILE_GAMER_YAXIS_INVERSION,
    XPROFILE_GAMER_CONTROL_SENSITIVITY,
    XPROFILE_GAMER_ACTION_MOVEMENT_CONTROL,
    XPROFILE_TITLE_SPECIFIC1,
#else
    0, 0, 0, 0, 0
#endif
};

//-------------------------------------------------------------------------------------
// Time             Since fAppTime is a float, we need to keep the quadword app
// time
//                  as a LARGE_INTEGER so that we don't lose precision after
//                  running for a long time.
//-------------------------------------------------------------------------------------

// functions for storing and converting rich presence strings from wchar to utf8
uint8_t* AddRichPresenceString(int iID);
void FreeRichPresenceStrings();

BOOL g_bWidescreen = TRUE;

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

#if 0
HRESULT InitD3D( IDirect3DDevice9 **ppDevice,
D3DPRESENT_PARAMETERS *pd3dPP )
{
IDirect3D9 *pD3D;

pD3D = Direct3DCreate9( D3D_SDK_VERSION );

// Set up the structure used to create the D3DDevice
// Using a permanent 1280x720 backbuffer now no matter what the actual video resolution.right Have also disabled letterboxing,
// which would letterbox a 1280x720 output if it detected a 4:3 video source - we're doing an anamorphic squash in this
// mode so don't need this functionality.

ZeroMemory( pd3dPP, sizeof(D3DPRESENT_PARAMETERS) );
XVIDEO_MODE VideoMode;
XGetVideoMode( &VideoMode );
g_bWidescreen = VideoMode.fIsWideScreen;
pd3dPP->BackBufferWidth        = 1280;
pd3dPP->BackBufferHeight       = 720;
pd3dPP->BackBufferFormat       = D3DFMT_A8R8G8B8;
pd3dPP->BackBufferCount        = 1;
pd3dPP->EnableAutoDepthStencil = TRUE;
pd3dPP->AutoDepthStencilFormat = D3DFMT_D24S8;
pd3dPP->SwapEffect             = D3DSWAPEFFECT_DISCARD;
pd3dPP->PresentationInterval   = D3DPRESENT_INTERVAL_ONE;
//pd3dPP->Flags				   = D3DPRESENTFLAG_NO_LETTERBOX;
//ERR[D3D]: Can't set D3DPRESENTFLAG_NO_LETTERBOX when wide-screen is enabled
//	in the launcher/dashboard.
if(g_bWidescreen)
    pd3dPP->Flags=0;
else
    pd3dPP->Flags				   = D3DPRESENTFLAG_NO_LETTERBOX;

// Create the device.
return pD3D->CreateDevice(
    0,
D3DDEVTYPE_HAL,
NULL,
D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_BUFFER_2_FRAMES,
pd3dPP,
ppDevice );
}
#endif
// #define MEMORY_TRACKING

#ifdef MEMORY_TRACKING
void ResetMem();
void DumpMem();
void MemPixStuff();
#else
void MemSect(int sect) {}
#endif

#ifndef __linux__
HINSTANCE g_hInst = NULL;
HWND g_hWnd = NULL;
D3D_DRIVER_TYPE g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device* g_pd3dDevice = NULL;
ID3D11DeviceContext* g_pImmediateContext = NULL;
IDXGISwapChain* g_pSwapChain = NULL;
ID3D11RenderTargetView* g_pRenderTargetView = NULL;
ID3D11DepthStencilView* g_pDepthStencilView = NULL;
ID3D11Texture2D* g_pDepthStencilBuffer = NULL;

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
                         LPARAM lParam) {
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message) {
        case WM_COMMAND:
            wmId = LOWORD(wParam);
            wmEvent = HIWORD(wParam);
            // Parse the menu selections:
            switch (wmId) {
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code here...
            EndPaint(hWnd, &ps);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, "Minecraft");
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = "Minecraft";
    wcex.lpszClassName = "MinecraftClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    g_hInst = hInstance;  // Store instance handle in our global variable

    g_hWnd = CreateWindow("MinecraftClass", "Minecraft", WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL,
                          hInstance, NULL);

    if (!g_hWnd) {
        return FALSE;
    }

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    return TRUE;
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice() {
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect(g_hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes;
         driverTypeIndex++) {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(
            NULL, g_driverType, NULL, createDeviceFlags, featureLevels,
            numFeatureLevels, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
            &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
        if (HRESULT_SUCCEEDED(hr)) break;
    }
    if (FAILED(hr)) return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                 (LPVOID*)&pBackBuffer);
    if (FAILED(hr)) return hr;

    // Create a depth stencil buffer
    D3D11_TEXTURE2D_DESC descDepth;

    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr =
        g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencilBuffer);

    D3D11_DEPTH_STENCIL_VIEW_DESC descDSView;
    descDSView.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDSView.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSView.Texture2D.MipSlice = 0;

    hr = g_pd3dDevice->CreateDepthStencilView(
        g_pDepthStencilBuffer, &descDSView, &g_pDepthStencilView);

    hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL,
                                              &g_pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr)) return hr;

    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView,
                                            g_pDepthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports(1, &vp);

    RenderManager.Initialise(g_pd3dDevice, g_pSwapChain);

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Render the frame
//--------------------------------------------------------------------------------------
void Render() {
    // Just clear the backbuffer
    float ClearColor[4] = {0.0f, 0.125f, 0.3f, 1.0f};  // red,green,blue,alpha

    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);
    g_pSwapChain->Present(0, 0);
}

//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice() {
    if (g_pImmediateContext) g_pImmediateContext->ClearState();

    if (g_pRenderTargetView) g_pRenderTargetView->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pImmediateContext) g_pImmediateContext->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();
}
#endif

int StartMinecraftThreadProc(void* lpParameter) {
    Vec3::UseDefaultThreadStorage();
    AABB::UseDefaultThreadStorage();
    Tesselator::CreateNewThreadStorage(1024 * 1024);
    RenderManager.InitialiseContext();
    Minecraft::start(std::wstring(), std::wstring());
    delete Tesselator::getInstance();
    return 0;
}

int main(int argc, const char* argv[]) {
#if defined(__linux__) && defined(__GLIBC__)
    struct sigaction sa;
    sa.sa_handler = sigsegv_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGTRAP, &sa, NULL);
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

#if 0
    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
    if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
    {
    TranslateMessage( &msg );
    DispatchMessage( &msg );
}
else
{
Render();
}
}

return (int) msg.wParam;
#endif

    static bool bTrialTimerDisplayed = true;

#ifdef MEMORY_TRACKING
    ResetMem();
    MEMORYSTATUS memStat;
    GlobalMemoryStatus(&memStat);
    printf("RESETMEM start: Avail. phys %d\n",
           memStat.dwAvailPhys / (1024 * 1024));
#endif

#if 0
// Initialize D3D
hr = InitD3D( &pDevice, &d3dpp );
g_pD3DDevice = pDevice;
if( FAILED(hr) )
{
app.DebugPrintf
( "Failed initializing D3D.\n" );
return -1;
}

// Initialize the application, assuming sharing of the d3d interface.
hr = app.InitShared( pDevice, &d3dpp,
XuiPNGTextureLoader );

if ( FAILED(hr) )
{
app.DebugPrintf
( "Failed initializing application.\n" );

return -1;
}

#endif

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
                        (LPVOID)&app, (char*)"");

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
    AABB::CreateNewThreadStorage();
    Vec3::CreateNewThreadStorage();
    IntCache::CreateNewThreadStorage();
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
#ifdef _ENABLEIGGY
        if (pMinecraft->pollResize()) {
            int fbw, fbh;
            RenderManager.GetFramebufferSize(fbw, fbh);
            ui.setScreenSize(fbw, fbh);
        }
#endif
        app.UpdateTime();
        PIXBeginNamedEvent(0, "Input manager tick");
        InputManager.Tick();
        PIXEndNamedEvent();
        PIXBeginNamedEvent(0, "Profile manager tick");
        ProfileManager.Tick();
        PIXEndNamedEvent();
        PIXBeginNamedEvent(0, "Storage manager tick");
        StorageManager.Tick();
        PIXEndNamedEvent();
        PIXBeginNamedEvent(0, "Render manager tick");
        RenderManager.Tick();
        PIXEndNamedEvent();

        // Tick the social networking manager.
        PIXBeginNamedEvent(0, "Social network manager tick");
        //		CSocialManager::Instance()->Tick();
        PIXEndNamedEvent();

        // Tick sentient.
        PIXBeginNamedEvent(0, "Sentient tick");
        MemSect(37);
        //		SentientManager.Tick();
        MemSect(0);
        PIXEndNamedEvent();

        PIXBeginNamedEvent(0, "Network manager do work #1");
        g_NetworkManager.DoWork();
        PIXEndNamedEvent();
        // Render game graphics.
#ifdef ENABLE_JAVA_GUIS
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
            MemSect(28);
            pMinecraft->soundEngine->tick(NULL, 0.0f);
            MemSect(0);
            pMinecraft->textures->tick(true, false);
            IntCache::Reset();
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

#ifdef MEMORY_TRACKING
        static bool bResetMemTrack = false;
        static bool bDumpMemTrack = false;

        MemPixStuff();

        if (bResetMemTrack) {
            ResetMem();
            MEMORYSTATUS memStat;
            GlobalMemoryStatus(&memStat);
            printf("RESETMEM: Avail. phys %d\n",
                   memStat.dwAvailPhys / (1024 * 1024));
            bResetMemTrack = false;
        }

        if (bDumpMemTrack) {
            DumpMem();
            bDumpMemTrack = false;
            MEMORYSTATUS memStat;
            GlobalMemoryStatus(&memStat);
            printf("DUMPMEM: Avail. phys %d\n",
                   memStat.dwAvailPhys / (1024 * 1024));
            printf("Renderer used: %d\n", RenderManager.CBuffSize(-1));
        }
#endif
#if 0
static bool bDumpTextureUsage = false;
if( bDumpTextureUsage )
{
RenderManager.TextureGetStats();
bDumpTextureUsage = false;
}
#endif
        ui.tick();
        ui.render();
#if 0
app.HandleButtonPresses();

// store the minecraft renderstates, and re-set them after the xui render
GetRenderAndSamplerStates(pDevice,RenderStateA,SamplerStateA);

// Tick XUI
PIXBeginNamedEvent(0,"Xui running");
app.RunFrame();
PIXEndNamedEvent();

// Render XUI

PIXBeginNamedEvent(0,"XUI render");
MemSect(7);
hr = app.Render();
MemSect(0);
GetRenderAndSamplerStates(pDevice,RenderStateA2,SamplerStateA2);
PIXEndNamedEvent();

for(int i=0;i<8;i++)
{
if(RenderStateA2[i]!=RenderStateA[i])
{
//printf("Reseting RenderStateA[%d] after a XUI render\n",i);
pDevice->SetRenderState(RenderStateModes[i],RenderStateA[i]);
}
}
for(int i=0;i<5;i++)
{
if(SamplerStateA2[i]!=SamplerStateA[i])
{
//printf("Reseting SamplerStateA[%d] after a XUI render\n",i);
pDevice->SetSamplerState(0,SamplerStateModes[i],SamplerStateA[i]);
}
}

RenderManager.Set_matrixDirty();
#endif

        // Present the frame.
        RenderManager.Present();

        ui.CheckMenuDisplayed();
        PIXBeginNamedEvent(0, "Profile load check");
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

#ifdef _DEBUG_MENUS_ENABLED
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
        PIXEndNamedEvent();

        PIXBeginNamedEvent(0, "Network manager do work #2");
        g_NetworkManager.DoWork();
        PIXEndNamedEvent();

#if 0
PIXBeginNamedEvent(0,"Misc extra xui");
// Update XUI Timers
hr = XuiTimersRun();

#endif
        // Any threading type things to deal with from the xui side?
        app.HandleXuiActions();
#if 0
PIXEndNamedEvent();
#endif

        // 4J-PB - Update the trial timer display if we are in the trial version
        if (!ProfileManager.IsFullVersion()) {
            // display the trial timer
            if (app.GetGameStarted()) {
                // 4J-PB - if the game is paused, add the elapsed time to the
                // trial timer count so it doesn't tick down
                if (app.IsAppPaused()) {
                    app.UpdateTrialPausedTimer();
                }
                ui.UpdateTrialTimer(ProfileManager.GetPrimaryPad());
            }
        } else {
            // need to turn off the trial timer if it was on , and we've
            // unlocked the full version
            if (bTrialTimerDisplayed) {
                ui.ShowTrialTimer(false);
                bTrialTimerDisplayed = false;
            }
        }

        // Fix for #7318 - Title crashes after short soak in the leaderboards
        // menu A memory leak was caused because the icon renderer kept creating
        // new Vec3's because the pool wasn't reset
        Vec3::resetPool();
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

// convert std::wstring to UTF-8 string
// wchar_t is 32bit on all Linux systems, and interpreted as UTF-32
// the code base stores all strings internally as UCS-2 (16bit, subset of
// UTF-16), which, scince it only stores BMP code points, is trivially
// convertable to UTF-32 as well as UTF-16. hence this parser simply parses
// UTF-32

// all implementations of libc (including glibc, musl, uClibc...) implement
// wchar_t as 4byte/32bit (scince around 1999), it would break the libc ABI,
// if this ever will get changed, hence this assert
static_assert(sizeof(wchar_t) == 4, "Linux with non 32bit wchar_t");

std::string wstring_to_utf8(const std::wstring& str) {
    std::string result;
    // preallocation, so it will never need to resize.
    // same allocation size as for the 4byte wstring representation.
    // it well get destructed instantly, in the function that it gets called
    // from
    result.reserve(str.size() * 4);

    for (size_t i = 0; i < str.size(); ++i) {
        uint32_t cp = static_cast<uint32_t>(str[i]);

        // outside of valid unicode range or preserved UTF-16 surrogate pairs
        // (just in case)
        if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) {
            cp = 0xFFFD;  // unicode replacement character
        }

        if (cp < 0x80) {
            // ASCII
            result += static_cast<char>(cp);
            // extract multibyte unicode into multiple bytes of UTF-8
        } else if (cp < 0x800) {
            result += static_cast<char>(0xC0 | (cp >> 6));
            result += static_cast<char>(0x80 | (cp & 0x3F));
        } else if (cp < 0x10000) {
            result += static_cast<char>(0xE0 | (cp >> 12));
            result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (cp & 0x3F));
        } else {
            result += static_cast<char>(0xF0 | (cp >> 18));
            result += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
            result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (cp & 0x3F));
        }
    }

    return result;
}

uint8_t* mallocAndCreateUTF8ArrayFromString(int iID) {
    LPCWSTR wchString = app.GetString(iID);

    std::wstring srcString = wchString;
    std::string dstString = wstring_to_utf8(srcString);

    int dst_len = dstString.size() + 1;
    uint8_t* strUtf8 = (uint8_t*)malloc(dst_len);
    memcpy(strUtf8, dstString.c_str(), dst_len);

    return strUtf8;
}

uint8_t* AddRichPresenceString(int iID) {
    uint8_t* strUtf8 = mallocAndCreateUTF8ArrayFromString(iID);
    if (strUtf8 != NULL) {
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

#ifdef MEMORY_TRACKING

int totalAllocGen = 0;
std::unordered_map<int, int> allocCounts;
bool trackEnable = false;
bool trackStarted = false;
volatile size_t sizeCheckMin = 1160;
volatile size_t sizeCheckMax = 1160;
volatile int sectCheck = 48;
CRITICAL_SECTION memCS;
DWORD tlsIdx;

LPVOID XMemAlloc(SIZE_T dwSize, DWORD dwAllocAttributes) {
    if (!trackStarted) {
        void* p = XMemAllocDefault(dwSize, dwAllocAttributes);
        size_t realSize = XMemSizeDefault(p, dwAllocAttributes);
        totalAllocGen += realSize;
        return p;
    }

    EnterCriticalSection(&memCS);

    void* p = XMemAllocDefault(dwSize + 16, dwAllocAttributes);
    size_t realSize = XMemSizeDefault(p, dwAllocAttributes) - 16;

    if (trackEnable) {
#if 1
        int sect = ((int)TlsGetValue(tlsIdx)) & 0x3f;
        *(((unsigned char*)p) + realSize) = sect;

        if ((realSize >= sizeCheckMin) && (realSize <= sizeCheckMax) &&
            ((sect == sectCheck) || (sectCheck == -1))) {
            app.DebugPrintf("Found one\n");
        }
#endif

        if (p) {
            totalAllocGen += realSize;
            trackEnable = false;
            int key = (sect << 26) | realSize;
            int oldCount = allocCounts[key];
            allocCounts[key] = oldCount + 1;

            trackEnable = true;
        }
    }

    LeaveCriticalSection(&memCS);

    return p;
}

void* operator new(size_t size) {
    return (unsigned char*)XMemAlloc(
        size, MAKE_XALLOC_ATTRIBUTES(
                  0, FALSE, TRUE, FALSE, 0, XALLOC_PHYSICAL_ALIGNMENT_DEFAULT,
                  XALLOC_MEMPROTECT_READWRITE, FALSE, XALLOC_MEMTYPE_HEAP));
}

void operator delete(void* p) {
    XMemFree(p, MAKE_XALLOC_ATTRIBUTES(
                    0, FALSE, TRUE, FALSE, 0, XALLOC_PHYSICAL_ALIGNMENT_DEFAULT,
                    XALLOC_MEMPROTECT_READWRITE, FALSE, XALLOC_MEMTYPE_HEAP));
}

void WINAPI XMemFree(PVOID pAddress, DWORD dwAllocAttributes) {
    bool special = false;
    if (dwAllocAttributes == 0) {
        dwAllocAttributes = MAKE_XALLOC_ATTRIBUTES(
            0, FALSE, TRUE, FALSE, 0, XALLOC_PHYSICAL_ALIGNMENT_DEFAULT,
            XALLOC_MEMPROTECT_READWRITE, FALSE, XALLOC_MEMTYPE_HEAP);
        special = true;
    }
    if (!trackStarted) {
        size_t realSize = XMemSizeDefault(pAddress, dwAllocAttributes);
        XMemFreeDefault(pAddress, dwAllocAttributes);
        totalAllocGen -= realSize;
        return;
    }
    EnterCriticalSection(&memCS);
    if (pAddress) {
        size_t realSize = XMemSizeDefault(pAddress, dwAllocAttributes) - 16;

        if (trackEnable) {
            int sect = *(((unsigned char*)pAddress) + realSize);
            totalAllocGen -= realSize;
            trackEnable = false;
            int key = (sect << 26) | realSize;
            int oldCount = allocCounts[key];
            allocCounts[key] = oldCount - 1;
            trackEnable = true;
        }
        XMemFreeDefault(pAddress, dwAllocAttributes);
    }
    LeaveCriticalSection(&memCS);
}

SIZE_T WINAPI XMemSize(PVOID pAddress, DWORD dwAllocAttributes) {
    if (trackStarted) {
        return XMemSizeDefault(pAddress, dwAllocAttributes) - 16;
    } else {
        return XMemSizeDefault(pAddress, dwAllocAttributes);
    }
}

void DumpMem() {
    int totalLeak = 0;
    for (AUTO_VAR(it, allocCounts.begin()); it != allocCounts.end(); it++) {
        if (it->second > 0) {
            app.DebugPrintf("%d %d %d %d\n", (it->first >> 26) & 0x3f,
                            it->first & 0x03ffffff, it->second,
                            (it->first & 0x03ffffff) * it->second);
            totalLeak += (it->first & 0x03ffffff) * it->second;
        }
    }
    app.DebugPrintf("Total %d\n", totalLeak);
}

void ResetMem() {
    if (!trackStarted) {
        trackEnable = true;
        trackStarted = true;
        totalAllocGen = 0;
        InitializeCriticalSection(&memCS);
        tlsIdx = TlsAlloc();
    }
    EnterCriticalSection(&memCS);
    trackEnable = false;
    allocCounts.clear();
    trackEnable = true;
    LeaveCriticalSection(&memCS);
}

void MemSect(int section) {
    unsigned int value = (unsigned int)TlsGetValue(tlsIdx);
    if (section == 0)  // pop
    {
        value = (value >> 6) & 0x03ffffff;
    } else {
        value = (value << 6) | section;
    }
    TlsSetValue(tlsIdx, (LPVOID)value);
}

void MemPixStuff() {
    const int MAX_SECT = 46;

    int totals[MAX_SECT] = {0};

    for (AUTO_VAR(it, allocCounts.begin()); it != allocCounts.end(); it++) {
        if (it->second > 0) {
            int sect = (it->first >> 26) & 0x3f;
            int bytes = it->first & 0x03ffffff;
            totals[sect] += bytes * it->second;
        }
    }

    unsigned int allSectsTotal = 0;
    for (int i = 0; i < MAX_SECT; i++) {
        allSectsTotal += totals[i];
        PIXAddNamedCounter(((float)totals[i]) / 1024.0f, "MemSect%d", i);
    }

    PIXAddNamedCounter(((float)allSectsTotal) / (4096.0f),
                       "MemSect total pages");
}

#endif
