#pragma once
#include "../Screen.h"

class NameEntryScreen : public Screen {
private:
    Screen* lastScreen;

protected:
    std::wstring title;

private:
    int slot;
    std::wstring name;
    int frame;

public:
    NameEntryScreen(Screen* lastScreen, const std::wstring& oldName, int slot);
    virtual void init() override;
    virtual void removed() override;
    virtual void tick() override;

protected:
    virtual void buttonClicked(Button button);

private:
    static const std::wstring allowedChars;

protected:
    virtual void keyPressed(wchar_t ch, int eventKey) override;

public:
    virtual void render(int xm, int ym, float a) override;
};