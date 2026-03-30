#pragma once
#include "../Screen.h"

class ErrorScreen : public Screen {
private:
    std::wstring title, message;

public:
    ErrorScreen(const std::wstring& title, const std::wstring& message);
    virtual void init() override;
    virtual void render(int xm, int ym, float a) override;

protected:
    virtual void keyPressed(wchar_t eventCharacter, int eventKey) override;
};