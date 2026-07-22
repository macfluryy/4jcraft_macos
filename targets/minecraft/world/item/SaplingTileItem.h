#pragma once

#include "TileItem.h"

class SaplingTileItem : public TileItem {
public:
    SaplingTileItem(int id);

    virtual int getLevelDataForAuxValue(int auxValue);
    virtual Icon* getIcon(int itemAuxValue);

    
    virtual unsigned int getDescriptionId(
        std::shared_ptr<ItemInstance> instance);
};