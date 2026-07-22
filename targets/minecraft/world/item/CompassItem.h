#pragma once



#include <string>

#include "platform/PlatformTypes.h"
#include "Item.h"

class Icon;

class CompassItem : public Item {
private:
    Icon** icons;
    static const std::wstring TEXTURE_PLAYER_ICON[XUSER_MAX_COUNT];

public:
    CompassItem(int id);

    virtual Icon* getIcon(int auxValue);

    
    void registerIcons(IconRegister* iconRegister);
};