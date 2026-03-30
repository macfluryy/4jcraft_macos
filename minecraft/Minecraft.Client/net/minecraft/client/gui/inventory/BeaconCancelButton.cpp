#include "Minecraft.World/Header Files/stdafx.h"
#include "BeaconCancelButton.h"
#include "BeaconScreen.h"
#include "Minecraft.World/net/minecraft/locale/net.minecraft.locale.h"

// 4jcraft: referenced from MCP 8.11 (JE 1.6.4)
#ifdef ENABLE_JAVA_GUIS
extern ResourceLocation GUI_BEACON_LOCATION;
#endif

BeaconCancelButton::BeaconCancelButton(BeaconScreen* screen, int id, int x,
                                       int y)
    : AbstractBeaconButton(id, x, y) {
    this->screen = screen;
#ifdef ENABLE_JAVA_GUIS
    this->iconRes = &GUI_BEACON_LOCATION;
#endif
    this->iconU = 112;
    this->iconV = 220;
}

void BeaconCancelButton::renderTooltip(int xm, int ym) {
    screen->renderTooltip(Language::getInstance()->getElement(L"gui.cancel"),
                          xm, ym);
}