// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once

#ifdef __PS3__
#else
#define AUTO_VAR(_var, _val) auto _var = _val
#endif

#if ( defined _XBOX || defined _WINDOWS64  || defined _DURANGO )
typedef unsigned __int64 __uint64;
#endif

#ifdef _WINDOWS64
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <malloc.h>
#include <tchar.h>
// TODO: reference additional headers your program requires here
#include <d3d11.h>
#endif

#ifdef _DURANGO
#include <xdk.h>
#include <wrl.h>
#include <d3d11_x.h>
#include <DirectXMath.h>
using namespace DirectX;
#include <pix.h>
#include "../../Minecraft.Client/Platform/Durango/DurangoExtras/DurangoStubs.h"
#endif

#if (defined __PS3__ || defined _XBOX )
// C RunTime Header Files
#include <stdlib.h>
#endif

#ifdef __ORBIS__
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <scebase.h>
#include <kernel.h>
#include <fios2.h>
#include <message_dialog.h>
#endif

#ifdef _XBOX
#include <xtl.h>
#include <xboxmath.h>
#include <xuiapp.h>
#include <xact3.h>
typedef XINVITE_INFO INVITE_INFO;
typedef XUID PlayerUID;
typedef XNKID SessionID;
typedef XUID GameSessionUID;
#endif

#ifdef __PS3__
#include <cell/l10n.h>
#include <cell/pad.h>
#include <cell/cell_fs.h>
#include <sys/process.h>
#include <sys/ppu_thread.h>
#include <cell/sysmodule.h>
#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_savedata.h>
#include <sysutil/sysutil_sysparam.h>


#include "../../Minecraft.Client/Platform/PS3/PS3Extras/Ps3Types.h"
#include "../../Minecraft.Client/Platform/PS3/PS3Extras/Ps3Stubs.h"
#include "../../Minecraft.Client/Platform/PS3/PS3Extras/PS3Maths.h"

#elif defined __ORBIS__
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "../../Minecraft.Client/Platform/Orbis/OrbisExtras/OrbisTypes.h"
#include "../../Minecraft.Client/Platform/Orbis/OrbisExtras/OrbisStubs.h"
#include "../../Minecraft.Client/Platform/Orbis/OrbisExtras/OrbisMaths.h"
#elif defined __PSVITA__
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <touch.h>
#include "../../Minecraft.Client/Platform/PSVita/PSVitaExtras/PSVitaTypes.h"
#include "../../Minecraft.Client/Platform/PSVita/PSVitaExtras/PSVitaStubs.h"
#include "../../Minecraft.Client/Platform/PSVita/PSVitaExtras/PSVitaMaths.h"
#elif defined(__linux__)
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../../Minecraft.Client/Platform/Linux/Stubs/LinuxStubs.h"
#else
#include <unordered_map>
#include <unordered_set>
#include <sal.h>
#include <vector>
#endif //__PS3__

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

#ifndef __PS3__			// the PS3 lib assert is rubbish, and aborts the code, we define our own in PS3Types.h
#include <assert.h>
#endif

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

#ifdef _XBOX
#include "../Minecraft.Client/xbox/4JLibs/inc/4J_Profile.h"
#include "../Minecraft.Client/xbox/4JLibs/inc/4J_Render.h"
#include "../Minecraft.Client/xbox/4JLibs/inc/4J_XTMS.h"
#include "../Minecraft.Client/xbox/4JLibs/inc/4J_Storage.h"
#include "../Minecraft.Client/xbox/4JLibs/inc/4J_Input.h"
#elif defined (__PS3__)
#include "../../Minecraft.Client/Platform/PS3/4JLibs/inc/4J_Profile.h"
#include "../../Minecraft.Client/Platform/PS3/4JLibs/inc/4J_Render.h"
#include "../../Minecraft.Client/Platform/PS3/4JLibs/inc/4J_Storage.h"
#include "../../Minecraft.Client/Platform/PS3/4JLibs/inc/4J_Input.h"
#elif defined _DURANGO
#include "../../Minecraft.Client/Platform/Durango/4JLibs/inc/4J_Profile.h"
#include "../../Minecraft.Client/Platform/Durango/4JLibs/inc/4J_Render.h"
#include "../../Minecraft.Client/Platform/Durango/4JLibs/inc/4J_Storage.h"
#include "../../Minecraft.Client/Platform/Durango/4JLibs/inc/4J_Input.h"
#elif defined _WINDOWS64
#include "../../Minecraft.Client/Platform/Windows64/4JLibs/inc/4J_Profile.h"
#include "../../Minecraft.Client/Platform/Windows64/4JLibs/inc/4J_Render.h"
#include "../../Minecraft.Client/Platform/Windows64/4JLibs/inc/4J_Storage.h"
#include "../../Minecraft.Client/Platform/Windows64/4JLibs/inc/4J_Input.h"
#elif defined __PSVITA__
#include "../../Minecraft.Client/Platform/PSVita/4JLibs/inc/4J_Profile.h"
#include "../../Minecraft.Client/Platform/PSVita/4JLibs/inc/4J_Render.h"
#include "../../Minecraft.Client/Platform/PSVita/4JLibs/inc/4J_Storage.h"
#include "../../Minecraft.Client/Platform/PSVita/4JLibs/inc/4J_Input.h"
#elif defined __linux__
#include "../../4J.Profile/4J_Profile.h"
#include "../../4J.Render/4J_Render.h"
#include "../../4J.Storage/4J_Storage.h"
#include "../../4J.Input/4J_Input.h"
#else
#include "../../Minecraft.Client/Platform/Orbis/4JLibs/inc/4J_Profile.h"
#include "../../Minecraft.Client/Platform/Orbis/4JLibs/inc/4J_Render.h"
#include "../../Minecraft.Client/Platform/Orbis/4JLibs/inc/4J_Storage.h"
#include "../../Minecraft.Client/Platform/Orbis/4JLibs/inc/4J_Input.h"
#endif

#include "../../Common/Network/GameNetworkManager.h"

// #ifdef _XBOX
#include "../../Common/UI/UIEnums.h"
#include "../../Common/App_Defines.h"
#include "../../Common/App_enums.h"
#include "../../Common/Tutorial/TutorialEnum.h"
#include "../../Common/App_structs.h"
//#endif

#ifdef _XBOX
#include "../../Common/XUI/XUI_Helper.h"
#include "../../Common/XUI/XUI_Scene_Base.h"
#endif
#include "../../Common/Consoles_App.h"
#include "../../Common/Minecraft_Macros.h"
#include "../../Common/Colours/ColourTable.h"

#include "../../Common/BuildVer.h"

// This is generated at build time via scripts/pack_loc.py
#include "strings.h"

// This is generated at build time via scripts/pack_loc.py
#include "strings.h"

#ifdef _XBOX
#include "../../Minecraft.Client/Platform/Xbox/Xbox_App.h"
#include "../../Minecraft.Client/Platform/Xbox/Sentient/SentientTelemetryCommon.h"
#include "../../Minecraft.Client/Platform/Xbox/Sentient/MinecraftTelemetry.h"

#elif defined (__PS3__)
#include "../../Minecraft.Client/Platform/PS3/PS3_App.h"
#include "../../Minecraft.Client/Platform/PS3/Sentient/SentientTelemetryCommon.h"
#include "../../Minecraft.Client/Platform/PS3/Sentient/MinecraftTelemetry.h"

#elif defined _DURANGO
#include "../../Minecraft.Client/Platform/Durango/Durango_App.h"
//#include "../../Minecraft.Client/Platform/Durango/Sentient/SentientManager.h"
#include "../../Minecraft.Client/Platform/Durango/Sentient/SentientTelemetryCommon.h"
#include "../../Minecraft.Client/Platform/Durango/Sentient/MinecraftTelemetry.h"
#include "../../Minecraft.Client/Platform/Durango/Sentient/TelemetryEnum.h"

#elif defined _WINDOWS64
#include "../../Minecraft.Client/Platform/Windows64/Windows64_App.h"
#include "../../Minecraft.Client/Platform/Windows64/Sentient/SentientTelemetryCommon.h"
#include "../../Minecraft.Client/Platform/Windows64/Sentient/MinecraftTelemetry.h"

#elif defined __PSVITA__
#include "../../Minecraft.Client/Platform/PSVita/PSVita_App.h"
#include "../../Minecraft.Client/Platform/PSVita/Sentient/SentientManager.h"
#include "../../Minecraft.Client/Platform/PSVita/Sentient/MinecraftTelemetry.h"
#elif defined(__linux__)
	// Use Orbis-compatible headers on Linux (same as Minecraft.Client/Platform/stdafx.h).
	// All Orbis Sentient headers have #pragma once, preventing double-inclusion
	// when DLC/other Common files also pull in Minecraft.Client stdafx.h.
	#include "../../Minecraft.Client/Platform/Linux/Linux_App.h"
	#include "../../Minecraft.Client/Platform/Orbis/Sentient/SentientTelemetryCommon.h"
	#include "../../Minecraft.Client/Platform/Orbis/Sentient/DynamicConfigurations.h"
	#include "../../Minecraft.Client/Platform/Orbis/GameConfig/Minecraft.spa.h"
// #include "../../Minecraft.Client/Platform/Windows64/Sentient/MinecraftTelemetry.h"
#else
#include "../../Minecraft.Client/Platform/Orbis/Orbis_App.h"
#include "../../Minecraft.Client/Platform/Orbis/Sentient/SentientTelemetryCommon.h"
#include "../../Minecraft.Client/Platform/Orbis/Sentient/MinecraftTelemetry.h"
#endif

#include "../../Common/DLC/DLCSkinFile.h"
#include "../../Common/Console_Awards_enum.h"
#include "../../Common/Potion_Macros.h"
#include "../../Common/Console_Debug_enum.h"
#include "../../Common/GameRules/ConsoleGameRulesConstants.h"
#include "../../Common/GameRules/ConsoleGameRules.h"
#include "../../Common/Telemetry/TelemetryManager.h"
