// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once

#ifdef _WINDOWS64
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <malloc.h>
#include <tchar.h>
#include <d3d11.h>
#endif

#ifdef __linux__
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Minecraft.Client/Linux/Stubs/LinuxStubs.h"
#else
#include <unordered_map>
#include <unordered_set>
#include <sal.h>
#include <vector>
#endif

#include <memory>

#include "../x64headers/extraX64.h"

#include <list>
#include <map>
#include <set>
#include <queue>
#include <deque>
#include <algorithm>
#include <math.h>
#include <limits>
#include <string>
#include <sstream>
#include <iostream>
#include <exception>

#include <assert.h>

#include "../ConsoleHelpers/Definitions.h"
#include "../ConsoleJavaLibs/Class.h"
#include "../ConsoleJavaLibs/Exceptions.h"
#include "../net/minecraft/util/Mth.h"
#include "../ConsoleHelpers/StringHelpers.h"
#include "../ConsoleHelpers/ArrayWithLength.h"
#include "../ConsoleJavaLibs/Random.h"
#include "../net/minecraft/world/level/TilePos.h"
#include "../net/minecraft/world/level/ChunkPos.h"
#include "../Header Files/compression.h"
#include "../ConsoleHelpers/PerformanceTimer.h"

#ifdef _FINAL_BUILD
#define printf BREAKTHECOMPILE
#define wprintf BREAKTHECOMPILE
#undef OutputDebugString
#define OutputDebugString BREAKTHECOMPILE
#define OutputDebugStringA BREAKTHECOMPILE
#define OutputDebugStringW BREAKTHECOMPILE
#endif

void MemSect(int sect);

#ifdef _WINDOWS64
#include "Minecraft.Client/Platform/Windows64/4JLibs/inc/4J_Profile.h"
#include "Minecraft.Client/Platform/Windows64/4JLibs/inc/4J_Render.h"
#include "Minecraft.Client/Platform/Windows64/4JLibs/inc/4J_Storage.h"
#include "Minecraft.Client/Platform/Windows64/4JLibs/inc/4J_Input.h"
#else
#include "4J_Profile.h"
#include "4J_Render.h"
#include "4J_Storage.h"
#include "4J_Input.h"
#endif

#include "Minecraft.Client/Common/Source Files/Network/GameNetworkManager.h"

#include "Minecraft.Client/Common/Source Files/UI/All Platforms/UIEnums.h"
#include "Minecraft.Client/Common/App_Defines.h"
#include "Minecraft.Client/Common/App_enums.h"
#include "Minecraft.Client/Common/Source Files/Tutorial/TutorialEnum.h"
#include "Minecraft.Client/Common/App_structs.h"

#include "Minecraft.Client/Common/Consoles_App.h"
#include "Minecraft.Client/Common/Minecraft_Macros.h"
#include "Minecraft.Client/Common/Source Files/Colours/ColourTable.h"

#include "Minecraft.Client/Common/Source Files/BuildVer/BuildVer.h"

// This is generated at build time via scripts/pack_loc.py
#include "strings.h"

#ifdef _WINDOWS64
#include "Minecraft.Client/Windows64/Windows64_App.h"
#include "Minecraft.Client/Windows64/Source Files/Sentient/SentientTelemetryCommon.h"
#include "Minecraft.Client/Windows64/Source Files/Sentient/MinecraftTelemetry.h"
#else
// Use the Linux runtime path with supported metadata/config headers only.
#include "Minecraft.Client/Linux/Linux_App.h"
#include "Minecraft.Client/Linux/Sentient/SentientTelemetryCommon.h"
#include "Minecraft.Client/Linux/Sentient/DynamicConfigurations.h"
#endif

#include "Minecraft.Client/Common/Source Files/DLC/DLCSkinFile.h"
#include "Minecraft.Client/Common/Source Files/Console_Awards_enum.h"
#include "Minecraft.Client/Common/Potion_Macros.h"
#include "Minecraft.Client/Common/Source Files/Console_Debug_enum.h"
#include "Minecraft.Client/Common/Source Files/GameRules/ConsoleGameRulesConstants.h"
#include "Minecraft.Client/Common/Source Files/GameRules/ConsoleGameRules.h"
#include "Minecraft.Client/Common/Source Files/Telemetry/TelemetryManager.h"
