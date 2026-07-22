#pragma once

#include <memory>
#include <vector>

#include "Item.h"
#include "java/Class.h"

class HangingEntity;
class Level;

class HangingEntityItem : public Item {
private:
    
    eINSTANCEOF eType;

public:
    HangingEntityItem(int id, eINSTANCEOF eClassType);

    virtual bool useOn(
        std::shared_ptr<ItemInstance> instance, std::shared_ptr<Player> player,
        Level* level, int xt, int yt, int zt, int face, float clickX,
        float clickY, float clickZ,
        bool bTestUseOnOnly);  

private:
    std::shared_ptr<HangingEntity> createEntity(
        Level* level, int x, int y, int z, int dir,
        int auxValue);  

public:
    virtual void appendHoverText(std::shared_ptr<ItemInstance> itemInstance,
                                 std::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool advanced);
};
