// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _WINDOWS64
#include "Minecraft.Client/Windows64/Windows64_App.h"
#include "Minecraft.Client/Windows64/XML/ATGXmlParser.h"
#include "Minecraft.Client/Common/Source Files/Audio/SoundEngine.h"
#include "Minecraft.Client/Windows64/Iggy/include/iggy.h"
#include "Minecraft.Client/Windows64/Iggy/gdraw/gdraw_d3d11.h"
#include "Minecraft.Client/Windows64/Windows64_UIController.h"
#else
#include "Minecraft.Client/Linux/Stubs/LinuxStubs.h"
#include "Minecraft.Client/Linux/Linux_App.h"
#include "Minecraft.Client/Linux/Iggy/include/iggy.h"
#include "Minecraft.Client/Common/Source Files/Audio/SoundEngine.h"
#include "Minecraft.Client/Linux/Linux_UIController.h"
#endif
