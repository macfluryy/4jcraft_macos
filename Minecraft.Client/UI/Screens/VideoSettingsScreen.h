#pragma once
#include "../Screen.h"
class Options;

class VideoSettingsScreen : public Screen {
private:
    Screen* lastScreen;

protected:
    std::wstring title;

private:
    Options* options;

public:
    VideoSettingsScreen(Screen* lastScreen, Options* options);
    virtual void init() override;

protected:
    virtual void buttonClicked(Button* button) override;

public:
    virtual void render(int xm, int ym, float a) override;
};