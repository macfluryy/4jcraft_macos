#include "4J_Input.h"
#include "../Minecraft.Client/Platform/Common/App_enums.h"
#include "../4J.Render/4J_Render.h"
#include <SDL2/SDL.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

C_4JInput InputManager;

static const int KEY_COUNT = SDL_NUM_SCANCODES;
static const float MOUSE_SCALE = 0.015f;
// Vars
static bool s_sdlInitialized = false;
static bool s_keysCurrent[KEY_COUNT] = {};
static bool s_keysPrev[KEY_COUNT] = {};
static bool s_mouseLeftCurrent = false, s_mouseLeftPrev = false;
static bool s_mouseRightCurrent = false, s_mouseRightPrev = false;
static bool s_menuDisplayed[4] = {};
static bool s_prevMenuDisplayed = false;
static bool s_snapTaken = false;
static float s_accumRelX = 0, s_accumRelY = 0;
static float s_snapRelX = 0, s_snapRelY = 0;

static int s_scrollTicksForButtonPressed = 0;
static int s_scrollTicksForGetValue = 0;
static int s_scrollTicksSnap = 0;
static bool s_scrollSnapTaken = false;

// We set all the watched keys
// I don't know if I'll need to change this if we add chat support soon.
static const int s_watchedKeys[] = {
    SDL_SCANCODE_W,      SDL_SCANCODE_A,      SDL_SCANCODE_S,
    SDL_SCANCODE_D,      SDL_SCANCODE_SPACE,  SDL_SCANCODE_LSHIFT,
    SDL_SCANCODE_RSHIFT, SDL_SCANCODE_E,      SDL_SCANCODE_Q,
    SDL_SCANCODE_F,      SDL_SCANCODE_C,      SDL_SCANCODE_ESCAPE,
    SDL_SCANCODE_RETURN, SDL_SCANCODE_F3,     SDL_SCANCODE_F5,
    SDL_SCANCODE_UP,     SDL_SCANCODE_DOWN,   SDL_SCANCODE_LEFT,
    SDL_SCANCODE_RIGHT,  SDL_SCANCODE_PAGEUP, SDL_SCANCODE_PAGEDOWN,
    SDL_SCANCODE_TAB,    SDL_SCANCODE_LCTRL,  SDL_SCANCODE_RCTRL,
    SDL_SCANCODE_1,      SDL_SCANCODE_2,      SDL_SCANCODE_3,
    SDL_SCANCODE_4,      SDL_SCANCODE_5,      SDL_SCANCODE_6,
    SDL_SCANCODE_7,      SDL_SCANCODE_8,      SDL_SCANCODE_9,
};
static const int s_watchedKeyCount =
    (int)(sizeof(s_watchedKeys) / sizeof(s_watchedKeys[0]));

static inline bool KDown(int sc) {
    return (sc > 0 && sc < KEY_COUNT) ? s_keysCurrent[sc] : false;
}
static inline bool KPressed(int sc) {
    return (sc > 0 && sc < KEY_COUNT) ? !s_keysPrev[sc] && s_keysCurrent[sc]
                                      : false;
}
static inline bool KReleased(int sc) {
    return (sc > 0 && sc < KEY_COUNT) ? s_keysPrev[sc] && !s_keysCurrent[sc]
                                      : false;
}

static inline bool MouseLDown() { return s_mouseLeftCurrent; }
static inline bool MouseLPressed() {
    return s_mouseLeftCurrent && !s_mouseLeftPrev;
}
static inline bool MouseLReleased() {
    return !s_mouseLeftCurrent && s_mouseLeftPrev;
}
static inline bool MouseRDown() { return s_mouseRightCurrent; }
static inline bool MouseRPressed() {
    return s_mouseRightCurrent && !s_mouseRightPrev;
}
static inline bool MouseRReleased() {
    return !s_mouseRightCurrent && s_mouseRightPrev;
}

// get directly into SDL events before the game queue can steal them.
// this took me a while.
static int SDLCALL EventWatcher(void*, SDL_Event* e) {
    if (e->type == SDL_MOUSEWHEEL) {
        int y = e->wheel.y;
        if (e->wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
            y = -y;
        }
        s_scrollTicksForGetValue += y;
        s_scrollTicksForButtonPressed += y;
    } else if (e->type == SDL_MOUSEBUTTONDOWN) {
        if (e->button.button == 4) {
            s_scrollTicksForGetValue++;
            s_scrollTicksForButtonPressed++;
        } else if (e->button.button == 5) {
            s_scrollTicksForGetValue--;
            s_scrollTicksForButtonPressed--;
        }
    } else if (e->type == SDL_MOUSEMOTION) {
        s_accumRelX += (float)e->motion.xrel;
        s_accumRelY += (float)e->motion.yrel;
    }
    return 1;
}

static int ScrollSnap() {
    if (!s_scrollSnapTaken) {
        s_scrollTicksSnap = s_scrollTicksForButtonPressed;
        s_scrollTicksForButtonPressed = 0;
        s_scrollSnapTaken = true;
    }
    return s_scrollTicksSnap;
}

static void TakeSnapIfNeeded() {
    if (!s_snapTaken) {
        s_snapRelX = s_accumRelX;
        s_accumRelX = 0;
        s_snapRelY = s_accumRelY;
        s_accumRelY = 0;
        s_snapTaken = true;
    }
}
// We initialize the SDL input
void C_4JInput::Initialise(int, unsigned char, unsigned char, unsigned char) {
    if (!s_sdlInitialized) {
        if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
            SDL_Init(SDL_INIT_VIDEO);
        }
        SDL_AddEventWatch(EventWatcher, NULL);
        s_sdlInitialized = true;
    }

    memset(s_keysCurrent, 0, sizeof(s_keysCurrent));
    memset(s_keysPrev, 0, sizeof(s_keysPrev));
    memset(s_menuDisplayed, 0, sizeof(s_menuDisplayed));

    s_mouseLeftCurrent = s_mouseLeftPrev = s_mouseRightCurrent =
        s_mouseRightPrev = false;
    s_accumRelX = s_accumRelY = s_snapRelX = s_snapRelY = 0;
    // i really gotta name these vars better..
    s_scrollTicksForButtonPressed = s_scrollTicksForGetValue =
        s_scrollTicksSnap = 0;
    s_snapTaken = s_scrollSnapTaken = s_prevMenuDisplayed = false;

    if (s_sdlInitialized) SDL_SetRelativeMouseMode(SDL_TRUE);
}
// Each tick we update the input state by polling SDL, this is where we get the
// kbd and mouse state.
void C_4JInput::Tick() {
    if (!s_sdlInitialized) return;

    memcpy(s_keysPrev, s_keysCurrent, sizeof(s_keysCurrent));
    s_mouseLeftPrev = s_mouseLeftCurrent;
    s_mouseRightPrev = s_mouseRightCurrent;
    s_snapTaken = false;
    s_scrollSnapTaken = false;
    s_snapRelX = s_snapRelY = 0;
    s_scrollTicksSnap = 0;

    SDL_PumpEvents();

    if (s_menuDisplayed[0]) {
        s_scrollTicksForGetValue = 0;
    }

    const Uint8* state = SDL_GetKeyboardState(NULL);
    for (int i = 0; i < s_watchedKeyCount; ++i) {
        int sc = s_watchedKeys[i];
        if (sc > 0 && sc < KEY_COUNT) s_keysCurrent[sc] = state[sc] != 0;
    }

    Uint32 btns = SDL_GetMouseState(NULL, NULL);
    s_mouseLeftCurrent = (btns & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    s_mouseRightCurrent = (btns & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;

    if (!SDL_GetRelativeMouseMode()) {
        s_accumRelX = 0;
        s_accumRelY = 0;
    }

    if (!SDL_GetKeyboardFocus()) {
        SDL_Window* mf = SDL_GetMouseFocus();
        if (mf) {
            SDL_RaiseWindow(mf);
            SDL_SetWindowGrab(mf, SDL_TRUE);
        }
    }
}

int C_4JInput::GetHotbarSlotPressed(int iPad) {
    if (iPad != 0) return -1;

    constexpr size_t NUM_HOTBAR_SLOTS = 9;

    static const int sc[NUM_HOTBAR_SLOTS] = {
        SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
        SDL_SCANCODE_4, SDL_SCANCODE_5, SDL_SCANCODE_6,
        SDL_SCANCODE_7, SDL_SCANCODE_8, SDL_SCANCODE_9,
    };
    static bool s_wasDown[NUM_HOTBAR_SLOTS] = {};

    for (int i = 0; i < NUM_HOTBAR_SLOTS; ++i) {
        bool down = KDown(sc[i]);
        bool pressed = down && !s_wasDown[i];
        s_wasDown[i] = down;
        if (pressed) return i;
    }
    return -1;
}

#define ACTION_CASES(FN)                       \
    case ACTION_MENU_UP:                       \
        return FN(SDL_SCANCODE_UP);            \
    case ACTION_MENU_DOWN:                     \
        return FN(SDL_SCANCODE_DOWN);          \
    case ACTION_MENU_LEFT:                     \
        return FN(SDL_SCANCODE_LEFT);          \
    case ACTION_MENU_RIGHT:                    \
        return FN(SDL_SCANCODE_RIGHT);         \
    case ACTION_MENU_PAGEUP:                   \
        return FN(SDL_SCANCODE_PAGEUP);        \
    case ACTION_MENU_PAGEDOWN:                 \
        return FN(SDL_SCANCODE_PAGEDOWN);      \
    case ACTION_MENU_OK:                       \
        return FN(SDL_SCANCODE_RETURN);        \
    case ACTION_MENU_CANCEL:                   \
        return FN(SDL_SCANCODE_ESCAPE);        \
    case MINECRAFT_ACTION_JUMP:                \
        return FN(SDL_SCANCODE_SPACE);         \
    case MINECRAFT_ACTION_FORWARD:             \
        return FN(SDL_SCANCODE_W);             \
    case MINECRAFT_ACTION_BACKWARD:            \
        return FN(SDL_SCANCODE_S);             \
    case MINECRAFT_ACTION_LEFT:                \
        return FN(SDL_SCANCODE_A);             \
    case MINECRAFT_ACTION_RIGHT:               \
        return FN(SDL_SCANCODE_D);             \
    case MINECRAFT_ACTION_INVENTORY:           \
        return FN(SDL_SCANCODE_E);             \
    case MINECRAFT_ACTION_PAUSEMENU:           \
        return FN(SDL_SCANCODE_ESCAPE);        \
    case MINECRAFT_ACTION_DROP:                \
        return FN(SDL_SCANCODE_Q);             \
    case MINECRAFT_ACTION_CRAFTING:            \
        return FN(SDL_SCANCODE_C);             \
    case MINECRAFT_ACTION_RENDER_THIRD_PERSON: \
        return FN(SDL_SCANCODE_F5);            \
    case MINECRAFT_ACTION_GAME_INFO:           \
        return FN(SDL_SCANCODE_F3);            \
    case MINECRAFT_ACTION_DPAD_LEFT:           \
        return FN(SDL_SCANCODE_LEFT);          \
    case MINECRAFT_ACTION_DPAD_RIGHT:          \
        return FN(SDL_SCANCODE_RIGHT);         \
    case MINECRAFT_ACTION_DPAD_UP:             \
        return FN(SDL_SCANCODE_UP);            \
    case MINECRAFT_ACTION_DPAD_DOWN:           \
        return FN(SDL_SCANCODE_DOWN);          \
    default:                                   \
        return false;

bool C_4JInput::ButtonDown(int iPad, unsigned char ucAction) {
    if (iPad != 0) return false;
    if (ucAction == 255) {
        for (int i = 0; i < s_watchedKeyCount; ++i)
            if (s_keysCurrent[s_watchedKeys[i]]) return true;
        return s_mouseLeftCurrent || s_mouseRightCurrent;
    }
    switch (ucAction) {
        case MINECRAFT_ACTION_ACTION:
            return MouseLDown() || KDown(SDL_SCANCODE_RETURN);
        case MINECRAFT_ACTION_USE:
            return MouseRDown() || KDown(SDL_SCANCODE_F);
        case MINECRAFT_ACTION_SNEAK_TOGGLE:
            return KDown(SDL_SCANCODE_LSHIFT) || KDown(SDL_SCANCODE_RSHIFT);
        case MINECRAFT_ACTION_SPRINT:
            return KDown(SDL_SCANCODE_LCTRL) || KDown(SDL_SCANCODE_RCTRL);
        case MINECRAFT_ACTION_LEFT_SCROLL:
        case ACTION_MENU_LEFT_SCROLL:
            return ScrollSnap() > 0;
        case MINECRAFT_ACTION_RIGHT_SCROLL:
        case ACTION_MENU_RIGHT_SCROLL:
            return ScrollSnap() < 0;
            ACTION_CASES(KDown)
    }
}
// The part that handles completing the action of pressing a button.
bool C_4JInput::ButtonPressed(int iPad, unsigned char ucAction) {
    if (iPad != 0 || ucAction == 255) return false;
    switch (ucAction) {
        case MINECRAFT_ACTION_ACTION:
            return MouseLPressed() || KPressed(SDL_SCANCODE_RETURN);
        case MINECRAFT_ACTION_USE:
            return MouseRPressed() || KPressed(SDL_SCANCODE_F);
        case MINECRAFT_ACTION_SNEAK_TOGGLE:
            return KPressed(SDL_SCANCODE_LSHIFT) ||
                   KPressed(SDL_SCANCODE_RSHIFT);
        case MINECRAFT_ACTION_SPRINT:
            return KPressed(SDL_SCANCODE_LCTRL) || KPressed(SDL_SCANCODE_RCTRL);
        case MINECRAFT_ACTION_LEFT_SCROLL:
        case ACTION_MENU_LEFT_SCROLL:
            return ScrollSnap() > 0;
        case MINECRAFT_ACTION_RIGHT_SCROLL:
        case ACTION_MENU_RIGHT_SCROLL:
            return ScrollSnap() < 0;
            ACTION_CASES(KPressed)
    }
}
// The part that handles Releasing a button.
bool C_4JInput::ButtonReleased(int iPad, unsigned char ucAction) {
    if (iPad != 0 || ucAction == 255) return false;
    switch (ucAction) {
        case MINECRAFT_ACTION_ACTION:
            return MouseLReleased() || KReleased(SDL_SCANCODE_RETURN);
        case MINECRAFT_ACTION_USE:
            return MouseRReleased() || KReleased(SDL_SCANCODE_F);
        case MINECRAFT_ACTION_SNEAK_TOGGLE:
            return KReleased(SDL_SCANCODE_LSHIFT) ||
                   KReleased(SDL_SCANCODE_RSHIFT);
        case MINECRAFT_ACTION_SPRINT:
            KReleased(SDL_SCANCODE_LCTRL) || KReleased(SDL_SCANCODE_RCTRL);
        case MINECRAFT_ACTION_LEFT_SCROLL:
        case ACTION_MENU_LEFT_SCROLL:
        case MINECRAFT_ACTION_RIGHT_SCROLL:
        case ACTION_MENU_RIGHT_SCROLL:
            return false;
            ACTION_CASES(KReleased)
    }
}

unsigned int C_4JInput::GetValue(int iPad, unsigned char ucAction, bool) {
    if (iPad != 0) return 0;
    if (ucAction == MINECRAFT_ACTION_LEFT_SCROLL) {
        if (s_scrollTicksForGetValue > 0) {
            unsigned int v = (unsigned int)s_scrollTicksForGetValue;
            s_scrollTicksForGetValue = 0;
            return v;
        }
        return 0u;
    }
    if (ucAction == MINECRAFT_ACTION_RIGHT_SCROLL) {
        if (s_scrollTicksForGetValue < 0) {
            unsigned int v = (unsigned int)(-s_scrollTicksForGetValue);
            s_scrollTicksForGetValue = 0;
            return v;
        }
        return 0u;
    }
    return ButtonDown(iPad, ucAction) ? 1u : 0u;
}
// Left stick movement, the one that moves the player around or selects menu
// options. (Soon be tested.)
float C_4JInput::GetJoypadStick_LX(int, bool) {
    return (KDown(SDL_SCANCODE_D) ? 1.f : 0.f) -
           (KDown(SDL_SCANCODE_A) ? 1.f : 0.f);
}
float C_4JInput::GetJoypadStick_LY(int, bool) {
    return (KDown(SDL_SCANCODE_W) ? 1.f : 0.f) -
           (KDown(SDL_SCANCODE_S) ? 1.f : 0.f);
}
// We use mouse movement and convert it into a Right Stick output using
// logarithmic scaling This is the most important mouse part. Yet it's so small.
static float MouseAxis(float raw) {
    if (fabsf(raw) < 0.0001f) return 0.f;  // from 4j previous code
    return (raw >= 0.f ? 1.f : -1.f) * sqrtf(fabsf(raw));
}
// We apply the Stick movement on the R(Right) X(2D Position)
float C_4JInput::GetJoypadStick_RX(int, bool) {
    if (!SDL_GetRelativeMouseMode()) return 0.f;
    TakeSnapIfNeeded();
    return MouseAxis(s_snapRelX * MOUSE_SCALE);
}
// Bis. but with Y(2D Position)
float C_4JInput::GetJoypadStick_RY(int, bool) {
    if (!SDL_GetRelativeMouseMode()) return 0.f;
    TakeSnapIfNeeded();
    return MouseAxis(-s_snapRelY * MOUSE_SCALE);
}

unsigned char C_4JInput::GetJoypadLTrigger(int, bool) {
    return s_mouseRightCurrent ? 255 : 0;
}
unsigned char C_4JInput::GetJoypadRTrigger(int, bool) {
    return s_mouseLeftCurrent ? 255 : 0;
}
// We detect if a Menu is visible on the player's screen to the mouse being
// stuck.
void C_4JInput::SetMenuDisplayed(int iPad, bool bVal) {
    if (iPad >= 0 && iPad < 4) s_menuDisplayed[iPad] = bVal;
    if (!s_sdlInitialized || bVal == s_prevMenuDisplayed) return;
    SDL_SetRelativeMouseMode(bVal ? SDL_FALSE : SDL_TRUE);
    s_prevMenuDisplayed = bVal;
}

int C_4JInput::GetScrollDelta() {
    int v = s_scrollTicksForButtonPressed;
    s_scrollTicksForButtonPressed = 0;
    return v;
}

void C_4JInput::SetDeadzoneAndMovementRange(unsigned int, unsigned int) {}
void C_4JInput::SetGameJoypadMaps(unsigned char, unsigned char, unsigned int) {}
unsigned int C_4JInput::GetGameJoypadMaps(unsigned char, unsigned char) {
    return 0;
}
void C_4JInput::SetJoypadMapVal(int, unsigned char) {}
unsigned char C_4JInput::GetJoypadMapVal(int) { return 0; }
void C_4JInput::SetJoypadSensitivity(int, float) {}
void C_4JInput::SetJoypadStickAxisMap(int, unsigned int, unsigned int) {}
void C_4JInput::SetJoypadStickTriggerMap(int, unsigned int, unsigned int) {}
void C_4JInput::SetKeyRepeatRate(float, float) {}
void C_4JInput::SetDebugSequence(const char*, int (*)(void*), void*) {}
FLOAT C_4JInput::GetIdleSeconds(int) { return 0.f; }
bool C_4JInput::IsPadConnected(int iPad) { return iPad == 0; }

// Silly check, we check if we have a keyboard.
EKeyboardResult C_4JInput::RequestKeyboard(const wchar_t*, const wchar_t*, int,
                                           unsigned int,
                                           int (*)(void*, const bool), void*,
                                           C_4JInput::EKeyboardMode) {
    return EKeyboard_Cancelled;
}

void C_4JInput::GetText(uint16_t* s) {
    if (s) s[0] = 0;
}
bool C_4JInput::VerifyStrings(wchar_t**, int,
                              int (*)(void*, STRING_VERIFY_RESPONSE*), void*) {
    return true;
}
void C_4JInput::CancelQueuedVerifyStrings(int (*)(void*,
                                                  STRING_VERIFY_RESPONSE*),
                                          void*) {}
void C_4JInput::CancelAllVerifyInProgress() {}
