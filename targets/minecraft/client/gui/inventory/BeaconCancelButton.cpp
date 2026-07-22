#include "BeaconCancelButton.h"

#include <string>

#include "BeaconScreen.h"
#include "minecraft/client/gui/inventory/AbstractBeaconButton.h"
#include "minecraft/locale/Language.h"


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