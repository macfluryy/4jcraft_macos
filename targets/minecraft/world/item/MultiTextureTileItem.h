#pragma once

#include "TileItem.h"

class Tile;

class MultiTextureTileItem : public TileItem {
private:
    Tile* parentTile;
    
    int* nameExtensions;
    int m_iNameExtensionsLength;
    int m_anyValueName;  

public:
    MultiTextureTileItem(int id, Tile* parentTile, int* nameExtensions,
                         int iLength,
                         int anyValueName = -1);  

    virtual Icon* getIcon(int itemAuxValue);
    virtual int getLevelDataForAuxValue(int auxValue);
    virtual unsigned int getDescriptionId(int iData = -1);
    virtual unsigned int getDescriptionId(
        std::shared_ptr<ItemInstance> instance);
};
