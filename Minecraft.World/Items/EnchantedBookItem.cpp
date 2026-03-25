#include "../Platform/stdafx.h"
#include "../Headers/net.minecraft.world.item.enchantment.h"
#include "../Headers/net.minecraft.world.item.h"
#include "../Util/WeighedTreasure.h"
#include "EnchantedBookItem.h"

const std::wstring EnchantedBookItem::TAG_STORED_ENCHANTMENTS =
    L"StoredEnchantments";

EnchantedBookItem::EnchantedBookItem(int id) : Item(id) {}

bool EnchantedBookItem::isFoil(std::shared_ptr<ItemInstance> itemInstance) {
    return true;
}

bool EnchantedBookItem::isEnchantable(
    std::shared_ptr<ItemInstance> itemInstance) {
    return false;
}

const Rarity* EnchantedBookItem::getRarity(
    std::shared_ptr<ItemInstance> itemInstance) {
    ListTag<CompoundTag>* enchantments = getEnchantments(itemInstance);
    if (enchantments && enchantments->size() > 0) {
        return Rarity::uncommon;
    } else {
        return Item::getRarity(itemInstance);
    }
}

ListTag<CompoundTag>* EnchantedBookItem::getEnchantments(
    std::shared_ptr<ItemInstance> item) {
    if (item->tag == NULL ||
        !item->tag->contains((wchar_t*)TAG_STORED_ENCHANTMENTS.c_str())) {
        return new ListTag<CompoundTag>();
    }

    return (ListTag<CompoundTag>*)item->tag->get(
        (wchar_t*)TAG_STORED_ENCHANTMENTS.c_str());
}

void EnchantedBookItem::appendHoverText(
    std::shared_ptr<ItemInstance> itemInstance, std::shared_ptr<Player> player,
    std::vector<HtmlString>* lines, bool advanced) {
    Item::appendHoverText(itemInstance, player, lines, advanced);

    ListTag<CompoundTag>* list = getEnchantments(itemInstance);

    if (list != NULL) {
        std::wstring unformatted = L"";
        for (int i = 0; i < list->size(); i++) {
            int type =
                list->get(i)->getShort((wchar_t*)ItemInstance::TAG_ENCH_ID);
            int level =
                list->get(i)->getShort((wchar_t*)ItemInstance::TAG_ENCH_LEVEL);

            if (Enchantment::enchantments[type] != NULL) {
                lines->push_back(
                    Enchantment::enchantments[type]->getFullname(level));
            }
        }
    }
}

void EnchantedBookItem::addEnchantment(std::shared_ptr<ItemInstance> item,
                                       EnchantmentInstance* enchantment) {
    ListTag<CompoundTag>* enchantments = getEnchantments(item);
    bool add = true;

    for (int i = 0; i < enchantments->size(); i++) {
        CompoundTag* tag = enchantments->get(i);

        if (tag->getShort((wchar_t*)ItemInstance::TAG_ENCH_ID) ==
            enchantment->enchantment->id) {
            if (tag->getShort((wchar_t*)ItemInstance::TAG_ENCH_LEVEL) <
                enchantment->level) {
                tag->putShort((wchar_t*)ItemInstance::TAG_ENCH_LEVEL,
                              (short)enchantment->level);
            }

            add = false;
            break;
        }
    }

    if (add) {
        CompoundTag* tag = new CompoundTag();

        tag->putShort((wchar_t*)ItemInstance::TAG_ENCH_ID,
                      (short)enchantment->enchantment->id);
        tag->putShort((wchar_t*)ItemInstance::TAG_ENCH_LEVEL,
                      (short)enchantment->level);

        enchantments->add(tag);
    }

    if (!item->hasTag()) item->setTag(new CompoundTag());
    item->getTag()->put((wchar_t*)TAG_STORED_ENCHANTMENTS.c_str(),
                        enchantments);
}

std::shared_ptr<ItemInstance> EnchantedBookItem::createForEnchantment(
    EnchantmentInstance* enchant) {
    std::shared_ptr<ItemInstance> item =
        std::shared_ptr<ItemInstance>(new ItemInstance(this));
    addEnchantment(item, enchant);
    return item;
}

void EnchantedBookItem::createForEnchantment(
    Enchantment* enchant, std::vector<std::shared_ptr<ItemInstance> >* items) {
    for (int i = enchant->getMinLevel(); i <= enchant->getMaxLevel(); i++) {
        items->push_back(
            createForEnchantment(new EnchantmentInstance(enchant, i)));
    }
}

std::shared_ptr<ItemInstance> EnchantedBookItem::createForRandomLoot(
    Random* random) {
    Enchantment* enchantment = Enchantment::validEnchantments[random->nextInt(
        Enchantment::validEnchantments.size())];
    std::shared_ptr<ItemInstance> book =
        std::shared_ptr<ItemInstance>(new ItemInstance(id, 1, 0));
    int level = Mth::nextInt(random, enchantment->getMinLevel(),
                             enchantment->getMaxLevel());

    addEnchantment(book, new EnchantmentInstance(enchantment, level));

    return book;
}

WeighedTreasure* EnchantedBookItem::createForRandomTreasure(Random* random) {
    return createForRandomTreasure(random, 1, 1, 1);
}

WeighedTreasure* EnchantedBookItem::createForRandomTreasure(Random* random,
                                                            int minCount,
                                                            int maxCount,
                                                            int weight) {
    Enchantment* enchantment = Enchantment::validEnchantments[random->nextInt(
        Enchantment::validEnchantments.size())];
    std::shared_ptr<ItemInstance> book =
        std::shared_ptr<ItemInstance>(new ItemInstance(id, 1, 0));
    int level = Mth::nextInt(random, enchantment->getMinLevel(),
                             enchantment->getMaxLevel());

    addEnchantment(book, new EnchantmentInstance(enchantment, level));

    return new WeighedTreasure(book, minCount, maxCount, weight);
}