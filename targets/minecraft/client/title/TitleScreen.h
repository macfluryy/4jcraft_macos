#pragma once
#include <string>

#include "minecraft/client/gui/Screen.h"

class Random;
class Button;

class TitleScreen : public Screen {
private:
    static Random* random;

    float vo;

    std::wstring splash;
    Button* multiplayerButton;

    
    void renderPanorama(float a);
    void renderSkybox(float a);
    void rotateAndBlur(float a);
    int viewportTexture;

    
    
    enum eSplashIndexes {
        eSplashHappyBirthdayEx = 0,
        eSplashHappyBirthdayNotch,
        eSplashMerryXmas,
        eSplashHappyNewYear,

        
        
        eSplashRandomStart,
    };

public:
    TitleScreen();
    virtual void tick() override;

protected:
    virtual void keyPressed(wchar_t eventCharacter, int eventKey) override;

public:
    virtual void init() override;

protected:
    virtual void buttonClicked(Button* button) override;

public:
    virtual void render(int xm, int ym, float a) override;
};