#include "ConsoleGameMode.h"
#include "Minecraft.Client/Common/src/Tutorial/Tutorial.h"
#include "Minecraft.Client/Common/src/ConsoleGameMode.h"
#include "Minecraft.Client/Common/src/Tutorial/TutorialMode.h"

class ClientConnection;
class Minecraft;

ConsoleGameMode::ConsoleGameMode(int iPad, Minecraft* minecraft,
                                 ClientConnection* connection)
    : TutorialMode(iPad, minecraft, connection) {
    tutorial = new Tutorial(iPad);
}