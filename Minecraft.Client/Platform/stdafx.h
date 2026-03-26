// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

//#include <xtl.h>
//#include <xboxmath.h>

#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "(" __STR1__(__LINE__) ") : 4J Warning Msg: "

// use  - #pragma message(__LOC__"Need to do something here")

#define AUTO_VAR(_var, _val) auto _var = _val
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifdef __linux__
#include "../Platform/Linux/Stubs/LinuxStubs.h"
#else
typedef unsigned __int64 __uint64;
#endif

#ifdef _WINDOWS64
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <malloc.h>
#include <tchar.h>
#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

#define HRESULT_SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#endif

#include "../../Minecraft.World/Platform/x64headers/extraX64.h"

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

#include "../../Minecraft.World/Util/Definitions.h"
#include "../../Minecraft.World/Util/Class.h"
#include "../../Minecraft.World/Util/ArrayWithLength.h"
#include "../../Minecraft.World/Util/SharedConstants.h"
#include "../../Minecraft.World/Util/Random.h"
#include "../../Minecraft.World/IO/Streams/Compression.h"
#include "../../Minecraft.World/Util/PerformanceTimer.h"

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

#include "../Textures/Textures.h"
#include "../UI/Font.h"
#include "../ClientConstants.h"
#include "../UI/Gui.h"
#include "../UI/Screen.h"
#include "../UI/ScreenSizeCalculator.h"
#include "../Minecraft.h"
#include "../Utils/MemoryTracker.h"
#include "stubs.h"
#include "../Textures/BufferedImage.h"

#include "../Platform/Common/Network/GameNetworkManager.h"

#include "../Platform/Common/UI/UIEnums.h"
#include "../Platform/Common/UI/UIStructs.h"
#include "../Platform/Common/App_Defines.h"
#include "../Platform/Common/App_enums.h"
#include "../Platform/Common/Tutorial/TutorialEnum.h"
#include "../Platform/Common/App_structs.h"

#include "../Platform/Common/Consoles_App.h"
#include "../Platform/Common/Minecraft_Macros.h"
#include "../Platform/Common/BuildVer.h"

// This is generated at build time via scripts/pack_loc.py
#include "strings.h"

#ifdef _WINDOWS64
#include "../Platform/Windows64/Sentient/MinecraftTelemetry.h"
#include "../Platform/Windows64/Windows64_App.h"
#include "../Platform/Windows64/Sentient/DynamicConfigurations.h"
#include "../Platform/Windows64/Sentient/SentientTelemetryCommon.h"
#include "../Platform/Windows64/GameConfig/Minecraft.spa.h"
#include "../Platform/Windows64/XML/ATGXmlParser.h"
#include "../Platform/Windows64/Social/SocialManager.h"
#include "../Platform/Common/Audio/SoundEngine.h"
#include "../Platform/Windows64/Iggy/include/iggy.h"
#include "../Platform/Windows64/Iggy/gdraw/gdraw_d3d11.h"
#include "../Platform/Windows64/Windows64_UIController.h"
#else
// Linux build: keep the Linux runtime/controller path and use the supported
// Linux/Windows64 metadata headers only.
#include "../Platform/Linux/Linux_App.h"
#include "../Platform/Linux/Iggy/include/iggy.h"
#include "../Platform/Linux/Sentient/SentientTelemetryCommon.h"
#include "../Platform/Linux/Sentient/DynamicConfigurations.h"
#include "../Platform/Windows64/GameConfig/Minecraft.spa.h"
#include "../Platform/Common/Audio/SoundEngine.h"
#include "../Platform/Linux/Linux_UIController.h"
#include "../Platform/Linux/Social/SocialManager.h"
#endif

#include "../Platform/Common/ConsoleGameMode.h"
#include "../Platform/Common/Console_Debug_enum.h"
#include "../Platform/Common/Console_Awards_enum.h"
#include "../Platform/Common/Tutorial/TutorialMode.h"
#include "../Platform/Common/Tutorial/Tutorial.h"
#include "../Platform/Common/Tutorial/FullTutorialMode.h"
#include "../Platform/Common/Trial/TrialMode.h"
#include "../Platform/Common/GameRules/ConsoleGameRules.h"
#include "../Platform/Common/GameRules/ConsoleSchematicFile.h"
#include "../Platform/Common/Colours/ColourTable.h"
#include "../Platform/Common/DLC/DLCSkinFile.h"
#include "../Platform/Common/DLC/DLCManager.h"
#include "../Platform/Common/DLC/DLCPack.h"
#include "../Platform/Common/Telemetry/TelemetryManager.h"

#include "extraX64client.h"

#ifdef _FINAL_BUILD
#define printf BREAKTHECOMPILE
#define wprintf BREAKTHECOMPILE
#undef OutputDebugString
#define OutputDebugString BREAKTHECOMPILE
#define OutputDebugStringA BREAKTHECOMPILE
#define OutputDebugStringW BREAKTHECOMPILE
#endif

void MemSect(int sect);
