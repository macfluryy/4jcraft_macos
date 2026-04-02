#include "PauseScreen.h"

#include <math.h>

#include <memory>
#include <numbers>
#include <string>
#include <vector>

#include "platform/sdl2/Input.h"
#include "Button.h"
#include "MessageScreen.h"
#include "Minecraft.Client/Common/App_enums.h"
#include "Minecraft.Client/Common/src/Network/GameNetworkManager.h"
#include "Minecraft.Client/Linux/Linux_App.h"
#include "OptionsScreen.h"
#include "minecraft/client/Minecraft.h"
#include "minecraft/client/gui/Screen.h"
#include "minecraft/client/multiplayer/MultiPlayerLocalPlayer.h"
#include "minecraft/locale/I18n.h"
#include "minecraft/server/MinecraftServer.h"

PauseScreen::PauseScreen() {
    saveStep = 0;
    visibleTime = 0;
}

void PauseScreen::init() {
    saveStep = 0;
    buttons.clear();
    int yo = -16;
    // 4jcraft: solves the issue of client-side only pausing in the java gui
    if (g_NetworkManager.IsLocalGame() &&
        g_NetworkManager.GetPlayerCount() == 1)
        app.SetXuiServerAction(InputManager.GetPrimaryPad(),
                               eXuiServerAction_PauseServer, (void*)true);
    buttons.push_back(new Button(1, width / 2 - 100, height / 4 + 24 * 5 + yo,
                                 I18n::get(L"menu.returnToMenu")));
    if (!g_NetworkManager.IsHost()) {
        buttons[0]->msg = I18n::get(L"menu.disconnect");
    }

    buttons.push_back(new Button(4, width / 2 - 100, height / 4 + 24 * 1 + yo,
                                 L"LBack to game"));
    buttons.push_back(new Button(0, width / 2 - 100, height / 4 + 24 * 4 + yo,
                                 L"LOptions..."));

    buttons.push_back(new Button(4, width / 2 - 100, height / 4 + 24 * 1 + yo,
                                 I18n::get(L"menu.returnToGame")));
    buttons.push_back(new Button(0, width / 2 - 100, height / 4 + 24 * 4 + yo,
                                 I18n::get(L"menu.options")));

    buttons.push_back(new Button(5, width / 2 - 100, height / 4 + 24 * 2 + yo,
                                 98, 20, I18n::get(L"gui.achievements")));
    buttons.push_back(new Button(6, width / 2 + 2, height / 4 + 24 * 2 + yo, 98,
                                 20, I18n::get(L"gui.stats")));
    /*
     * if (minecraft->serverConnection!=null) { buttons.get(1).active =
     * false; buttons.get(2).active = false; buttons.get(3).active = false;
     * }
     */
}

void PauseScreen::exitWorld(Minecraft* minecraft, bool save) {
    // 4jcraft: made our own static method for use in the java gui (other
    // places such as the deathscreen need this)
    MinecraftServer* server = MinecraftServer::getInstance();

    minecraft->setScreen(new MessageScreen(L"Leaving world"));
    if (g_NetworkManager.IsHost()) {
        server->setSaveOnExit(save);
    }
    app.SetAction(minecraft->player->GetXboxPad(), eAppAction_ExitWorld);
}

void PauseScreen::buttonClicked(Button* button) {
    if (button->id == 0) {
        minecraft->setScreen(new OptionsScreen(this, minecraft->options));
    }
    if (button->id == 1) {
        // if (minecraft->isClientSide())
        // {
        //     minecraft->level->disconnect();
        // }

        // minecraft->setLevel(nullptr);
        // minecraft->setScreen(new TitleScreen());

        // 4jcraft: exit with our new exitWorld method
        exitWorld(minecraft, true);
    }
    if (button->id == 4) {
        app.SetXuiServerAction(InputManager.GetPrimaryPad(),
                               eXuiServerAction_PauseServer, (void*)false);
        minecraft->setScreen(nullptr);
        //       minecraft->grabMouse();		// 4J - removed
    }

    if (button->id == 5) {
        //        minecraft->setScreen(new AchievementScreen(minecraft->stats));
        //        // 4J TODO - put back
    }
    if (button->id == 6) {
        //        minecraft->setScreen(new StatsScreen(this, minecraft->stats));
        //        // 4J TODO - put back
    }
}

void PauseScreen::tick() {
    Screen::tick();
    visibleTime++;
}

void PauseScreen::render(int xm, int ym, float a) {
    renderBackground();

    bool isSaving = false;  //! minecraft->level->pauseSave(saveStep++);
    if (isSaving || visibleTime < 20) {
        float col = ((visibleTime % 10) + a) / 10.0f;
        col = sinf(col * std::numbers::pi * 2) * 0.2f + 0.8f;
        int br = (int)(255 * col);

        drawString(font, L"Saving level..", 8, height - 16,
                   br << 16 | br << 8 | br);
    }

    drawCenteredString(font, L"Game menu", width / 2, 40, 0xffffff);

    Screen::render(xm, ym, a);
}