#include "../../../../Minecraft.World/Header Files/stdafx.h"
#include "TutorialMessage.h"

TutorialMessage::TutorialMessage(
    int messageId, bool limitRepeats /*= false*/,
    unsigned char numRepeats /*= TUTORIAL_MESSAGE_DEFAULT_SHOW*/)
    : messageId(messageId),
      limitRepeats(limitRepeats),
      numRepeats(numRepeats),
      timesShown(0) {}

bool TutorialMessage::canDisplay() {
    return !limitRepeats || (timesShown < numRepeats);
}

const wchar_t* TutorialMessage::getMessageForDisplay() {
    if (!canDisplay()) return L"";

    if (limitRepeats) ++timesShown;

    return app.GetString(messageId);
}
