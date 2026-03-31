#include "ConsoleGameMode.h"
#include "Minecraft.Client/Common/Source Files/Tutorial/Tutorial.h"
#include "Minecraft.Client/Common/Source Files/ConsoleGameMode.h"
#include "Minecraft.Client/Common/Source Files/Tutorial/TutorialMode.h"

class ClientConnection;
class Minecraft;

ConsoleGameMode::ConsoleGameMode(int iPad, Minecraft* minecraft,
                                 ClientConnection* connection)
    : TutorialMode(iPad, minecraft, connection) {
    tutorial = new Tutorial(iPad);
}