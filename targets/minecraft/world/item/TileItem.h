#pragma once

#include <memory>

#include "Item.h"

class Player;
class Level;
class Icon;
class ItemInstance;

class TileItem : public Item {
public:
    static const int _class = 0;
    using Item::getColor;

private:
    int tileId;
    Icon* itemIcon;

public:
    TileItem(int id);

    virtual int getTileId();

    
    int getIconType();

    
    Icon* getIcon(int auxValue);

    virtual bool useOn(std::shared_ptr<ItemInstance> instance,
                       std::shared_ptr<Player> player, Level* level, int x,
                       int y, int z, int face, float clickX, float clickY,
                       float clickZ, bool bTestUseOnOnly = false);
    virtual unsigned int getDescriptionId(
        std::shared_ptr<ItemInstance> instance);
    virtual unsigned int getDescriptionId(int iData = -1);

    
    virtual int getColor(int itemAuxValue, int spriteLayer);

    
    virtual unsigned int getUseDescriptionId(
        std::shared_ptr<ItemInstance> instance);
    virtual unsigned int getUseDescriptionId();

    virtual bool mayPlace(Level* level, int x, int y, int z, int face,
                          std::shared_ptr<Player> player,
                          std::shared_ptr<ItemInstance> item);

    
    virtual void registerIcons(IconRegister* iconRegister);
};
