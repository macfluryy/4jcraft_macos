#pragma once
#include <string>
#include <vector>

#include "GuiComponent.h"

class Button;
class GuiParticles;
class Minecraft;
class Font;

class Screen : public GuiComponent {
protected:
    Minecraft* minecraft;

public:
    int width;
    int height;

protected:
    std::vector<Button*> buttons;

public:
    bool passEvents;

protected:
    Font* font;

public:
    GuiParticles* particles;
    
    // 4J - FIX: Deferred UI rebuild to prevent use-after-free crashes
    bool needsUIRebuild;
    int pendingRebuildWidth;
    int pendingRebuildHeight;
    
    // 4J - SMOOTH GUI SCALE CHANGE: Fade animation to prevent flicker
    bool isRebuildingUI;              // Flag during rebuild phase (no render/input)
    float uiFadeAlpha;                 // 1.0 = fully visible, 0.0 = invisible (during rebuild)
    int rebuildFrameCounter;           // Frame counter for fade-out/in timing
    static constexpr int FADE_OUT_FRAMES = 2;  // Frames to fade out before rebuild
    static constexpr int FADE_IN_FRAMES = 3;   // Frames to fade in after rebuild

    Screen();  // 4J added
    virtual void render(int xm, int ym, float a);

public:
    virtual void keyPressed(wchar_t eventCharacter, int eventKey);
    static std::wstring getClipboard();
    static void setClipboard(const std::wstring& str);

protected:
    Button* clickedButton;

    virtual void mouseClicked(int x, int y, int buttonNum);
    virtual void mouseReleased(int x, int y, int buttonNum);
    virtual void buttonClicked(Button* button);

public:
    virtual void init(Minecraft* minecraft, int width, int height);
    virtual void setSize(int width, int height);
    virtual void init();
    virtual void updateEvents();
    virtual void mouseEvent();
    virtual void keyboardEvent();
    virtual void tick();
    virtual void removed();
    virtual void renderBackground();
    virtual void renderBackground(int vo);
    virtual void renderDirtBackground(int vo);
    virtual bool isPauseScreen();
    virtual void confirmResult(bool result, int id);
    virtual void tabPressed();
    
    // 4J - FIX: Safe deferred UI rebuild handler (called from Minecraft::tick)
    void _performDeferredUIRebuild();
};
