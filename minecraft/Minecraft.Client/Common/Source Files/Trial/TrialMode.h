#pragma once
#include "Minecraft.Client/Common/Source Files/Tutorial/FullTutorialMode.h"

class ClientConnection;
class Minecraft;

class TrialMode : public FullTutorialMode {
public:
    TrialMode(int iPad, Minecraft* minecraft, ClientConnection* connection);

    virtual bool isImplemented() { return true; }
};