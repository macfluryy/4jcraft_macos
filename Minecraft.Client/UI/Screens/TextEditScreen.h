#pragma once
#include "../Screen.h"
class SignTileEntity;


class TextEditScreen : public Screen
{
protected:
	std::wstring title;
private:
	std::shared_ptr<SignTileEntity> sign;
    int frame;
    int line;

public:
	TextEditScreen(std::shared_ptr<SignTileEntity> sign);
    virtual void init();
    virtual void removed();
    virtual void tick();
protected:
	virtual void buttonClicked(Button *button);
private:
	static const std::wstring allowedChars;
protected:
	virtual void keyPressed(wchar_t ch, int eventKey);
public:
	virtual void render(int xm, int ym, float a);
};