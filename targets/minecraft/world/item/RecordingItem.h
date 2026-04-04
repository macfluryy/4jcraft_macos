#pragma once

#include <string>
#include <unordered_map>

#include "Item.h"

class RecordingItem : public Item {
private:
    static std::unordered_map<std::wstring, RecordingItem*> BY_NAME;

public:
    const std::wstring recording;

public
    :  // 4J Stu - Was protected in Java, but the can't access it where we need
    RecordingItem(int id, const std::wstring& recording);

    Icon* getIcon(int auxValue);
    virtual bool useOn(std::shared_ptr<ItemInstance> itemInstance,
                       std::shared_ptr<Player> player, Level* level, int x,
                       int y, int z, int face, float clickX, float clickY,
                       float clickZ, bool bTestUseOnOnly = false);

    virtual void appendHoverText(std::shared_ptr<ItemInstance> itemInstance,
                                 std::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool advanced);
    virtual const Rarity* getRarity(std::shared_ptr<ItemInstance> itemInstance);

    void registerIcons(IconRegister* iconRegister);
    static RecordingItem* getByName(const std::wstring& name);
};