#include "TrialMode.h"
#include "Minecraft.Client/Common/src/Tutorial/FullTutorial.h"
#include "Minecraft.Client/Common/src/Trial/TrialMode.h"
#include "Minecraft.Client/Common/src/Tutorial/FullTutorialMode.h"

class ClientConnection;
class Minecraft;

TrialMode::TrialMode(int iPad, Minecraft* minecraft,
                     ClientConnection* connection)
    : FullTutorialMode(iPad, minecraft, connection) {
    tutorial = new FullTutorial(iPad, true);
}