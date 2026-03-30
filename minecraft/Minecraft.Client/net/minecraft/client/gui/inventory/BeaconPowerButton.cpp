#include "Minecraft.World/Header Files/stdafx.h"
#include "BeaconPowerButton.h"
#include "BeaconScreen.h"
#include "Minecraft.World/net/minecraft/world/effect/MobEffect.h"
#include "Minecraft.World/net/minecraft/locale/net.minecraft.locale.h"
#include "../../renderer/Textures.h"
#include "../../resources/ResourceLocation.h"

// 4jcraft: referenced from MCP 8.11 (JE 1.6.4)
#ifdef ENABLE_JAVA_GUIS
ResourceLocation GUI_INVENTORY_LOCATION = ResourceLocation(TN_GUI_INVENTORY);
#endif

BeaconPowerButton::BeaconPowerButton(BeaconScreen* screen, int id, int x, int y,
                                     int effectId, int tier)
    : AbstractBeaconButton(id, x, y) {
    this->screen = screen;
    this->effectId = effectId;
    this->tier = tier;

#ifdef ENABLE_JAVA_GUIS
    this->iconRes = &GUI_INVENTORY_LOCATION;
#endif

    int statusIconIndex = MobEffect::javaId(effectId);
    this->iconU = (statusIconIndex % 8) * 18;
    this->iconV = 198 + (statusIconIndex / 8) * 18;
}

void BeaconPowerButton::renderTooltip(int xm, int ym) {
    MobEffect* effect = MobEffect::effects[effectId];
    if (!effect) return;

    std::wstring name = app.GetString(effect->getDescriptionId());
    if (tier >= 3 && effect->id != MobEffect::regeneration->id) {
        name += L" II";
    }
    screen->renderTooltip(name, xm, ym);
}