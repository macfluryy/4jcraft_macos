// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once

#define AUTO_VAR(_var, _val) auto _var = _val

#ifdef _WINDOWS64
typedef unsigned __int64 __uint64;
#endif

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
#include "../../Minecraft.Client/Platform/Linux/Stubs/LinuxStubs.h"
#else
#include <unordered_map>
#include <unordered_set>
#include <sal.h>
#include <vector>
#endif

#include <memory>

#include "x64headers/extraX64.h"

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

#include "../Util/Definitions.h"
#include "../Util/Class.h"
#include "../Util/Exceptions.h"
#include "../Util/Mth.h"
#include "../Util/StringHelpers.h"
#include "../Util/ArrayWithLength.h"
#include "../Util/Random.h"
#include "../Level/TilePos.h"
#include "../Level/ChunkPos.h"
#include "../IO/Streams/Compression.h"
#include "../Util/PerformanceTimer.h"

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
#include "../../Minecraft.Client/Platform/Windows64/4JLibs/inc/4J_Profile.h"
#include "../../Minecraft.Client/Platform/Windows64/4JLibs/inc/4J_Render.h"
#include "../../Minecraft.Client/Platform/Windows64/4JLibs/inc/4J_Storage.h"
#include "../../Minecraft.Client/Platform/Windows64/4JLibs/inc/4J_Input.h"
#else
#include "4J_Profile.h"
#include "4J_Render.h"
#include "4J_Storage.h"
#include "4J_Input.h"
#endif

#include "../../Minecraft.Client/Platform/Common/Network/GameNetworkManager.h"

#include "../../Minecraft.Client/Platform/Common/UI/UIEnums.h"
#include "../../Minecraft.Client/Platform/Common/App_Defines.h"
#include "../../Minecraft.Client/Platform/Common/App_enums.h"
#include "../../Minecraft.Client/Platform/Common/Tutorial/TutorialEnum.h"
#include "../../Minecraft.Client/Platform/Common/App_structs.h"

#include "../../Minecraft.Client/Platform/Common/Consoles_App.h"
#include "../../Minecraft.Client/Platform/Common/Minecraft_Macros.h"
#include "../../Minecraft.Client/Platform/Common/Colours/ColourTable.h"

#include "../../Minecraft.Client/Platform/Common/BuildVer.h"

// This is generated at build time via scripts/pack_loc.py
#include "strings.h"

#ifdef _WINDOWS64
#include "../../Minecraft.Client/Platform/Windows64/Windows64_App.h"
#include "../../Minecraft.Client/Platform/Windows64/Sentient/SentientTelemetryCommon.h"
#include "../../Minecraft.Client/Platform/Windows64/Sentient/MinecraftTelemetry.h"
#else
// Use the Linux runtime path with supported metadata/config headers only.
#include "../../Minecraft.Client/Platform/Linux/Linux_App.h"
#include "../../Minecraft.Client/Platform/Linux/Sentient/SentientTelemetryCommon.h"
#include "../../Minecraft.Client/Platform/Linux/Sentient/DynamicConfigurations.h"
#include "../../Minecraft.Client/Platform/Windows64/GameConfig/Minecraft.spa.h"
#endif

#include "../../Minecraft.Client/Platform/Common/DLC/DLCSkinFile.h"
#include "../../Minecraft.Client/Platform/Common/Console_Awards_enum.h"
#include "../../Minecraft.Client/Platform/Common/Potion_Macros.h"
#include "../../Minecraft.Client/Platform/Common/Console_Debug_enum.h"
#include "../../Minecraft.Client/Platform/Common/GameRules/ConsoleGameRulesConstants.h"
#include "../../Minecraft.Client/Platform/Common/GameRules/ConsoleGameRules.h"
#include "../../Minecraft.Client/Platform/Common/Telemetry/TelemetryManager.h"
