#pragma once

#include "Item.h"

class FireworksChargeItem : public Item {
private:
    Icon* overlay;

public:
    FireworksChargeItem(int id);

    virtual Icon* getLayerIcon(int auxValue, int spriteLayer);
    virtual int getColor(std::shared_ptr<ItemInstance> item, int spriteLayer);
    virtual bool hasMultipleSpriteLayers();

    static Tag* getExplosionTagField(std::shared_ptr<ItemInstance> instance,
                                     const std::wstring& field);

    virtual void appendHoverText(std::shared_ptr<ItemInstance> itemInstance,
                                 std::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool advanced);

    static void appendHoverText(CompoundTag* expTag,
                                std::vector<HtmlString>* lines);

    virtual void registerIcons(IconRegister* iconRegister);
};