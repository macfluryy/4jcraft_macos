// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _WINDOWS64
#include "app/common/src/Audio/SoundEngine.h"
#include "app/windows/Iggy/gdraw/gdraw_d3d11.h"
#include "app/windows/Iggy/include/iggy.h"
#include "app/windows/Windows64_App.h"
#include "app/windows/Windows64_UIController.h"
#include "app/windows/XML/ATGXmlParser.h"
#else
#include "app/common/src/Audio/SoundEngine.h"
#include "app/linux/Iggy/include/iggy.h"
#include "app/linux/Linux_App.h"
#include "app/linux/Linux_UIController.h"
#include "app/linux/Stubs/LinuxStubs.h"
#endif
