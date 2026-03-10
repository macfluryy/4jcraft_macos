#pragma once

// The default number of times any message should be shown
#define TUTORIAL_MESSAGE_DEFAULT_SHOW 3

class TutorialMessage
{
private:
	int messageId;
	bool limitRepeats;
	unsigned char numRepeats;
	unsigned char timesShown;

public:
	TutorialMessage(int messageId, bool limitRepeats = false, unsigned char numRepeats = TUTORIAL_MESSAGE_DEFAULT_SHOW);

	bool canDisplay();
	const wchar_t *getMessageForDisplay();
};
