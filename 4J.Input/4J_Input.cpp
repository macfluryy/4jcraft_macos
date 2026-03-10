// 4J_Input.cpp - GLFW keyboard + mouse input for the Linux port
// Replaces the SDL2 oldimpl with GLFW equivalents.
// Uses glfwGetCurrentContext() to get the window the render manager created,
// avoiding a coupling dependency on 4J_Render.h.

#include "4J_Input.h"
#include "../Minecraft.Client/Platform/Common/App_enums.h"
#include <GLFW/glfw3.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

C_4JInput InputManager;

// ---------------------------------------------------------------------------
// State - all static to avoid adding new fields to C_4JInput
// ---------------------------------------------------------------------------

static const int KEY_COUNT = GLFW_KEY_LAST + 1;    // 349 on GLFW 3

static bool s_keysCurrent[KEY_COUNT] = {};
static bool s_keysPrev[KEY_COUNT]    = {};

static bool s_mouseLeftCurrent  = false, s_mouseLeftPrev  = false;
static bool s_mouseRightCurrent = false, s_mouseRightPrev = false;

// Accumulated cursor delta from the GLFW cursor-pos callback.
// Snapshotted into s_frameRelX/Y at Tick() start, then reset to 0.
static double s_lastCursorX = 0.0, s_lastCursorY = 0.0;
static bool   s_cursorInitialized = false;
static float  s_mouseAccumX = 0.0f, s_mouseAccumY = 0.0f; // callback accumulator
static float  s_frameRelX   = 0.0f, s_frameRelY   = 0.0f; // per-frame snapshot

// Scroll wheel
static float s_scrollAccum     = 0.0f;  // callback accumulator
static float s_scrollFrame     = 0.0f;  // current frame snapshot
static float s_scrollPrevFrame = 0.0f;

// Mouse lock / menu state
static bool s_mouseLocked      = false;
static bool s_menuDisplayed[4] = {};
static bool s_prevMenuDisplayed = true; // start as "in menu" so auto-lock triggers after first frame

// Sensitivity: scales raw pixel delta before sqrt-compression
// Smaller value = less mouse movement per pixel
static const float MOUSE_SCALE = 0.012f;

// ---------------------------------------------------------------------------
// GLFW window (obtained lazily via glfwGetCurrentContext on the render thread)
// ---------------------------------------------------------------------------
static GLFWwindow *s_inputWindow = nullptr;

static GLFWwindow *getWindow() {
    if (!s_inputWindow) {
        s_inputWindow = glfwGetCurrentContext();
    }
    return s_inputWindow;
}

// ---------------------------------------------------------------------------
// GLFW callbacks
// ---------------------------------------------------------------------------

static void onCursorPos(GLFWwindow * /*w*/, double x, double y) {
    if (s_cursorInitialized) {
        s_mouseAccumX += (float)(x - s_lastCursorX);
        s_mouseAccumY += (float)(y - s_lastCursorY);
    } else {
        s_cursorInitialized = true;
    }
    s_lastCursorX = x;
    s_lastCursorY = y;
}

static void onScroll(GLFWwindow * /*w*/, double /*xoffset*/, double yoffset) {
    s_scrollAccum += (float)yoffset;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static inline bool KDown(int key) {
    return (key >= 0 && key < KEY_COUNT) ? s_keysCurrent[key] : false;
}
static inline bool KPressed(int key) {
    return (key >= 0 && key < KEY_COUNT) ? (s_keysCurrent[key] && !s_keysPrev[key]) : false;
}
static inline bool KReleased(int key) {
    return (key >= 0 && key < KEY_COUNT) ? (!s_keysCurrent[key] && s_keysPrev[key]) : false;
}

static inline bool MouseLDown()     { return s_mouseLeftCurrent; }
static inline bool MouseLPressed()  { return s_mouseLeftCurrent  && !s_mouseLeftPrev;  }
static inline bool MouseLReleased() { return !s_mouseLeftCurrent && s_mouseLeftPrev;   }
static inline bool MouseRDown()     { return s_mouseRightCurrent; }
static inline bool MouseRPressed()  { return s_mouseRightCurrent && !s_mouseRightPrev; }
static inline bool MouseRReleased() { return !s_mouseRightCurrent && s_mouseRightPrev; }

static inline bool WheelUp()        { return s_scrollFrame >  0.1f; }
static inline bool WheelDown()      { return s_scrollFrame < -0.1f; }
static inline bool WheelUpEdge()    { return s_scrollFrame >  0.1f && s_scrollPrevFrame <=  0.1f; }
static inline bool WheelDownEdge()  { return s_scrollFrame < -0.1f && s_scrollPrevFrame >= -0.1f; }

// Keys to snapshot each Tick (avoid iterating all 349 entries)
static const int s_watchedKeys[] = {
    GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D,
    GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_RIGHT_SHIFT,
    GLFW_KEY_E, GLFW_KEY_Q, GLFW_KEY_F,
    GLFW_KEY_ESCAPE, GLFW_KEY_ENTER,
    GLFW_KEY_F3, GLFW_KEY_F5,
    GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT,
    GLFW_KEY_PAGE_UP, GLFW_KEY_PAGE_DOWN,
    GLFW_KEY_TAB,
    GLFW_KEY_LEFT_CONTROL, GLFW_KEY_RIGHT_CONTROL,
    GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5,
    GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9,
};
static const int s_watchedKeyCount = (int)(sizeof(s_watchedKeys) / sizeof(s_watchedKeys[0]));

// ---------------------------------------------------------------------------
// C_4JInput::Initialise
// ---------------------------------------------------------------------------
void C_4JInput::Initialise(int /*iInputStateC*/, unsigned char /*ucMapC*/,
                           unsigned char /*ucActionC*/, unsigned char /*ucMenuActionC*/) {
    memset(s_keysCurrent, 0, sizeof(s_keysCurrent));
    memset(s_keysPrev,    0, sizeof(s_keysPrev));
    memset(s_menuDisplayed, 0, sizeof(s_menuDisplayed));

    s_mouseLeftCurrent  = s_mouseLeftPrev  = false;
    s_mouseRightCurrent = s_mouseRightPrev = false;
    s_mouseAccumX = s_mouseAccumY = 0.0f;
    s_frameRelX   = s_frameRelY   = 0.0f;
    s_scrollAccum = s_scrollFrame = s_scrollPrevFrame = 0.0f;
    s_mouseLocked = false;
    s_prevMenuDisplayed = true;  // triggers auto-lock once game leaves first menu
    s_cursorInitialized = false;

    GLFWwindow *w = getWindow();
    if (w) {
        glfwSetCursorPosCallback(w, onCursorPos);
        glfwSetScrollCallback(w, onScroll);
        // NOTE: GLFW_RAW_MOUSE_MOTION must only be set when cursor mode is
        // GLFW_CURSOR_DISABLED (Wayland zwp_relative_pointer_v1 requirement).
        // It is activated at the cursor-lock call sites below in Tick().
    }

    printf("[4J_Input] GLFW input initialised\n");
    printf("  WASD=move  Mouse=look  LMB=attack  RMB=use\n");
    printf("  Space=jump  LShift=sneak  E=inventory  Q=drop  Esc=pause\n");
    printf("  F5=3rd-person  F3=debug  Scroll=hotbar\n");
    fflush(stdout);
}

// ---------------------------------------------------------------------------
// C_4JInput::Tick  (called once per frame, BEFORE Present / glfwPollEvents)
// ---------------------------------------------------------------------------
void C_4JInput::Tick(void) {
    GLFWwindow *w = getWindow();
    if (!w) return;

    // 1. Save previous frame
    memcpy(s_keysPrev, s_keysCurrent, sizeof(s_keysCurrent));
    s_mouseLeftPrev  = s_mouseLeftCurrent;
    s_mouseRightPrev = s_mouseRightCurrent;
    s_scrollPrevFrame = s_scrollFrame;

    // 2. Snapshot current keyboard state for watched keys
    for (int i = 0; i < s_watchedKeyCount; i++) {
        int k = s_watchedKeys[i];
        s_keysCurrent[k] = (glfwGetKey(w, k) == GLFW_PRESS);
    }

    // 3. Snapshot and reset scroll accumulator
    s_scrollFrame = s_scrollAccum;
    s_scrollAccum = 0.0f;

    // 4. Mouse-lock management based on menu display state
    bool menuNow = s_menuDisplayed[0];

    if (menuNow && s_mouseLocked) {
        // Re-entered a menu → release mouse cursor
        s_mouseLocked = false;
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(w, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
        glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        // Discard stale delta so the view doesn't jerk on re-lock
        s_mouseAccumX = s_mouseAccumY = 0.0f;
        s_cursorInitialized = false;
    }
    if (!menuNow && s_prevMenuDisplayed && !s_mouseLocked) {
        // Left the menu → lock mouse for look control
        s_mouseLocked = true;
        glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        // Enable raw (un-accelerated) relative motion now that cursor is disabled.
        // On Wayland this activates zwp_relative_pointer_v1 for sub-pixel precise
        // mouse deltas; on X11 it bypasses the compositor acceleration curve.
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(w, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        s_mouseAccumX = s_mouseAccumY = 0.0f;
        s_cursorInitialized = false;
    }
    s_prevMenuDisplayed = menuNow;

    // 5. Snapshot and reset mouse delta from callback
    s_frameRelX = s_mouseAccumX;
    s_frameRelY = s_mouseAccumY;
    s_mouseAccumX = s_mouseAccumY = 0.0f;

    // 6. Mouse buttons (only meaningful when locked in-game)
    if (s_mouseLocked) {
        s_mouseLeftCurrent  = (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT)  == GLFW_PRESS);
        s_mouseRightCurrent = (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
    } else {
        // Not locked. Allow a left-click to re-lock (if not in a menu)
        bool lclick = (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
        if (!menuNow && lclick) {
            s_mouseLocked = true;
            glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            if (glfwRawMouseMotionSupported())
                glfwSetInputMode(w, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            s_mouseAccumX = s_mouseAccumY = 0.0f;
            s_cursorInitialized = false;
        }
        s_mouseLeftCurrent  = false;
        s_mouseRightCurrent = false;
        s_frameRelX = s_frameRelY = 0.0f;
    }
}

// ---------------------------------------------------------------------------
// ButtonDown – is action held this frame?
// ---------------------------------------------------------------------------
bool C_4JInput::ButtonDown(int /*iPad*/, unsigned char ucAction) {
    switch (ucAction) {
        // ---- Menu navigation ----
        case ACTION_MENU_UP:    return KDown(GLFW_KEY_UP)    || KDown(GLFW_KEY_W);
        case ACTION_MENU_DOWN:  return KDown(GLFW_KEY_DOWN)  || KDown(GLFW_KEY_S);
        case ACTION_MENU_LEFT:  return KDown(GLFW_KEY_LEFT)  || KDown(GLFW_KEY_A);
        case ACTION_MENU_RIGHT: return KDown(GLFW_KEY_RIGHT) || KDown(GLFW_KEY_D);

        case ACTION_MENU_A:
        case ACTION_MENU_OK:
            return KDown(GLFW_KEY_ENTER) || KDown(GLFW_KEY_SPACE);

        case ACTION_MENU_B:
        case ACTION_MENU_CANCEL:
            return KDown(GLFW_KEY_ESCAPE);

        case ACTION_MENU_X:           return KDown(GLFW_KEY_F);
        case ACTION_MENU_Y:           return KDown(GLFW_KEY_E);

        case ACTION_MENU_PAGEUP:      return KDown(GLFW_KEY_PAGE_UP)   || KDown(GLFW_KEY_LEFT_SHIFT);
        case ACTION_MENU_PAGEDOWN:    return KDown(GLFW_KEY_PAGE_DOWN) || KDown(GLFW_KEY_RIGHT_SHIFT);
        case ACTION_MENU_RIGHT_SCROLL: return KDown(GLFW_KEY_RIGHT_SHIFT) || WheelUp();
        case ACTION_MENU_LEFT_SCROLL:  return KDown(GLFW_KEY_LEFT_SHIFT)  || WheelDown();

        case ACTION_MENU_STICK_PRESS:        return false;
        case ACTION_MENU_OTHER_STICK_PRESS:  return false;
        case ACTION_MENU_OTHER_STICK_UP:     return KDown(GLFW_KEY_UP);
        case ACTION_MENU_OTHER_STICK_DOWN:   return KDown(GLFW_KEY_DOWN);
        case ACTION_MENU_OTHER_STICK_LEFT:   return KDown(GLFW_KEY_LEFT);
        case ACTION_MENU_OTHER_STICK_RIGHT:  return KDown(GLFW_KEY_RIGHT);

        case ACTION_MENU_PAUSEMENU: return KDown(GLFW_KEY_ESCAPE);

        // ---- Minecraft in-game ----
        case MINECRAFT_ACTION_JUMP:     return KDown(GLFW_KEY_SPACE);
        case MINECRAFT_ACTION_FORWARD:  return KDown(GLFW_KEY_W);
        case MINECRAFT_ACTION_BACKWARD: return KDown(GLFW_KEY_S);
        case MINECRAFT_ACTION_LEFT:     return KDown(GLFW_KEY_A);
        case MINECRAFT_ACTION_RIGHT:    return KDown(GLFW_KEY_D);

        // Look axes are handled by analog stick RX/RY (mouse)
        case MINECRAFT_ACTION_LOOK_LEFT:  return false;
        case MINECRAFT_ACTION_LOOK_RIGHT: return false;
        case MINECRAFT_ACTION_LOOK_UP:    return false;
        case MINECRAFT_ACTION_LOOK_DOWN:  return false;

        case MINECRAFT_ACTION_USE:           return MouseRDown();
        case MINECRAFT_ACTION_ACTION:        return MouseLDown();
        case MINECRAFT_ACTION_LEFT_SCROLL:   return WheelDown();
        case MINECRAFT_ACTION_RIGHT_SCROLL:  return WheelUp();

        case MINECRAFT_ACTION_INVENTORY:           return KDown(GLFW_KEY_E);
        case MINECRAFT_ACTION_PAUSEMENU:           return KDown(GLFW_KEY_ESCAPE);
        case MINECRAFT_ACTION_DROP:                return KDown(GLFW_KEY_Q);
        case MINECRAFT_ACTION_SNEAK_TOGGLE:        return KDown(GLFW_KEY_LEFT_SHIFT);
        case MINECRAFT_ACTION_CRAFTING:            return KDown(GLFW_KEY_F);
        case MINECRAFT_ACTION_RENDER_THIRD_PERSON: return KDown(GLFW_KEY_F5);
        case MINECRAFT_ACTION_GAME_INFO:           return KDown(GLFW_KEY_F3);

        case MINECRAFT_ACTION_DPAD_LEFT:  return KDown(GLFW_KEY_LEFT);
        case MINECRAFT_ACTION_DPAD_RIGHT: return KDown(GLFW_KEY_RIGHT);
        case MINECRAFT_ACTION_DPAD_UP:    return KDown(GLFW_KEY_UP);
        case MINECRAFT_ACTION_DPAD_DOWN:  return KDown(GLFW_KEY_DOWN);

        default: return false;
    }
}

// ---------------------------------------------------------------------------
// ButtonPressed – rising edge (press event this frame)
// ---------------------------------------------------------------------------
bool C_4JInput::ButtonPressed(int /*iPad*/, unsigned char ucAction) {
    switch (ucAction) {
        case ACTION_MENU_UP:    return KPressed(GLFW_KEY_UP)    || KPressed(GLFW_KEY_W);
        case ACTION_MENU_DOWN:  return KPressed(GLFW_KEY_DOWN)  || KPressed(GLFW_KEY_S);
        case ACTION_MENU_LEFT:  return KPressed(GLFW_KEY_LEFT)  || KPressed(GLFW_KEY_A);
        case ACTION_MENU_RIGHT: return KPressed(GLFW_KEY_RIGHT) || KPressed(GLFW_KEY_D);

        case ACTION_MENU_A:
        case ACTION_MENU_OK:
            return KPressed(GLFW_KEY_ENTER) || KPressed(GLFW_KEY_SPACE);

        case ACTION_MENU_B:
        case ACTION_MENU_CANCEL:
            return KPressed(GLFW_KEY_ESCAPE);

        case ACTION_MENU_X:  return KPressed(GLFW_KEY_F);
        case ACTION_MENU_Y:  return KPressed(GLFW_KEY_E);

        case ACTION_MENU_PAGEUP:       return KPressed(GLFW_KEY_PAGE_UP)   || KPressed(GLFW_KEY_LEFT_SHIFT);
        case ACTION_MENU_PAGEDOWN:     return KPressed(GLFW_KEY_PAGE_DOWN) || KPressed(GLFW_KEY_RIGHT_SHIFT);
        case ACTION_MENU_RIGHT_SCROLL: return KPressed(GLFW_KEY_RIGHT_SHIFT) || WheelUpEdge();
        case ACTION_MENU_LEFT_SCROLL:  return KPressed(GLFW_KEY_LEFT_SHIFT)  || WheelDownEdge();

        case ACTION_MENU_STICK_PRESS:        return false;
        case ACTION_MENU_OTHER_STICK_PRESS:  return false;
        case ACTION_MENU_OTHER_STICK_UP:     return KPressed(GLFW_KEY_UP);
        case ACTION_MENU_OTHER_STICK_DOWN:   return KPressed(GLFW_KEY_DOWN);
        case ACTION_MENU_OTHER_STICK_LEFT:   return KPressed(GLFW_KEY_LEFT);
        case ACTION_MENU_OTHER_STICK_RIGHT:  return KPressed(GLFW_KEY_RIGHT);
        case ACTION_MENU_PAUSEMENU:          return KPressed(GLFW_KEY_ESCAPE);

        case MINECRAFT_ACTION_JUMP:     return KPressed(GLFW_KEY_SPACE);
        case MINECRAFT_ACTION_FORWARD:  return KPressed(GLFW_KEY_W);
        case MINECRAFT_ACTION_BACKWARD: return KPressed(GLFW_KEY_S);
        case MINECRAFT_ACTION_LEFT:     return KPressed(GLFW_KEY_A);
        case MINECRAFT_ACTION_RIGHT:    return KPressed(GLFW_KEY_D);

        case MINECRAFT_ACTION_LOOK_LEFT:
        case MINECRAFT_ACTION_LOOK_RIGHT:
        case MINECRAFT_ACTION_LOOK_UP:
        case MINECRAFT_ACTION_LOOK_DOWN:
            return false;

        case MINECRAFT_ACTION_USE:           return MouseRPressed();
        case MINECRAFT_ACTION_ACTION:        return MouseLPressed();
        case MINECRAFT_ACTION_LEFT_SCROLL:   return WheelDownEdge();
        case MINECRAFT_ACTION_RIGHT_SCROLL:  return WheelUpEdge();

        case MINECRAFT_ACTION_INVENTORY:           return KPressed(GLFW_KEY_E);
        case MINECRAFT_ACTION_PAUSEMENU:           return KPressed(GLFW_KEY_ESCAPE);
        case MINECRAFT_ACTION_DROP:                return KPressed(GLFW_KEY_Q);
        case MINECRAFT_ACTION_SNEAK_TOGGLE:        return KPressed(GLFW_KEY_LEFT_SHIFT);
        case MINECRAFT_ACTION_CRAFTING:            return KPressed(GLFW_KEY_F);
        case MINECRAFT_ACTION_RENDER_THIRD_PERSON: return KPressed(GLFW_KEY_F5);
        case MINECRAFT_ACTION_GAME_INFO:           return KPressed(GLFW_KEY_F3);

        case MINECRAFT_ACTION_DPAD_LEFT:  return KPressed(GLFW_KEY_LEFT);
        case MINECRAFT_ACTION_DPAD_RIGHT: return KPressed(GLFW_KEY_RIGHT);
        case MINECRAFT_ACTION_DPAD_UP:    return KPressed(GLFW_KEY_UP);
        case MINECRAFT_ACTION_DPAD_DOWN:  return KPressed(GLFW_KEY_DOWN);

        default: return false;
    }
}

// ---------------------------------------------------------------------------
// ButtonReleased – falling edge (released this frame)
// ---------------------------------------------------------------------------
bool C_4JInput::ButtonReleased(int /*iPad*/, unsigned char ucAction) {
    switch (ucAction) {
        case ACTION_MENU_UP:    return KReleased(GLFW_KEY_UP)    || KReleased(GLFW_KEY_W);
        case ACTION_MENU_DOWN:  return KReleased(GLFW_KEY_DOWN)  || KReleased(GLFW_KEY_S);
        case ACTION_MENU_LEFT:  return KReleased(GLFW_KEY_LEFT)  || KReleased(GLFW_KEY_A);
        case ACTION_MENU_RIGHT: return KReleased(GLFW_KEY_RIGHT) || KReleased(GLFW_KEY_D);

        case ACTION_MENU_A:
        case ACTION_MENU_OK:
            return KReleased(GLFW_KEY_ENTER) || KReleased(GLFW_KEY_SPACE);

        case ACTION_MENU_B:
        case ACTION_MENU_CANCEL:
            return KReleased(GLFW_KEY_ESCAPE);

        case ACTION_MENU_X:  return KReleased(GLFW_KEY_F);
        case ACTION_MENU_Y:  return KReleased(GLFW_KEY_E);

        case ACTION_MENU_PAGEUP:       return KReleased(GLFW_KEY_PAGE_UP)   || KReleased(GLFW_KEY_LEFT_SHIFT);
        case ACTION_MENU_PAGEDOWN:     return KReleased(GLFW_KEY_PAGE_DOWN) || KReleased(GLFW_KEY_RIGHT_SHIFT);
        case ACTION_MENU_RIGHT_SCROLL: return KReleased(GLFW_KEY_RIGHT_SHIFT);
        case ACTION_MENU_LEFT_SCROLL:  return KReleased(GLFW_KEY_LEFT_SHIFT);

        case ACTION_MENU_STICK_PRESS:        return false;
        case ACTION_MENU_OTHER_STICK_PRESS:  return false;
        case ACTION_MENU_OTHER_STICK_UP:     return KReleased(GLFW_KEY_UP);
        case ACTION_MENU_OTHER_STICK_DOWN:   return KReleased(GLFW_KEY_DOWN);
        case ACTION_MENU_OTHER_STICK_LEFT:   return KReleased(GLFW_KEY_LEFT);
        case ACTION_MENU_OTHER_STICK_RIGHT:  return KReleased(GLFW_KEY_RIGHT);
        case ACTION_MENU_PAUSEMENU:          return KReleased(GLFW_KEY_ESCAPE);

        case MINECRAFT_ACTION_JUMP:     return KReleased(GLFW_KEY_SPACE);
        case MINECRAFT_ACTION_FORWARD:  return KReleased(GLFW_KEY_W);
        case MINECRAFT_ACTION_BACKWARD: return KReleased(GLFW_KEY_S);
        case MINECRAFT_ACTION_LEFT:     return KReleased(GLFW_KEY_A);
        case MINECRAFT_ACTION_RIGHT:    return KReleased(GLFW_KEY_D);

        case MINECRAFT_ACTION_LOOK_LEFT:
        case MINECRAFT_ACTION_LOOK_RIGHT:
        case MINECRAFT_ACTION_LOOK_UP:
        case MINECRAFT_ACTION_LOOK_DOWN:
            return false;

        case MINECRAFT_ACTION_USE:           return MouseRReleased();
        case MINECRAFT_ACTION_ACTION:        return MouseLReleased();
        case MINECRAFT_ACTION_LEFT_SCROLL:   return false; // scroll wheel has no "release"
        case MINECRAFT_ACTION_RIGHT_SCROLL:  return false;

        case MINECRAFT_ACTION_INVENTORY:           return KReleased(GLFW_KEY_E);
        case MINECRAFT_ACTION_PAUSEMENU:           return KReleased(GLFW_KEY_ESCAPE);
        case MINECRAFT_ACTION_DROP:                return KReleased(GLFW_KEY_Q);
        case MINECRAFT_ACTION_SNEAK_TOGGLE:        return KReleased(GLFW_KEY_LEFT_SHIFT);
        case MINECRAFT_ACTION_CRAFTING:            return KReleased(GLFW_KEY_F);
        case MINECRAFT_ACTION_RENDER_THIRD_PERSON: return KReleased(GLFW_KEY_F5);
        case MINECRAFT_ACTION_GAME_INFO:           return KReleased(GLFW_KEY_F3);

        case MINECRAFT_ACTION_DPAD_LEFT:  return KReleased(GLFW_KEY_LEFT);
        case MINECRAFT_ACTION_DPAD_RIGHT: return KReleased(GLFW_KEY_RIGHT);
        case MINECRAFT_ACTION_DPAD_UP:    return KReleased(GLFW_KEY_UP);
        case MINECRAFT_ACTION_DPAD_DOWN:  return KReleased(GLFW_KEY_DOWN);

        default: return false;
    }
}

// ---------------------------------------------------------------------------
// GetValue – returns 1 if action held, 0 otherwise
// ---------------------------------------------------------------------------
unsigned int C_4JInput::GetValue(int iPad, unsigned char ucAction, bool /*bRepeat*/) {
    return ButtonDown(iPad, ucAction) ? 1u : 0u;
}

// ---------------------------------------------------------------------------
// Analog sticks
//
// Left stick  = WASD keyboard (±1.0)
// Right stick = mouse delta with sqrt-compression to linearise the quadratic
//               look-speed formula used by the console game code:
//               turnSpeed = rx * |rx| * 50 * sensitivity
//               Passing sqrt(|raw|)*sign(raw) makes turn speed proportional
//               to raw pixel delta, giving a natural mouse feel.
// ---------------------------------------------------------------------------

float C_4JInput::GetJoypadStick_LX(int /*iPad*/, bool /*bCheckMenuDisplay*/) {
    // Return 0 while cursor is still in menu mode so Input::tick()'s
    // lReset guard sees a zero-stick frame during the menu->game transition.
    // Once s_mouseLocked becomes true (cursor captured for gameplay), return
    // the real WASD values.
    if (!s_mouseLocked) return 0.0f;
    float v = 0.0f;
    if (KDown(GLFW_KEY_A)) v -= 1.0f;
    if (KDown(GLFW_KEY_D)) v += 1.0f;
    return v;
}

float C_4JInput::GetJoypadStick_LY(int /*iPad*/, bool /*bCheckMenuDisplay*/) {
    if (!s_mouseLocked) return 0.0f;
    float v = 0.0f;
    if (KDown(GLFW_KEY_W)) v += 1.0f;  // W = forward = negative Y on consoles
    if (KDown(GLFW_KEY_S)) v -= 1.0f;
    return v;
}

float C_4JInput::GetJoypadStick_RX(int /*iPad*/, bool /*bCheckMenuDisplay*/) {
    if (!s_mouseLocked) return 0.0f;
    float raw = s_frameRelX * MOUSE_SCALE;
    float absRaw = fabsf(raw);
    if (absRaw > 1.0f) absRaw = 1.0f;
    if (absRaw < 0.0001f) return 0.0f;
    return (raw >= 0.0f ? 1.0f : -1.0f) * sqrtf(absRaw);
}

float C_4JInput::GetJoypadStick_RY(int /*iPad*/, bool /*bCheckMenuDisplay*/) {
    if (!s_mouseLocked) return 0.0f;
    float raw = -s_frameRelY * MOUSE_SCALE;
    float absRaw = fabsf(raw);
    if (absRaw > 1.0f) absRaw = 1.0f;
    if (absRaw < 0.0001f) return 0.0f;
    return (raw >= 0.0f ? 1.0f : -1.0f) * sqrtf(absRaw);
}

// Left trigger  = right mouse button (use/place)
// Right trigger = left mouse button  (attack/destroy)
unsigned char C_4JInput::GetJoypadLTrigger(int /*iPad*/, bool /*bCheckMenuDisplay*/) {
    return s_mouseRightCurrent ? 255 : 0;
}
unsigned char C_4JInput::GetJoypadRTrigger(int /*iPad*/, bool /*bCheckMenuDisplay*/) {
    return s_mouseLeftCurrent ? 255 : 0;
}

// ---------------------------------------------------------------------------
// Joypad map / sensitivity stubs (not meaningful for keyboard+mouse)
// ---------------------------------------------------------------------------
void C_4JInput::SetDeadzoneAndMovementRange(unsigned int /*uiDeadzone*/, unsigned int /*uiMovementRangeMax*/) {}
void C_4JInput::SetGameJoypadMaps(unsigned char /*ucMap*/, unsigned char /*ucAction*/, unsigned int /*uiActionVal*/) {}
unsigned int C_4JInput::GetGameJoypadMaps(unsigned char /*ucMap*/, unsigned char /*ucAction*/) { return 0; }
void C_4JInput::SetJoypadMapVal(int /*iPad*/, unsigned char /*ucMap*/) {}
unsigned char C_4JInput::GetJoypadMapVal(int /*iPad*/) { return 0; }
void C_4JInput::SetJoypadSensitivity(int /*iPad*/, float /*fSensitivity*/) {}
void C_4JInput::SetJoypadStickAxisMap(int /*iPad*/, unsigned int /*uiFrom*/, unsigned int /*uiTo*/) {}
void C_4JInput::SetJoypadStickTriggerMap(int /*iPad*/, unsigned int /*uiFrom*/, unsigned int /*uiTo*/) {}
void C_4JInput::SetKeyRepeatRate(float /*fRepeatDelaySecs*/, float /*fRepeatRateSecs*/) {}
void C_4JInput::SetDebugSequence(const char * /*chSequenceA*/, int(*/*Func*/)(LPVOID), LPVOID /*lpParam*/) {}
FLOAT C_4JInput::GetIdleSeconds(int /*iPad*/) { return 0.0f; }
bool  C_4JInput::IsPadConnected(int iPad)     { return iPad == 0; }  // slot 0 = keyboard+mouse

void C_4JInput::SetMenuDisplayed(int iPad, bool bVal) {
    if (iPad >= 0 && iPad < 4) s_menuDisplayed[iPad] = bVal;
}

// ---------------------------------------------------------------------------
// Keyboard (text entry) / string verification stubs
// ---------------------------------------------------------------------------
EKeyboardResult C_4JInput::RequestKeyboard(const wchar_t * /*Title*/, const wchar_t * /*Text*/, int /*iPad*/,
                                           unsigned int /*uiMaxChars*/,
                                           int(*/*Func*/)(void *, const bool), void * /*lpParam*/,
                                           C_4JInput::EKeyboardMode /*eMode*/) {
    return EKeyboard_Cancelled;
}
void C_4JInput::GetText(uint16_t *UTF16String) { if (UTF16String) UTF16String[0] = 0; }
bool C_4JInput::VerifyStrings(WCHAR ** /*pwStringA*/, int /*iStringC*/,
                              int(*/*Func*/)(LPVOID, STRING_VERIFY_RESPONSE *), LPVOID /*lpParam*/) { return true; }
void C_4JInput::CancelQueuedVerifyStrings(int(*/*Func*/)(LPVOID, STRING_VERIFY_RESPONSE *), LPVOID /*lpParam*/) {}
void C_4JInput::CancelAllVerifyInProgress(void) {}
