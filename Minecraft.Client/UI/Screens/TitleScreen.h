#pragma once
#include "../Screen.h"
class Random;
class Button;

class TitleScreen : public Screen {
private:
    static Random* random;

    float vo;

    std::wstring splash;
    Button* multiplayerButton;

    // 4jcraft: panorama
    void renderPanorama(float a);
    void renderSkybox(float a);
    void rotateAndBlur(float a);
    int viewportTexture;
    float panoramaTimer;
    std::wstring titlePanoramaPaths[6];

    // 4jcraft: taken from UIScene_MainMenu
    // 4J Added
    enum eSplashIndexes {
        eSplashHappyBirthdayEx = 0,
        eSplashHappyBirthdayNotch,
        eSplashMerryXmas,
        eSplashHappyNewYear,

        // The start index in the splashes vector from which we can select a
        // random splash
        eSplashRandomStart,
    };

public:
    TitleScreen();
    virtual void tick();

protected:
    virtual void keyPressed(wchar_t eventCharacter, int eventKey);

public:
    virtual void init();

protected:
    virtual void buttonClicked(Button* button);

public:
    virtual void render(int xm, int ym, float a);
};