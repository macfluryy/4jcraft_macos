#pragma once

#include <memory>
#include <string>
#include <vector>

#include "EnchantmentCategory.h"
#include "minecraft/util/HtmlString.h"

class DamageSource;
class LivingEntity;
class ItemInstance;
class Mob;
class EnchantmentCategory;

class Enchantment  
{
public:
    
    static std::vector<Enchantment*> enchantments;
    static std::vector<Enchantment*> validEnchantments;

    static const int FREQ_COMMON = 10;
    static const int FREQ_UNCOMMON = 5;
    static const int FREQ_RARE = 2;
    static const int FREQ_VERY_RARE = 1;

    
    static Enchantment* allDamageProtection;
    static Enchantment* fireProtection;
    static Enchantment* fallProtection;
    static Enchantment* explosionProtection;
    static Enchantment* projectileProtection;
    static Enchantment* drownProtection;
    static Enchantment* waterWorker;
    static Enchantment* thorns;

    
    static Enchantment* damageBonus;
    static Enchantment* damageBonusUndead;
    static Enchantment* damageBonusArthropods;
    static Enchantment* knockback;
    static Enchantment* fireAspect;
    static Enchantment* lootBonus;

    
    static Enchantment* diggingBonus;
    static Enchantment* untouching;
    static Enchantment* digDurability;
    static Enchantment* resourceBonus;

    
    static Enchantment* arrowBonus;
    static Enchantment* arrowKnockback;
    static Enchantment* arrowFire;
    static Enchantment* arrowInfinite;

    const int id;

    static void staticCtor();

private:
    const int frequency;

public:
    const EnchantmentCategory* category;

protected:
    int descriptionId;

private:
    void _init(int id);

protected:
    Enchantment(int id, int frequency, const EnchantmentCategory* category);
    Enchantment(int id);

public:
    virtual int getFrequency();
    virtual int getMinLevel();
    virtual int getMaxLevel();
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getDamageProtection(int level, DamageSource* source);
    virtual float getDamageBonus(int level,
                                 std::shared_ptr<LivingEntity> target);
    virtual bool isCompatibleWith(Enchantment* other) const;
    virtual Enchantment* setDescriptionId(int id);
    virtual int getDescriptionId();
    
    virtual std::wstring getFullname(
        int level, std::wstring& unformatted);  
    virtual HtmlString getFullname(int level);
    virtual bool canEnchant(std::shared_ptr<ItemInstance> item);

private:
    
    std::wstring getLevelString(int level);
};