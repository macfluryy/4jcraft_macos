#include "Minecraft.World/Header Files/stdafx.h"
#include "../../../net/minecraft/client/Minecraft.h"
#include "FullTutorial.h"
#include "FullTutorialMode.h"

FullTutorialMode::FullTutorialMode(int iPad, Minecraft* minecraft,
                                   ClientConnection* connection)
    : TutorialMode(iPad, minecraft, connection) {
    tutorial = new FullTutorial(iPad);
    minecraft->playerStartedTutorial(iPad);
}

bool FullTutorialMode::isTutorial() {
    return !tutorial->m_fullTutorialComplete;
}