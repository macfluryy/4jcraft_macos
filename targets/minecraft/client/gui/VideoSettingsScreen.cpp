#include "VideoSettingsScreen.h"

#include <vector>

#include "SlideButton.h"
#include "SmallButton.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/Options.h"
#include "minecraft/client/gui/Button.h"
#include "minecraft/client/gui/Screen.h"
#include "minecraft/client/gui/ScreenSizeCalculator.h"
#include "minecraft/locale/Language.h"

// 4jcraft
#define ITEM_COUNT 12

VideoSettingsScreen::VideoSettingsScreen(Screen* lastScreen, Options* options) {
    this->title = L"Video Settings";  // 4J - added
    this->lastScreen = lastScreen;
    this->options = options;
}

void VideoSettingsScreen::init() {
    Language* language = Language::getInstance();
    this->title = language->getElement(L"options.videoTitle");

    const Options::Option* items[ITEM_COUNT] = {
        Options::Option::GRAPHICS,
        Options::Option::RENDER_DISTANCE,
        Options::Option::AMBIENT_OCCLUSION,
        Options::Option::FRAMERATE_LIMIT,
        Options::Option::ANAGLYPH,
        Options::Option::VIEW_BOBBING,
        Options::Option::GUI_SCALE,
        Options::Option::ADVANCED_OPENGL,
        Options::Option::GAMMA,
        Options::Option::FOV,
        Options::Option::RENDER_CLOUDS,
        Options::Option::PARTICLES};

    for (int i = 0; i < ITEM_COUNT; i++) {
        const Options::Option* item = items[i];
        int xPos = width / 2 - 155 + (i % 2 * 160);
        int yPos = height / 6 + 24 * (i / 2);

        if (!item->isProgress()) {
            buttons.push_back(new SmallButton(item->getId(), xPos, yPos, item,
                                              options->getMessage(item)));
        } else {
            buttons.push_back(new SlideButton(item->getId(), xPos, yPos, item,
                                              options->getMessage(item),
                                              options->getProgressValue(item)));
        }
    }

    //        buttons.add(new Button(VIDEO_BUTTON_ID, width / 2 - 100, height /
    //        6 + 24 * 4 + 12, language.getElement("options.video")));
    //        buttons.add(new Button(CONTROLS_BUTTON_ID, width / 2 - 100, height
    //        / 6 + 24 * 5 + 12, language.getElement("options.controls")));
    buttons.push_back(new Button(200, width / 2 - 100, height / 6 + 24 * 6,
                                 language->getElement(L"gui.done")));
}

void VideoSettingsScreen::buttonClicked(Button* button) {
    if (!button->active) return;
    if (button->id < 100 && (dynamic_cast<SmallButton*>(button) != nullptr)) {
        const Options::Option* option = ((SmallButton*)button)->getOption();
        options->toggle(option, 1);
        button->msg = options->getMessage(Options::Option::getItem(button->id));
        
        // 4J - SMOOTH GUI SCALE CHANGE: If GUI_SCALE changed, schedule smooth rebuild with fade
        if (option == Options::Option::GUI_SCALE) {
            // Calculate new screen dimensions based on updated guiScale
            ScreenSizeCalculator ssc(minecraft->options, minecraft->width, minecraft->height);
            int newScreenWidth = ssc.getWidth();
            int newScreenHeight = ssc.getHeight();
            
            // Schedule rebuild for next frame with smooth fade animation
            this->pendingRebuildWidth = newScreenWidth;
            this->pendingRebuildHeight = newScreenHeight;
            this->needsUIRebuild = true;
            this->rebuildFrameCounter = 0;      // Start from fade-out phase
            this->uiFadeAlpha = 1.0f;            // Start fully visible
            
            // Reset clickedButton to prevent dangling pointer issues
            this->clickedButton = nullptr;
        }
        return;
    }
    if (button->id == 200) {
        minecraft->options->save();
        minecraft->setScreen(lastScreen);
        return;
    }
}

void VideoSettingsScreen::render(int xm, int ym, float a) {
    renderBackground();
    drawCenteredString(font, title, width / 2, 20, 0xffffff);

    Screen::render(xm, ym, a);
}