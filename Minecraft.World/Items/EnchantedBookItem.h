#pragma once

#include "Item.h"

class EnchantmentInstance;

class EnchantedBookItem : public Item {
public:
    static const std::wstring TAG_STORED_ENCHANTMENTS;

    EnchantedBookItem(int id);

    bool isFoil(std::shared_ptr<ItemInstance> itemInstance);
    bool isEnchantable(std::shared_ptr<ItemInstance> itemInstance);
    const Rarity* getRarity(std::shared_ptr<ItemInstance> itemInstance);
    ListTag<CompoundTag>* getEnchantments(std::shared_ptr<ItemInstance> item);
    void appendHoverText(std::shared_ptr<ItemInstance> itemInstance,
                         std::shared_ptr<Player> player,
                         std::vector<std::wstring>* lines, bool advanced,
                         std::vector<std::wstring>& unformattedStrings);
    void addEnchantment(std::shared_ptr<ItemInstance> item,
                        EnchantmentInstance* enchantment);
    std::shared_ptr<ItemInstance> createForEnchantment(
        EnchantmentInstance* enchant);
    void createForEnchantment(
        Enchantment* enchant,
        std::vector<std::shared_ptr<ItemInstance> >* items);
    std::shared_ptr<ItemInstance> createForRandomLoot(Random* random);
    WeighedTreasure* createForRandomTreasure(Random* random);
    WeighedTreasure* createForRandomTreasure(Random* random, int minCount,
                                             int maxCount, int weight);
};