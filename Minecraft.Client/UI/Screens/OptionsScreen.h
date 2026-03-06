#pragma once
#include "../Screen.h"
class Options;


class OptionsScreen : public Screen
{
private:
	static const int CONTROLS_BUTTON_ID = 100;
    static const int VIDEO_BUTTON_ID = 101;
    Screen *lastScreen;
protected:
	std::wstring title;
private:
	Options *options;
public:
	OptionsScreen(Screen *lastScreen, Options *options);
    virtual void init();
protected:
	virtual void buttonClicked(Button *button);
public:
	virtual void render(int xm, int ym, float a);
};