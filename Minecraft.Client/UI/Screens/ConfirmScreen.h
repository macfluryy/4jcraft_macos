#pragma once
#include "../Screen.h"


class ConfirmScreen : public Screen
{
private:
	Screen *parent;
    std::wstring title1;
    std::wstring title2;
    std::wstring yesButton;
    std::wstring noButton;
    int id;

public:
	ConfirmScreen(Screen *parent, const std::wstring& title1, const std::wstring& title2, int id);
    ConfirmScreen(Screen *parent, const std::wstring& title1, const std::wstring& title2, const std::wstring& yesButton, const std::wstring& noButton, int id);
    virtual void init();
protected:
	virtual void buttonClicked(Button *button);
public:
	virtual void render(int xm, int ym, float a);
};