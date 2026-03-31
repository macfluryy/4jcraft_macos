#include <memory>
#include <string>
#include <vector>

#include "ReceivingLevelScreen.h"
#include "ClientConnection.h"
#include "Minecraft.World/net/minecraft/locale/Language.h"
#include "Minecraft.World/net/minecraft/network/packet/KeepAlivePacket.h"
#include "Minecraft.Client/net/minecraft/client/gui/Screen.h"

ReceivingLevelScreen::ReceivingLevelScreen(ClientConnection* connection) {
    tickCount = 0;
    this->connection = connection;
}

void ReceivingLevelScreen::keyPressed(char eventCharacter, int eventKey) {}

void ReceivingLevelScreen::init() { buttons.clear(); }

void ReceivingLevelScreen::tick() {
    tickCount++;
    if (tickCount % 20 == 0) {
        connection->send(
            std::make_shared<KeepAlivePacket>());
    }
    if (connection != nullptr) {
        connection->tick();
    }
}

void ReceivingLevelScreen::buttonClicked(Button* button) {}

void ReceivingLevelScreen::render(int xm, int ym, float a) {
    renderDirtBackground(0);

    Language* language = Language::getInstance();

    drawCenteredString(font,
                       language->getElement(L"multiplayer.downloadingTerrain"),
                       width / 2, height / 2 - 50, 0xffffff);

    Screen::render(xm, ym, a);
}