#include "AchievementScreen.h"

#include <GL/gl.h>

#include <string>
#include <vector>

#include "platform/sdl2/Render.h"
#include "app/include/stubs.h"
#include "minecraft/client/KeyMapping.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/Options.h"
#include "minecraft/client/gui/Button.h"
#include "minecraft/client/gui/Font.h"
#include "minecraft/client/gui/Screen.h"
#include "minecraft/client/gui/SmallButton.h"
#include "minecraft/locale/I18n.h"
#include "minecraft/stats/Achievement.h"
#include "minecraft/stats/Achievements.h"

AchievementScreen::AchievementScreen(StatsCounter* statsCounter) {
    
    imageWidth = 256;
    imageHeight = 202;
    xLastScroll = 0;
    yLastScroll = 0;
    scrolling = 0;

    
    
    xMin = Achievements::xMin * ACHIEVEMENT_COORD_SCALE - BIGMAP_WIDTH / 2;
    yMin = Achievements::yMin * ACHIEVEMENT_COORD_SCALE - BIGMAP_WIDTH / 2;
    xMax = Achievements::xMax * ACHIEVEMENT_COORD_SCALE - BIGMAP_HEIGHT / 2;
    yMax = Achievements::yMax * ACHIEVEMENT_COORD_SCALE - BIGMAP_HEIGHT / 2;

    this->statsCounter = statsCounter;
    int wBigMap = 141;
    int hBigMap = 141;

    xScrollO = xScrollP = xScrollTarget =
        Achievements::openInventory->x * ACHIEVEMENT_COORD_SCALE - wBigMap / 2 -
        12;
    yScrollO = yScrollP = yScrollTarget =
        Achievements::openInventory->y * ACHIEVEMENT_COORD_SCALE - hBigMap / 2;
}

void AchievementScreen::init() {
    buttons.clear();
    
    
    buttons.push_back(new SmallButton(1, width / 2 + 24, height / 2 + 74, 80,
                                      20, I18n::get(L"gui.done")));
}

void AchievementScreen::buttonClicked(Button* button) {
    if (button->id == 1) {
        minecraft->setScreen(nullptr);
        
    }
    Screen::buttonClicked(button);
}

void AchievementScreen::keyPressed(char eventCharacter, int eventKey) {
    if (eventKey == minecraft->options->keyBuild->key) {
        minecraft->setScreen(nullptr);
        
    } else {
        Screen::keyPressed(eventCharacter, eventKey);
    }
}

void AchievementScreen::render(int mouseX, int mouseY, float a) {
    if (Mouse::isButtonDown(0)) {
        int xo = (width - imageWidth) / 2;
        int yo = (height - imageHeight) / 2;

        int xBigMap = xo + 8;
        int yBigMap = yo + 17;

        if (scrolling == 0 || scrolling == 1) {
            if (mouseX >= xBigMap && mouseX < xBigMap + BIGMAP_WIDTH &&
                mouseY >= yBigMap && mouseY < yBigMap + BIGMAP_HEIGHT) {
                if (scrolling == 0) {
                    scrolling = 1;
                } else {
                    xScrollP -= mouseX - xLastScroll;
                    yScrollP -= mouseY - yLastScroll;
                    xScrollTarget = xScrollO = xScrollP;
                    yScrollTarget = yScrollO = yScrollP;
                }
                xLastScroll = mouseX;
                yLastScroll = mouseY;
            }
        }

        if (xScrollTarget < xMin) xScrollTarget = xMin;
        if (yScrollTarget < yMin) yScrollTarget = yMin;
        if (xScrollTarget >= xMax) xScrollTarget = xMax - 1;
        if (yScrollTarget >= yMax) yScrollTarget = yMax - 1;
    } else {
        scrolling = 0;
    }

    renderBackground();

    renderBg(mouseX, mouseY, a);

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    renderLabels();

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
}

void AchievementScreen::tick() {
    xScrollO = xScrollP;
    yScrollO = yScrollP;

    double xd = (xScrollTarget - xScrollP);
    double yd = (yScrollTarget - yScrollP);
    if (xd * xd + yd * yd < 4) {
        xScrollP += xd;
        yScrollP += yd;
    } else {
        xScrollP += xd * 0.85;
        yScrollP += yd * 0.85;
    }
}

void AchievementScreen::renderLabels() {
    int xo = (width - imageWidth) / 2;
    int yo = (height - imageHeight) / 2;
    font->draw(L"Achievements", xo + 15, yo + 5, 0x404040);

    
    
    
    
}

void AchievementScreen::renderBg(int xm, int ym, float a) {
    
}

bool AchievementScreen::isPauseScreen() { return true; }