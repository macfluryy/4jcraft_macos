// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

//#include <xtl.h>
//#include <xboxmath.h>

#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "("__STR1__(__LINE__)") : 4J Warning Msg: "

// use  - #pragma message(__LOC__"Need to do something here")

// #ifndef _XBOX
// #ifdef _CONTENT_PACKAGE
// #define TO_BE_IMPLEMENTED
// #endif
// #endif

#ifdef __PS3__

#include "../Platform/PS3/PS3Extras/Ps3Types.h"
#include "../Platform/PS3/PS3Extras/Ps3Stubs.h"
#include "../Platform/PS3/PS3Extras/PS3Maths.h"

#elif defined __ORBIS__
#define AUTO_VAR(_var, _val) auto _var = _val
#include <stdio.h>
#include <stdlib.h>
#include <scebase.h>
#include <kernel.h>
#include <unordered_map>
#include <unordered_set>
#include <vector> 
#include <fios2.h>
#include <message_dialog.h>
#include <game_live_streaming.h>
#include "../Platform/Orbis/OrbisExtras/OrbisTypes.h"
#include "../Platform/Orbis/OrbisExtras/OrbisStubs.h"
#include "../Platform/Orbis/OrbisExtras/OrbisMaths.h"
#define HRESULT_SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#elif defined __PSVITA__
#include <stdio.h>
#include <stdlib.h>
#include <scebase.h>
#include <kernel.h>
#include <unordered_map>
#include <unordered_set>
#include <vector> 
#include <touch.h>
#include "../Platform/PSVita/PSVitaExtras/PSVitaTypes.h"
#include "../Platform/PSVita/PSVitaExtras/PSVitaStubs.h"
#include "../Platform/PSVita/PSVitaExtras/PSVitaMaths.h"
#elif defined __linux__
#define AUTO_VAR(_var, _val) auto _var = _val
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../Platform/Linux/Stubs/LinuxStubs.h"
#else
#define AUTO_VAR(_var, _val) auto _var = _val
#include <unordered_map>
#include <unordered_set>
#include <vector>
typedef unsigned __int64 __uint64;
#endif

#ifdef  _WINDOWS64
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <malloc.h>
#include <tchar.h>
// TODO: reference additional headers your program requires here
#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX; 

#define HRESULT_SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)

#endif



#ifdef _DURANGO
#include <xdk.h>
#include <wrl.h>
#include <d3d11_x.h>
#include <DirectXMath.h>
#include <ppltasks.h>
#include <collection.h>
using namespace DirectX; 
#include <pix.h>
#include "../Platform/Durango/DurangoExtras/DurangoStubs.h"
#define HRESULT_SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#endif



#ifdef _XBOX
#include <xtl.h>
#include <xboxmath.h>
#include <xonline.h>
#include <xuiapp.h>
#include <xact3.h>
#include <xact3d3.h>
typedef XINVITE_INFO INVITE_INFO;
typedef XUID PlayerUID;
typedef XNKID SessionID;
typedef XUID GameSessionUID;
#define HRESULT_SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)

#include "../Minecraft.Client/xbox/network/extra.h"
#else
#include "../../Minecraft.World/Build/x64headers/extraX64.h"
#endif

#ifdef __PS3__
#include <cell/rtc.h>
#include <cell/l10n.h>
#include <cell/pad.h>
#include <cell/cell_fs.h>
#include <sys/process.h>
#include <sys/ppu_thread.h>
#include <cell/sysmodule.h>
#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_savedata.h>
#include <sysutil/sysutil_sysparam.h>

#endif

// C RunTime Header Files
#include <stdlib.h>

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

#ifdef _XBOX
#include <xonline.h>
#include <xparty.h>
#endif

#include "../../Minecraft.World/Util/Definitions.h"
#include "../../Minecraft.World/Util/Class.h"
#include "../../Minecraft.World/Util/ArrayWithLength.h"
#include "../../Minecraft.World/Util/SharedConstants.h"
#include "../../Minecraft.World/Util/Random.h"
#include "../../Minecraft.World/IO/Streams/Compression.h"
#include "../../Minecraft.World/Util/PerformanceTimer.h"

#ifdef _XBOX
	#include "xbox/4JLibs/inc/4J_Input.h"
	#include "xbox/4JLibs/inc/4J_Profile.h"
	#include "xbox/4JLibs/inc/4J_Render.h"
	#include "xbox/4JLibs/inc/4J_XTMS.h"
	#include "xbox/4JLibs/inc/4J_Storage.h"
#elif defined (__PS3__)

	#include "../Platform/PS3/4JLibs/inc/4J_Input.h"
	#include "../Platform/PS3/4JLibs/inc/4J_Profile.h"
	#include "../Platform/PS3/4JLibs/inc/4J_Render.h"
	#include "../Platform/PS3/4JLibs/inc/4J_Storage.h"
#elif defined _DURANGO
	#include "../Platform/Durango/4JLibs/inc/4J_Input.h"
	#include "../Platform/Durango/4JLibs/inc/4J_Profile.h"
	#include "../Platform/Durango/4JLibs/inc/4J_Render.h"
	#include "../Platform/Durango/4JLibs/inc/4J_Storage.h"
#elif defined _WINDOWS64
	#include "../Platform/Windows64/4JLibs/inc/4J_Input.h"	
	#include "../Platform/Windows64/4JLibs/inc/4J_Profile.h"
	#include "../Platform/Windows64/4JLibs/inc/4J_Render.h"
	#include "../Platform/Windows64/4JLibs/inc/4J_Storage.h"
#elif defined __linux__
	#include "../../4J.Input/4J_Input.h"
	#include "../../4J.Profile/4J_Profile.h"
	#include "../../4J.Render/4J_Render.h"
	#include "../../4J.Storage/4J_Storage.h"
#elif defined __PSVITA__
	#include "../Platform/PSVita/4JLibs/inc/4J_Input.h"	
	#include "../Platform/PSVita/4JLibs/inc/4J_Profile.h"
	#include "../Platform/PSVita/4JLibs/inc/4J_Render.h"
	#include "../Platform/PSVita/4JLibs/inc/4J_Storage.h"
#else
	#include "../Platform/Orbis/4JLibs/inc/4J_Input.h"	
	#include "../Platform/Orbis/4JLibs/inc/4J_Profile.h"
	#include "../Platform/Orbis/4JLibs/inc/4J_Render.h"
	#include "../Platform/Orbis/4JLibs/inc/4J_Storage.h"
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

#include "Common/Network/GameNetworkManager.h"

#ifdef _XBOX
#include "Common/XUI/XUI_Helper.h"
#include "Common/XUI/XUI_Scene_Base.h"
#endif

#include "Common/UI/UIEnums.h"
#include "Common/UI/UIStructs.h"
// #ifdef _XBOX
#include "Common/App_Defines.h"
#include "Common/App_enums.h"
#include "Common/Tutorial/TutorialEnum.h"
#include "Common/App_structs.h"
//#endif

#include "Common/Consoles_App.h"
#include "Common/Minecraft_Macros.h"
#include "Common/BuildVer.h"

#ifdef _XBOX
	#include "../Platform/Xbox/Xbox_App.h"
	#include "../Platform/Xbox/Sentient/MinecraftTelemetry.h"
	#include "../Platform/Xbox/Sentient/DynamicConfigurations.h"
	#include "XboxMedia/strings.h"
	#include "../Platform/Xbox/Sentient/SentientTelemetryCommon.h"
	#include "../Platform/Xbox/Sentient/Include/SenClientStats.h"
	#include "../Platform/Xbox/GameConfig/Minecraft.spa.h"
	#include "XboxMedia/4J_strings.h"
	#include "../Platform/Xbox/XML/ATGXmlParser.h"
	#include "../Platform/Xbox/Leaderboards/XboxLeaderboardManager.h"
	#include "../Platform/Xbox/Social/SocialManager.h"
	#include "../Platform/Xbox/Audio/SoundEngine.h"
	#include "../Platform/Xbox/Xbox_UIController.h"

#elif defined (__PS3__)
	#include "extraX64client.h"
	#include "../Platform/PS3/Sentient/MinecraftTelemetry.h"
	#include "../Platform/PS3/Sentient/DynamicConfigurations.h"
	#include "../Platform/PS3/Sentient/SentientTelemetryCommon.h"
	#include "../Platform/PS3Media/strings.h"
	#include "../Platform/PS3/PS3_App.h"
	#include "../Platform/PS3/GameConfig/Minecraft.spa.h"
	#include "../Platform/PS3Media/4J_strings.h"
	#include "../Platform/PS3/XML/ATGXmlParser.h"
	#include "../Platform/PS3/Social/SocialManager.h"
	#include "Common/Audio/SoundEngine.h"
	#include "../Platform/PS3/Iggy/include/iggy.h"
	#include "../Platform/PS3/Iggy/gdraw/gdraw_ps3gcm.h"
	#include "../Platform/PS3/PS3_UIController.h"
#elif defined _DURANGO
	#include "../Platform/Durango/Sentient/MinecraftTelemetry.h"
	#include "../Platform/DurangoMedia/strings.h"
	#include "../Platform/Durango/Durango_App.h"
	#include "../Platform/Durango/Sentient/DynamicConfigurations.h"	
	#include "../Platform/Durango/Sentient/TelemetryEnum.h"
	#include "../Platform/Durango/Sentient/SentientTelemetryCommon.h"
	#include "../Platform/Durango/PresenceIds.h"
	#include "../Platform/DurangoMedia/4J_strings.h"
	#include "../Platform/Durango/XML/ATGXmlParser.h"
	#include "../Platform/Durango/Social/SocialManager.h"
#include "Common/Audio/SoundEngine.h"
	#include "../Platform/Durango/Iggy/include/iggy.h"
	#include "../Platform/Durango/Iggy/gdraw/gdraw_d3d11.h"
	#include "../Platform/Durango/Durango_UIController.h"
#elif defined _WINDOWS64
	#include "../Platform/Windows64/Sentient/MinecraftTelemetry.h"
	#include "../Platform/Windows64Media/strings.h"
	#include "../Platform/Windows64/Windows64_App.h"
	#include "../Platform/Windows64/Sentient/DynamicConfigurations.h"
	#include "../Platform/Windows64/Sentient/SentientTelemetryCommon.h"
	#include "../Platform/Windows64/GameConfig/Minecraft.spa.h"
	#include "../Platform/Windows64/XML/ATGXmlParser.h"	
	#include "../Platform/Windows64/Social/SocialManager.h"
	#include "Common/Audio/SoundEngine.h"
	#include "../Platform/Windows64/Iggy/include/iggy.h"
	#include "../Platform/Windows64/Iggy/gdraw/gdraw_d3d11.h"
	#include "../Platform/Windows64/Windows64_UIController.h"
#elif defined __linux__
	// Linux build: avoid pulling in Windows64 platform headers (they cause
	// symbol/class redefinitions). Use Linux-specific stubs and controller.
	// FIXME: Produce proper Linux equivalents for telemetry/sentient/etc.
	#include "../Platform/OrbisMedia/strings.h" // temporary strings
	#include "../Platform/Xbox/Sentient/SentientTelemetryCommon.h"
	#include "../Platform/Xbox/Sentient/DynamicConfigurations.h"
	#include "../Platform/Xbox/GameConfig/Minecraft.spa.h"
	#include "Common/Audio/SoundEngine.h"
	#include "../Platform/Linux/Linux_UIController.h"
#elif defined __PSVITA__
	#include "../Platform/PSVita/PSVita_App.h"
	#include "../Platform/PSVitaMedia/strings.h"		// TODO - create PSVita-specific version of this
	#include "../Platform/PSVita/Sentient/SentientManager.h"
	#include "../Platform/PSVita/Sentient/MinecraftTelemetry.h"
	#include "../Platform/PSVita/Sentient/DynamicConfigurations.h"
	#include "../Platform/PSVita/GameConfig/Minecraft.spa.h"
	#include "../Platform/PSVita/XML/ATGXmlParser.h"
	#include "../Platform/PSVita/Social/SocialManager.h"
	#include "Common/Audio/SoundEngine.h"
	#include "../Platform/PSVita/Iggy/include/iggy.h"
	#include "../Platform/PSVita/Iggy/gdraw/gdraw_psp2.h"
	#include "../Platform/PSVita/PSVita_UIController.h"
#else
	#include "../Platform/Orbis/Sentient/MinecraftTelemetry.h"
	#include "../Platform/OrbisMedia/strings.h"
	#include "../Platform/Orbis/Orbis_App.h"
	#include "../Platform/Orbis/Sentient/SentientTelemetryCommon.h"
	#include "../Platform/Orbis/Sentient/DynamicConfigurations.h"
	#include "../Platform/Orbis/GameConfig/Minecraft.spa.h"
	#include "../Platform/OrbisMedia/4J_strings.h"
	#include "../Platform/Orbis/XML/ATGXmlParser.h"	
	#include "../Platform/Windows64/Social/SocialManager.h"
	#include "Common/Audio/SoundEngine.h"
	#include "../Platform/Orbis/Iggy/include/iggy.h"
	#include "../Platform/Orbis/Iggy/gdraw/gdraw_orbis.h"
	#include "../Platform/Orbis/Orbis_UIController.h"
#endif

#ifdef _XBOX
#include "Common/XUI/XUI_CustomMessages.h"
#include "Common/XUI/XUI_Scene_Inventory_Creative.h"
#include "Common/XUI/XUI_FullscreenProgress.h"
#include "Common/XUI/XUI_ConnectingProgress.h"
#include "Common/XUI/XUI_Scene_CraftingPanel.h"
#include "Common/XUI/XUI_TutorialPopup.h"
#include "Common/XUI/XUI_PauseMenu.h"
#endif
#include "Common/ConsoleGameMode.h"
#include "Common/Console_Debug_enum.h"
#include "Common/Console_Awards_enum.h"
#include "Common/Tutorial/TutorialMode.h"
#include "Common/Tutorial/Tutorial.h"
#include "Common/Tutorial/FullTutorialMode.h"
#include "Common/Trial/TrialMode.h"
#include "Common/GameRules/ConsoleGameRules.h"
#include "Common/GameRules/ConsoleSchematicFile.h"
#include "Common/Colours/ColourTable.h"
#include "Common/DLC/DLCSkinFile.h"
#include "Common/DLC/DLCManager.h"
#include "Common/DLC/DLCPack.h"
#include "Common/Telemetry/TelemetryManager.h"

#ifdef _XBOX
//#include "../Platform/Xbox/Xbox_App.h"
#elif !defined(__PS3__)
#include "extraX64client.h"
#endif



#ifdef _FINAL_BUILD
#define printf BREAKTHECOMPILE
#define wprintf BREAKTHECOMPILE
#undef OutputDebugString
#define OutputDebugString BREAKTHECOMPILE
#define OutputDebugStringA BREAKTHECOMPILE
#define OutputDebugStringW BREAKTHECOMPILE
#endif

void MemSect(int sect);
