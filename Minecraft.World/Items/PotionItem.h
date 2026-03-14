#pragma once

#include "Item.h"

class MobEffectInstance;

class PotionItem : public Item {
private:
    static const int DRINK_DURATION = (int)(20 * 1.6);

public:
    static const std::wstring DEFAULT_ICON;
    static const std::wstring THROWABLE_ICON;
    static const std::wstring CONTENTS_ICON;

private:
    std::unordered_map<int, std::vector<MobEffectInstance*>*> cachedMobEffects;

    Icon* iconThrowable;
    Icon* iconDrinkable;
    Icon* iconOverlay;

public:
    PotionItem(int id);

    virtual std::vector<MobEffectInstance*>* getMobEffects(
        std::shared_ptr<ItemInstance> potion);
    virtual std::vector<MobEffectInstance*>* getMobEffects(int auxValue);
    virtual std::shared_ptr<ItemInstance> useTimeDepleted(
        std::shared_ptr<ItemInstance> instance, Level* level,
        std::shared_ptr<Player> player);
    virtual int getUseDuration(std::shared_ptr<ItemInstance> itemInstance);
    virtual UseAnim getUseAnimation(std::shared_ptr<ItemInstance> itemInstance);
    virtual std::shared_ptr<ItemInstance> use(
        std::shared_ptr<ItemInstance> instance, Level* level,
        std::shared_ptr<Player> player);
    virtual bool TestUse(Level* level, std::shared_ptr<Player> player);
    virtual bool useOn(std::shared_ptr<ItemInstance> itemInstance,
                       std::shared_ptr<Player> player, Level* level, int x,
                       int y, int z, int face, float clickX, float clickY,
                       float clickZ, bool bTestUseOnOnly = false);
    virtual Icon* getIcon(int auxValue);
    virtual Icon* getLayerIcon(int auxValue, int spriteLayer);
    static bool isThrowable(int auxValue);
    int getColor(int data);
    virtual int getColor(std::shared_ptr<ItemInstance> item, int spriteLayer);
    virtual bool hasMultipleSpriteLayers();
    virtual bool hasInstantenousEffects(int itemAuxValue);
    virtual std::wstring getHoverName(
        std::shared_ptr<ItemInstance> itemInstance);
    virtual void appendHoverText(std::shared_ptr<ItemInstance> itemInstance,
                                 std::shared_ptr<Player> player,
                                 std::vector<std::wstring>* lines,
                                 bool advanced,
                                 std::vector<std::wstring>& unformattedStrings);
    virtual bool isFoil(std::shared_ptr<ItemInstance> itemInstance);

    virtual unsigned int getUseDescriptionId(
        std::shared_ptr<ItemInstance> instance);

    //@Override
    void registerIcons(IconRegister* iconRegister);
    static Icon* getTexture(const std::wstring& name);

    // 4J Stu - Based loosely on a function that gets added in java much later
    // on (1.3)
    static std::vector<std::pair<int, int> >* getUniquePotionValues();

private:
    // 4J Stu - Added to support function above, different from Java
    // implementation
    static std::vector<std::pair<int, int> > s_uniquePotionValues;
};