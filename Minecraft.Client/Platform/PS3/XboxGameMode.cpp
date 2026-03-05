#include "../../../Minecraft.World/Build/stdafx.h"
#include "XboxGameMode.h"
#include "../../Build/Common/Tutorial/Tutorial.h"

XboxGameMode::XboxGameMode(int iPad, Minecraft *minecraft, ClientConnection *connection)
	: TutorialMode(iPad, minecraft, connection)
{
	tutorial = new Tutorial(iPad);
}