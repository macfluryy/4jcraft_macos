#include <memory>

#include "CompassItem.h"
#include "Minecraft.Client/net/minecraft/client/Minecraft.h"
#include "Minecraft.Client/net/minecraft/client/multiplayer/MultiPlayerLocalPlayer.h"
#include "Minecraft.World/net/minecraft/world/IconRegister.h"
#include "Minecraft.World/net/minecraft/world/item/Item.h"

class Icon;

const std::wstring CompassItem::TEXTURE_PLAYER_ICON[XUSER_MAX_COUNT] = {
    L"compassP0", L"compassP1", L"compassP2", L"compassP3"};

CompassItem::CompassItem(int id) : Item(id) { icons = nullptr; }

// 4J Added so that we can override the icon id used to calculate the texture
// UV's for each player

Icon* CompassItem::getIcon(int auxValue) {
    Icon* icon = Item::getIcon(auxValue);
    Minecraft* pMinecraft = Minecraft::GetInstance();

    if (pMinecraft->player != nullptr && auxValue == 0) {
        icon = icons[pMinecraft->player->GetXboxPad()];
    }
    return icon;
}

void CompassItem::registerIcons(IconRegister* iconRegister) {
    Item::registerIcons(iconRegister);

    icons = new Icon*[XUSER_MAX_COUNT];

    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
        icons[i] = iconRegister->registerIcon(TEXTURE_PLAYER_ICON[i]);
    }
}
