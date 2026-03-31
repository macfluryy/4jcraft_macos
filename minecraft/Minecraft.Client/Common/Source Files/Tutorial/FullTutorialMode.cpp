#include "Minecraft.Client/net/minecraft/client/Minecraft.h"
#include "FullTutorial.h"
#include "FullTutorialMode.h"
#include "Minecraft.Client/Common/Source Files/Tutorial/FullTutorialMode.h"
#include "Minecraft.Client/Common/Source Files/Tutorial/Tutorial.h"
#include "Minecraft.Client/Common/Source Files/Tutorial/TutorialMode.h"

class ClientConnection;

FullTutorialMode::FullTutorialMode(int iPad, Minecraft* minecraft,
                                   ClientConnection* connection)
    : TutorialMode(iPad, minecraft, connection) {
    tutorial = new FullTutorial(iPad);
    minecraft->playerStartedTutorial(iPad);
}

bool FullTutorialMode::isTutorial() {
    return !tutorial->m_fullTutorialComplete;
}