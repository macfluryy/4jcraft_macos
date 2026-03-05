#pragma once
#include "../../Build/Common/Tutorial/TutorialMode.h"

class XboxGameMode : public TutorialMode
{
public:	
	XboxGameMode(int iPad, Minecraft *minecraft, ClientConnection *connection);

	virtual bool isImplemented() { return true; }
};