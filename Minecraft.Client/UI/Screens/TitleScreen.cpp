#include "../../Platform/stdafx.h"
#include "../Button.h"
#include "OptionsScreen.h"
#include "SelectWorldScreen.h"
#include "JoinMultiplayerScreen.h"
#include "../../Rendering/Tesselator.h"
#include "../../Textures/Textures.h"
#include "../../../Minecraft.World/Util/StringHelpers.h"
#include "../../../Minecraft.World/IO/Streams/InputOutputStream.h"
#include "../../../Minecraft.World/Headers/net.minecraft.locale.h"
#include "../../../Minecraft.World/Platform/System.h"
#include "../../../Minecraft.World/Util/Random.h"
#include <GL/gl.h>
#include "TitleScreen.h"

Random* TitleScreen::random = new Random();

TitleScreen::TitleScreen() {
    // 4J - added initialisers
    vo = 0;
    multiplayerButton = NULL;

    splash = L"missingno";
    //    try {	// 4J - removed try/catch
    std::vector<std::wstring> splashes;

    // 4jcraft: copied over from UIScene_MainMenu
    int splashIndex;

    std::wstring filename = L"splashes.txt";
    if (app.hasArchiveFile(filename)) {
        byteArray splashesArray = app.getArchiveFile(filename);
        ByteArrayInputStream bais(splashesArray);
        InputStreamReader isr(&bais);
        BufferedReader br(&isr);

        std::wstring line = L"";
        while (!(line = br.readLine()).empty()) {
            line = trimString(line);
            if (line.length() > 0) {
                splashes.push_back(line);
            }
        }

        br.close();
    }

    splashIndex =
        eSplashRandomStart + 1 +
        random->nextInt((int)splashes.size() - (eSplashRandomStart + 1));

    // Override splash text on certain dates
    SYSTEMTIME LocalSysTime;
    GetLocalTime(&LocalSysTime);
    if (LocalSysTime.wMonth == 11 && LocalSysTime.wDay == 9) {
        splashIndex = eSplashHappyBirthdayEx;
    } else if (LocalSysTime.wMonth == 6 && LocalSysTime.wDay == 1) {
        splashIndex = eSplashHappyBirthdayNotch;
    } else if (LocalSysTime.wMonth == 12 &&
               LocalSysTime.wDay == 24)  // the Java game shows this on
                                         // Christmas Eve, so we will too
    {
        splashIndex = eSplashMerryXmas;
    } else if (LocalSysTime.wMonth == 1 && LocalSysTime.wDay == 1) {
        splashIndex = eSplashHappyNewYear;
    }

    splash = splashes.at(splashIndex);
}

void TitleScreen::tick() {
    vo += 1.0f;
    // if( vo > 100.0f ) minecraft->setScreen(new SelectWorldScreen(this));
    // // 4J - temp testing
}

void TitleScreen::keyPressed(wchar_t eventCharacter, int eventKey) {}

void TitleScreen::init() {
    app.DebugPrintf("TitleScreen::init() START\n");
    /* 4J - removed
Calendar c = Calendar.getInstance();
c.setTime(new Date());

if (c.get(Calendar.MONTH) + 1 == 11 && c.get(Calendar.DAY_OF_MONTH) == 9) {
    splash = "Happy birthday, ez!";
} else if (c.get(Calendar.MONTH) + 1 == 6 && c.get(Calendar.DAY_OF_MONTH) == 1)
{ splash = "Happy birthday, Notch!"; } else if (c.get(Calendar.MONTH) + 1 == 12
&& c.get(Calendar.DAY_OF_MONTH) == 24) { splash = "Merry X-mas!"; } else if
(c.get(Calendar.MONTH) + 1 == 1 && c.get(Calendar.DAY_OF_MONTH) == 1) { splash =
"Happy new year!";
}
    */

    Language* language = Language::getInstance();

    const int spacing = 24;
    const int topPos = height / 4 + spacing * 2;

    buttons.push_back(new Button(1, width / 2 - 100, topPos,
                                 language->getElement(L"menu.singleplayer")));
    buttons.push_back(multiplayerButton = new Button(
                          2, width / 2 - 100, topPos + spacing * 1,
                          language->getElement(L"menu.multiplayer")));
    buttons.push_back(new Button(3, width / 2 - 100, topPos + spacing * 2,
                                 language->getElement(L"menu.mods")));

    if (minecraft->appletMode) {
        buttons.push_back(new Button(0, width / 2 - 100, topPos + spacing * 3,
                                     language->getElement(L"menu.options")));
    } else {
        buttons.push_back(new Button(0, width / 2 - 100,
                                     topPos + spacing * 3 + 12, 98, 20,
                                     language->getElement(L"menu.options")));
        buttons.push_back(new Button(4, width / 2 + 2,
                                     topPos + spacing * 3 + 12, 98, 20,
                                     language->getElement(L"menu.quit")));
    }

    if (minecraft->user == NULL) {
        multiplayerButton->active = false;
    }
}

void TitleScreen::buttonClicked(Button* button) {
    if (button->id == 0) {
        app.DebugPrintf(
            "TitleScreen::buttonClicked() 'Options...' if (button->id == 0)\n");
        minecraft->setScreen(new OptionsScreen(this, minecraft->options));
    }
    if (button->id == 1) {
        app.DebugPrintf(
            "TitleScreen::buttonClicked() 'Singleplayer' if (button->id == "
            "1)\n");
        minecraft->setScreen(new SelectWorldScreen(this));
    }
    if (button->id == 2) {
        app.DebugPrintf(
            "TitleScreen::buttonClicked() 'Multiplayer' if (button->id == "
            "2)\n");
        minecraft->setScreen(new JoinMultiplayerScreen(this));
    }
    if (button->id == 3) {
        app.DebugPrintf(
            "TitleScreen::buttonClicked() 'Texture Pack' if (button->id == "
            "3)\n");
        //       minecraft->setScreen(new TexturePackSelectScreen(this));
        //       // 4J - TODO put back in
    }
    if (button->id == 4) {
        app.DebugPrintf(
            "TitleScreen::buttonClicked() Exit Game if (button->id == 4)\n");
        RenderManager.Close();  // minecraft->stop();
    }
}

// 4jcraft: render our panorama
// uses the TU panorama instead of JE panorama and as such a different rendering
// method
void TitleScreen::renderPanorama() {
#ifdef ENABLE_JAVA_GUIS
    Tesselator* t = Tesselator::getInstance();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, 1000, 3000);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0, 0, -2000);

    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(false);

    glBindTexture(GL_TEXTURE_2D,
                  minecraft->textures->loadTexture(TN_TITLE_BG_PANORAMA));

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    float off = vo * 0.0001f;

    float screenAspect = (float)width / (float)height;
    float texAspect = 1748.0f / 144.0f;
    float scale;
    if (screenAspect > texAspect) {
        scale = (float)width / 1748.0f;
    } else {
        scale = (float)height / 144.0f;
    }

    float texWidth = 1748.0f * scale;
    float texHeight = 144.0f * scale;
    float yOff = (height - texHeight) / 2.0f;

    float uMax = off + (texWidth / 1748.0f);

    t->begin(GL_QUADS);
    t->color(0xffffff, 255);
    t->vertexUV(0, yOff + texHeight, 0, off, 1.0f);
    t->vertexUV(texWidth, yOff + texHeight, 0, uMax, 1.0f);
    t->vertexUV(texWidth, yOff, 0, uMax, 0.0f);
    t->vertexUV(0, yOff, 0, off, 0.0f);
    t->end();

    glDepthMask(true);
    glDisable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
#endif
}

void TitleScreen::render(int xm, int ym, float a) {
    // 4J Unused - Iggy Flash UI renders the title screen on consoles
#ifdef ENABLE_JAVA_GUIS
    renderPanorama();
    Tesselator* t = Tesselator::getInstance();

    int logoWidth = 155 + 119;
    int logoX = width / 2 - logoWidth / 2;
    int logoY = 30;

    glBindTexture(GL_TEXTURE_2D,
                  minecraft->textures->loadTexture(TN_TITLE_MCLOGO));
    glColor4f(1, 1, 1, 1);
    blit(logoX + 0, logoY + 0, 0, 0, 155, 44);
    blit(logoX + 155, logoY + 0, 0, 45, 155, 44);
    t->color(0xffffff);
    glPushMatrix();
    glTranslatef((float)width / 2 + 90, 70, 0);

    glRotatef(-20, 0, 0, 1);
    float sss = 1.8f - Mth::abs(Mth::sin(System::currentTimeMillis() % 1000 /
                                         1000.0f * PI * 2) *
                                0.1f);

    sss = sss * 100 / (font->width(splash) + 8 * 4);
    glScalef(sss, sss, sss);
    drawCenteredString(font, splash, 0, -8, 0xffff00);
    glPopMatrix();

    drawString(
        font, ClientConstants::VERSION_STRING, 2, height - 10,
        0xffffff);  // 4jcraft: use the same height as the copyright message
    wstring msg = L"Copyright Mojang AB. Do not distribute.";
    drawString(font, msg, width - font->width(msg) - 2, height - 10, 0xffffff);

    Screen::render(xm, ym, a);
#endif
}
