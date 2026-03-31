// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// #include <xtl.h>
// #include <xboxmath.h>

#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "(" __STR1__(__LINE__) ") : 4J Warning Msg: "

// use  - #pragma message(__LOC__"Need to do something here")

#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifdef __linux__
#include "../Linux/Stubs/LinuxStubs.h"
#endif

#ifdef _WINDOWS64
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <malloc.h>
#include <tchar.h>
#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

#define HRESULT_SUCCEEDED(hr) (((int32_t)(hr)) >= 0)
#endif

#include "Minecraft.World/x64headers/extraX64.h"

#include <memory>

#include <list>
#include <map>
#include <set>
#include <deque>
#include <algorithm>
#include <string>
#include <sstream>
#include <iostream>
#include <exception>

#include <assert.h>

#include "Minecraft.World/ConsoleHelpers/Definitions.h"
#include "java/Class.h"
#include "Minecraft.World/ConsoleHelpers/ArrayWithLength.h"
#include "Minecraft.World/net/minecraft/SharedConstants.h"
#include "java/Random.h"
#include "Minecraft.World/Header Files/compression.h"
#include "Minecraft.World/ConsoleHelpers/PerformanceTimer.h"

#ifdef _WINDOWS64
#include "../Platform/Windows64/4JLibs/inc/4J_Input.h"
#include "../Platform/Windows64/4JLibs/inc/4J_Profile.h"
#include "../Platform/Windows64/4JLibs/inc/4J_Render.h"
#include "../Platform/Windows64/4JLibs/inc/4J_Storage.h"
#else
#include "4J_Input.h"
#include "4J_Profile.h"
#include "4J_Render.h"
#include "4J_Storage.h"
#endif

#include "../net/minecraft/client/renderer/Textures.h"
#include "../net/minecraft/client/gui/Font.h"
#include "../net/minecraft/client/ClientConstants.h"
#include "../net/minecraft/client/gui/Gui.h"
#include "../net/minecraft/client/gui/Screen.h"
#include "../net/minecraft/client/gui/ScreenSizeCalculator.h"
#include "../net/minecraft/client/Minecraft.h"
#include "../net/minecraft/client/MemoryTracker.h"
#include "stubs.h"
#include "BufferedImage.h"

#include "../Common/Source Files/Network/GameNetworkManager.h"

#include "../Common/Source Files/UI/All Platforms/UIEnums.h"
#include "../Common/Source Files/UI/All Platforms/UIStructs.h"
#include "../Common/App_Defines.h"
#include "../Common/App_enums.h"
#include "../Common/Source Files/Tutorial/TutorialEnum.h"
#include "../Common/App_structs.h"

#include "../Common/Consoles_App.h"
#include "../Common/Minecraft_Macros.h"
#include "../Common/Source Files/BuildVer/BuildVer.h"

// This is generated at build time via scripts/pack_loc.py
#include "strings.h"

#ifdef _WINDOWS64
#include "../Windows64/Windows64_App.h"
#include "Minecraft.spa.h"
#include "../Windows64/XML/ATGXmlParser.h"
#include "../Common/Source Files/Audio/SoundEngine.h"
#include "../Windows64/Iggy/include/iggy.h"
#include "../Windows64/Iggy/gdraw/gdraw_d3d11.h"
#include "../Windows64/Windows64_UIController.h"
#else
#include "../Linux/Linux_App.h"
#include "../Linux/Iggy/include/iggy.h"
#include "Minecraft.spa.h"
#include "../Common/Source Files/Audio/SoundEngine.h"
#include "../Linux/Linux_UIController.h"
#endif

#include "../Common/Source Files/ConsoleGameMode.h"
#include "../Common/Source Files/Console_Debug_enum.h"
#include "../Common/Source Files/Console_Awards_enum.h"
#include "../Common/Source Files/Tutorial/TutorialMode.h"
#include "../Common/Source Files/Tutorial/Tutorial.h"
#include "../Common/Source Files/Tutorial/FullTutorialMode.h"
#include "../Common/Source Files/Trial/TrialMode.h"
#include "../Common/Source Files/GameRules/ConsoleGameRules.h"
#include "../Common/Source Files/GameRules/LevelGeneration/ConsoleSchematicFile.h"
#include "../Common/Source Files/Colours/ColourTable.h"
#include "../Common/Source Files/DLC/DLCSkinFile.h"
#include "../Common/Source Files/DLC/DLCManager.h"
#include "../Common/Source Files/DLC/DLCPack.h"

#include "extraX64client.h"

#ifdef _FINAL_BUILD
#define printf BREAKTHECOMPILE
#define wprintf BREAKTHECOMPILE
#undef OutputDebugString
#define OutputDebugString BREAKTHECOMPILE
#define OutputDebugStringA BREAKTHECOMPILE
#define OutputDebugStringW BREAKTHECOMPILE
#endif

