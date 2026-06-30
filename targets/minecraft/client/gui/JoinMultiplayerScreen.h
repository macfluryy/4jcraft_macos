#pragma once
#include <string>
#include <vector>

#include "Screen.h"

class EditBox;
class Button;

class JoinMultiplayerScreen : public Screen {
private:
    Screen* lastScreen;
    EditBox* ipEdit;

    struct LanRow {
        std::wstring label;
        std::string host;
        unsigned short port;
    };
    std::vector<LanRow> lanRows;

public:
    JoinMultiplayerScreen(Screen* lastScreen);
    virtual void tick() override;
    virtual void init() override;
    virtual void removed() override;

protected:
    virtual void buttonClicked(Button* button) override;

private:
    virtual int parseInt(const std::wstring& str, int def);

    void refreshLanRows();
    bool tryConnect(const std::string& host, int port);

protected:
    virtual void keyPressed(wchar_t ch, int eventKey) override;
    virtual void mouseClicked(int x, int y, int buttonNum) override;

public:
    virtual void render(int xm, int ym, float a) override;
};