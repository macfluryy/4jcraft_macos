#pragma once
#include "Minecraft.Client/Common/Source Files/Tutorial/TutorialMode.h"

class ClientConnection;
class Minecraft;

class ConsoleGameMode : public TutorialMode {
public:
    ConsoleGameMode(int iPad, Minecraft* minecraft,
                    ClientConnection* connection);

    virtual bool isImplemented() { return true; }
};