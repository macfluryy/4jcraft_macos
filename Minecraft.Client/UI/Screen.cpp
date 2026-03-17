#include "../Platform/stdafx.h"
#include "Screen.h"
#include "Button.h"
#include "../Rendering/Particles/GuiParticles.h"
#include "../Rendering/Tesselator.h"
#include "../Textures/Textures.h"
#include "../../Minecraft.World/Util/SoundTypes.h"

Screen::Screen()  // 4J added
{
    minecraft = NULL;
    width = 0;
    height = 0;
    passEvents = false;
    font = NULL;
    particles = NULL;
    clickedButton = NULL;
}

void Screen::render(int xm, int ym, float a) {
    AUTO_VAR(itEnd, buttons.end());
    for (AUTO_VAR(it, buttons.begin()); it != itEnd; it++) {
        Button* button = *it;  // buttons[i];
        button->render(minecraft, xm, ym);
    }
}

void Screen::keyPressed(wchar_t eventCharacter, int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->setScreen(NULL);
        //    minecraft->grabMouse();	// 4J - removed
    }
}

std::wstring Screen::getClipboard() {
    // 4J - removed
    return std::wstring();
}

void Screen::setClipboard(const std::wstring& str) {
    // 4J - removed
}

void Screen::mouseClicked(int x, int y, int buttonNum) {
    if (buttonNum == 0) {
        AUTO_VAR(itEnd, buttons.end());
        for (AUTO_VAR(it, buttons.begin()); it != itEnd; it++) {
            Button* button = *it;  // buttons[i];
            if (button->clicked(minecraft, x, y)) {
                clickedButton = button;
                minecraft->soundEngine->playUI(eSoundType_RANDOM_CLICK, 1, 1);
                buttonClicked(button);
            }
        }
    }
}

void Screen::mouseReleased(int x, int y, int buttonNum) {
    if (clickedButton != NULL && buttonNum == 0) {
        clickedButton->released(x, y);
        clickedButton = NULL;
    }
}

void Screen::buttonClicked(Button* button) {}

void Screen::init(Minecraft* minecraft, int width, int height) {
    particles = new GuiParticles(minecraft);
    this->minecraft = minecraft;
    this->font = minecraft->font;
    this->width = width;
    this->height = height;
    buttons.clear();
    init();
}

void Screen::setSize(int width, int height) {
    this->width = width;
    this->height = height;
}

void Screen::init() {}

void Screen::updateEvents() {
// TODO: update for SDL if we ever get around to that
#if (defined(ENABLE_JAVA_GUIS))
    int fbw, fbh;
    RenderManager.GetFramebufferSize(fbw, fbh);
    glViewport(0, 0, fbw, fbh);
    ScreenSizeCalculator ssc(minecraft->options, minecraft->width,
                             minecraft->height);
    int screenWidth = ssc.getWidth();
    int screenHeight = ssc.getHeight();
    int xMouse = InputManager.GetMouseX() * screenWidth / fbw;
    int yMouse = InputManager.GetMouseY() * screenHeight / fbh - 1;

    static bool prevLeftState = false;
    static bool prevRightState = false;

    bool leftState = InputManager.ButtonDown(0, MINECRAFT_ACTION_ACTION);
    bool rightState = InputManager.ButtonDown(0, MINECRAFT_ACTION_USE);

    if (leftState && !prevLeftState) {
        mouseClicked(xMouse, yMouse, 0);
    } else if (!leftState && prevLeftState) {
        mouseReleased(xMouse, yMouse, 0);
    }

    if (rightState && !prevRightState) {
        mouseClicked(xMouse, yMouse, 1);
    } else if (!rightState && prevRightState) {
        mouseReleased(xMouse, yMouse, 1);
    }

    prevLeftState = leftState;
    prevRightState = rightState;
#else
    /* 4J - TODO
while (Mouse.next()) {
    mouseEvent();
}

while (Keyboard.next()) {
    keyboardEvent();
}
    */
#endif
}

void Screen::mouseEvent() {
    /* 4J - TODO
if (Mouse.getEventButtonState()) {
    int xm = Mouse.getEventX() * width / minecraft.width;
    int ym = height - Mouse.getEventY() * height / minecraft.height - 1;
    mouseClicked(xm, ym, Mouse.getEventButton());
} else {
    int xm = Mouse.getEventX() * width / minecraft.width;
    int ym = height - Mouse.getEventY() * height / minecraft.height - 1;
    mouseReleased(xm, ym, Mouse.getEventButton());
}
    */
}

void Screen::keyboardEvent() {
    /* 4J - TODO
if (Keyboard.getEventKeyState()) {
    if (Keyboard.getEventKey() == Keyboard.KEY_F11) {
        minecraft.toggleFullScreen();
        return;
    }
    keyPressed(Keyboard.getEventCharacter(), Keyboard.getEventKey());
}
    */
}

void Screen::tick() {}

void Screen::removed() {}

void Screen::renderBackground() { renderBackground(0); }

void Screen::renderBackground(int vo) {
    if (minecraft->level != NULL) {
        fillGradient(0, 0, width, height, 0xc0101010, 0xd0101010);
    } else {
        renderDirtBackground(vo);
    }
}

void Screen::renderDirtBackground(int vo) {
    // 4J Unused - Iggy Flash UI renders the background on consoles
#ifdef ENABLE_JAVA_GUIS
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    Tesselator* t = Tesselator::getInstance();
    glBindTexture(GL_TEXTURE_2D,
                  minecraft->textures->loadTexture(TN_GUI_BACKGROUND));
    glColor4f(1, 1, 1, 1);
    float s = 32;
    t->begin();
    t->color(0x404040);
    t->vertexUV(static_cast<float>(0), static_cast<float>(height),
                static_cast<float>(0), static_cast<float>(0),
                static_cast<float>(height / s + vo));
    t->vertexUV(static_cast<float>(width), static_cast<float>(height),
                static_cast<float>(0), static_cast<float>(width / s),
                static_cast<float>(height / s + vo));
    t->vertexUV(static_cast<float>(width), static_cast<float>(0),
                static_cast<float>(0), static_cast<float>(width / s),
                static_cast<float>(0 + vo));
    t->vertexUV(static_cast<float>(0), static_cast<float>(0),
                static_cast<float>(0), static_cast<float>(0),
                static_cast<float>(0 + vo));
    t->end();
#endif
}

bool Screen::isPauseScreen() { return true; }

void Screen::confirmResult(bool result, int id) {}

void Screen::tabPressed() {}
