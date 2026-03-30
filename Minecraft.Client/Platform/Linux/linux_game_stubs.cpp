#ifdef __linux__

#include <cstddef>
#include <cstring>
#include <string>
#include <pthread.h>

#include "Stubs/LinuxStubs.h"
#include "../Common/Consoles_App.h"

void Display::update() {}

int CMinecraftApp::GetTPConfigVal(WCHAR* pwchDataFile) { return 0; }

#include "../../Minecraft.World/Platform/x64headers/extraX64.h"

void PIXSetMarkerDeprecated(int a, const char* b, ...) {}

#endif
