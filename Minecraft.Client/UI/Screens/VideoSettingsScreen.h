#pragma once
#include "../Screen.h"
class Options;


class VideoSettingsScreen : public Screen
{
private:
	Screen *lastScreen;
protected:
	std::wstring title;
private:
	Options *options;

public:
	VideoSettingsScreen(Screen *lastScreen, Options *options);
    virtual void init();
protected:
	virtual void buttonClicked(Button *button);
public:
	virtual void render(int xm, int ym, float a);
};