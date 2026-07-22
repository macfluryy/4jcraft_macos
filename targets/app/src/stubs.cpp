
#include "app/include/stubs.h"

#include "app/mac/MacGame.h"
#include "platform/JavaKeyInput.h"
#if defined(__linux__) || defined(__APPLE__)

void LinuxLogStubLightmapProbe() {
    static bool logged = false;
    if (logged) return;

    logged = true;
    app.DebugPrintf(
        "[linux-lightmap] stubs.cpp: Linux excludes the no-op multitexture "
        "stubs in this file; the runtime uses libGL/4jlibs symbols.\n");
}

#else

void glReadPixels(int, int, int, int, int, int, ByteBuffer*) {}

void glClearDepth(double) {}

void glVertexPointer(int, int, int, int) {}

void glVertexPointer(int, int, FloatBuffer*) {}

void glTexCoordPointer(int, int, int, int) {}

void glTexCoordPointer(int, int, FloatBuffer*) {}

void glNormalPointer(int, int, int) {}

void glNormalPointer(int, ByteBuffer*) {}

void glEnableClientState(int) {}

void glDisableClientState(int) {}

void glColorPointer(int, int, int, int) {}

void glColorPointer(int, bool, int, ByteBuffer*) {}

void glDrawArrays(int, int, int) {}

void glNormal3f(float, float, float) {}

void glGenQueriesARB(IntBuffer*) {}

void glBeginQueryARB(int, int) {}

void glEndQueryARB(int) {}

void glGetQueryObjectuARB(int, int, IntBuffer*) {}

void glShadeModel(int) {}

void glColorMaterial(int, int) {}


void glClientActiveTexture(int) {}

void glActiveTexture(int) {}

void glFlush() {}

void glTexGeni(int, int, int) {}

#endif

#include "strings.h"
#include <SDL2/SDL.h>

#include "util/StringHelpers.h"

std::vector<int> JavaKeyInput::pressedKeys;
std::vector<wchar_t> JavaKeyInput::typedChars;
bool JavaKeyInput::keysCurrent[512] = {};
bool JavaKeyInput::keysPrev[512] = {};

void Keyboard::update() {
    for (int i = 0; i < 512; i++) {
        JavaKeyInput::keysPrev[i] = JavaKeyInput::keysCurrent[i];
    }
    JavaKeyInput::pressedKeys.clear();
    JavaKeyInput::typedChars.clear();
}

bool Keyboard::isKeyDown(int key) {
    if (key >= 0 && key < 512) return JavaKeyInput::keysCurrent[key];
    return false;
}

bool Keyboard::isKeyPressed(int key) {
    if (key >= 0 && key < 512) {
        return !JavaKeyInput::keysPrev[key] && JavaKeyInput::keysCurrent[key];
    }
    return false;
}

std::wstring Keyboard::getKeyName(int key) {
    if (key < 0) return L"Unknown";

    const char* name = SDL_GetScancodeName((SDL_Scancode)key);
    if (name != nullptr && name[0] != '\0') {
        return convStringToWstring(name);
    }
    return L"Unknown";
}

void Keyboard::enableRepeatEvents(bool repeat) {
    if (repeat) {
        SDL_StartTextInput();
    } else {
        SDL_StopTextInput();
    }
}
