#include "TrialMode.h"
#include "Minecraft.Client/Common/Source Files/Tutorial/FullTutorial.h"
#include "Minecraft.Client/Common/Source Files/Trial/TrialMode.h"
#include "Minecraft.Client/Common/Source Files/Tutorial/FullTutorialMode.h"

class ClientConnection;
class Minecraft;

TrialMode::TrialMode(int iPad, Minecraft* minecraft,
                     ClientConnection* connection)
    : FullTutorialMode(iPad, minecraft, connection) {
    tutorial = new FullTutorial(iPad, true);
}