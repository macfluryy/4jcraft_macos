#include "../Platform/stdafx.h"
#include "../Headers/net.minecraft.world.item.h"
#include "../Headers/net.minecraft.world.item.alchemy.h"
#include "../Headers/net.minecraft.world.effect.h"
#include "../Headers/net.minecraft.world.level.h"
#include "../Headers/net.minecraft.world.entity.ai.attributes.h"
#include "../Headers/net.minecraft.world.entity.player.h"
#include "../Headers/net.minecraft.world.entity.projectile.h"
#include "../Headers/net.minecraft.world.h"
#include "../Entities/MobEffectInstance.h"
#include "../Util/StringHelpers.h"
#include "../Util/SharedConstants.h"
#include "PotionItem.h"
#include "../Util/SoundTypes.h"

const std::wstring PotionItem::DEFAULT_ICON = L"potion";
const std::wstring PotionItem::THROWABLE_ICON = L"potion_splash";
const std::wstring PotionItem::CONTENTS_ICON = L"potion_contents";

// 4J Added
std::vector<std::pair<int, int> > PotionItem::s_uniquePotionValues;

PotionItem::PotionItem(int id) : Item(id) {
    setMaxStackSize(1);
    setStackedByData(true);
    setMaxDamage(0);

    iconThrowable = nullptr;
    iconDrinkable = nullptr;
    iconOverlay = nullptr;
}

std::vector<MobEffectInstance*>* PotionItem::getMobEffects(
    std::shared_ptr<ItemInstance> potion) {
    if (!potion->hasTag() ||
        !potion->getTag()->contains(L"CustomPotionEffects")) {
        std::vector<MobEffectInstance*>* effects = nullptr;
        auto it = cachedMobEffects.find(potion->getAuxValue());
        if (it != cachedMobEffects.end()) effects = it->second;
        if (effects == nullptr) {
            effects = PotionBrewing::getEffects(potion->getAuxValue(), false);
            cachedMobEffects[potion->getAuxValue()] = effects;
        }

        // Result should be a new (unmanaged) vector, so create a new one
        return effects == nullptr
                   ? nullptr
                   : new std::vector<MobEffectInstance*>(*effects);
    } else {
        std::vector<MobEffectInstance*>* effects =
            new std::vector<MobEffectInstance*>();
        ListTag<CompoundTag>* customList =
            (ListTag<CompoundTag>*)potion->getTag()->getList(
                L"CustomPotionEffects");

        for (int i = 0; i < customList->size(); i++) {
            CompoundTag* tag = customList->get(i);
            effects->push_back(MobEffectInstance::load(tag));
        }

        return effects;
    }
}

std::vector<MobEffectInstance*>* PotionItem::getMobEffects(int auxValue) {
    std::vector<MobEffectInstance*>* effects = nullptr;
    auto it = cachedMobEffects.find(auxValue);
    if (it != cachedMobEffects.end()) effects = it->second;
    if (effects == nullptr) {
        effects = PotionBrewing::getEffects(auxValue, false);
        if (effects != nullptr)
            cachedMobEffects.insert(
                std::pair<int, std::vector<MobEffectInstance*>*>(auxValue,
                                                                 effects));
    }
    return effects;
}

std::shared_ptr<ItemInstance> PotionItem::useTimeDepleted(
    std::shared_ptr<ItemInstance> instance, Level* level,
    std::shared_ptr<Player> player) {
    if (!player->abilities.instabuild) instance->count--;

    if (!level->isClientSide) {
        std::vector<MobEffectInstance*>* effects = getMobEffects(instance);
        if (effects != nullptr) {
            // for (MobEffectInstance effect : effects)
            for (auto it = effects->begin(); it != effects->end(); ++it) {
                player->addEffect(new MobEffectInstance(*it));
            }
        }
    }
    if (!player->abilities.instabuild) {
        if (instance->count <= 0) {
            return std::shared_ptr<ItemInstance>(
                new ItemInstance(Item::glassBottle));
        } else {
            player->inventory->add(std::shared_ptr<ItemInstance>(
                new ItemInstance(Item::glassBottle)));
        }
    }

    return instance;
}

int PotionItem::getUseDuration(std::shared_ptr<ItemInstance> itemInstance) {
    return DRINK_DURATION;
}

UseAnim PotionItem::getUseAnimation(
    std::shared_ptr<ItemInstance> itemInstance) {
    return UseAnim_drink;
}

bool PotionItem::TestUse(std::shared_ptr<ItemInstance> itemInstance,
                         Level* level, std::shared_ptr<Player> player) {
    return true;
}

std::shared_ptr<ItemInstance> PotionItem::use(
    std::shared_ptr<ItemInstance> instance, Level* level,
    std::shared_ptr<Player> player) {
    if (isThrowable(instance->getAuxValue())) {
        if (!player->abilities.instabuild) instance->count--;
        level->playEntitySound(player, eSoundType_RANDOM_BOW, 0.5f,
                               0.4f / (random->nextFloat() * 0.4f + 0.8f));
        if (!level->isClientSide)
            level->addEntity(std::shared_ptr<ThrownPotion>(
                new ThrownPotion(level, player, instance->getAuxValue())));
        return instance;
    }
    player->startUsingItem(instance, getUseDuration(instance));
    return instance;
}

bool PotionItem::useOn(std::shared_ptr<ItemInstance> itemInstance,
                       std::shared_ptr<Player> player, Level* level, int x,
                       int y, int z, int face, float clickX, float clickY,
                       float clickZ, bool bTestUseOnOnly) {
    return false;
}

Icon* PotionItem::getIcon(int auxValue) {
    if (isThrowable(auxValue)) {
        return iconThrowable;
    }
    return iconDrinkable;
}

Icon* PotionItem::getLayerIcon(int auxValue, int spriteLayer) {
    if (spriteLayer == 0) {
        return iconOverlay;
    }
    return Item::getLayerIcon(auxValue, spriteLayer);
}

bool PotionItem::isThrowable(int auxValue) {
    return ((auxValue & PotionBrewing::THROWABLE_MASK) != 0);
}

int PotionItem::getColor(int data) {
    return PotionBrewing::getColorValue(data, false);
}

int PotionItem::getColor(std::shared_ptr<ItemInstance> item, int spriteLayer) {
    if (spriteLayer > 0) {
        return 0xffffff;
    }
    return PotionBrewing::getColorValue(item->getAuxValue(), false);
}

bool PotionItem::hasMultipleSpriteLayers() { return true; }

bool PotionItem::hasInstantenousEffects(int itemAuxValue) {
    std::vector<MobEffectInstance*>* mobEffects = getMobEffects(itemAuxValue);
    if (mobEffects == nullptr || mobEffects->empty()) {
        return false;
    }
    // for (MobEffectInstance effect : mobEffects) {
    for (auto it = mobEffects->begin(); it != mobEffects->end(); ++it) {
        MobEffectInstance* effect = *it;
        if (MobEffect::effects[effect->getId()]->isInstantenous()) {
            return true;
        }
    }
    return false;
}

std::wstring PotionItem::getHoverName(
    std::shared_ptr<ItemInstance> itemInstance) {
    if (itemInstance->getAuxValue() == 0) {
        return app.GetString(
            IDS_ITEM_WATER_BOTTLE);  // I18n.get("item.emptyPotion.name").trim();
    }

    std::wstring elementName = Item::getHoverName(itemInstance);
    if (isThrowable(itemInstance->getAuxValue())) {
        // elementName = I18n.get("potion.prefix.grenade").trim() + " " +
        // elementName;
        elementName = replaceAll(elementName, L"{*splash*}",
                                 app.GetString(IDS_POTION_PREFIX_GRENADE));
    } else {
        elementName = replaceAll(elementName, L"{*splash*}", L"");
    }

    std::vector<MobEffectInstance*>* effects =
        ((PotionItem*)Item::potion)->getMobEffects(itemInstance);
    if (effects != nullptr && !effects->empty()) {
        // String postfixString = effects.get(0).getDescriptionId();
        // postfixString += ".postfix";
        // return elementName + " " + I18n.get(postfixString).trim();

        elementName = replaceAll(elementName, L"{*prefix*}", L"");
        elementName = replaceAll(
            elementName, L"{*postfix*}",
            app.GetString(effects->at(0)->getPostfixDescriptionId()));
    } else {
        // String appearanceName =
        // PotionBrewing.getAppearanceName(itemInstance.getAuxValue()); return
        // I18n.get(appearanceName).trim() + " " + elementName;

        elementName = replaceAll(elementName, L"{*prefix*}",
                                 app.GetString(PotionBrewing::getAppearanceName(
                                     itemInstance->getAuxValue())));
        elementName = replaceAll(elementName, L"{*postfix*}", L"");
    }
    return elementName;
}

void PotionItem::appendHoverText(std::shared_ptr<ItemInstance> itemInstance,
                                 std::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines,
                                 bool advanced) {
    if (itemInstance->getAuxValue() == 0) {
        return;
    }
    std::vector<MobEffectInstance*>* effects =
        ((PotionItem*)Item::potion)->getMobEffects(itemInstance);
    attrAttrModMap modifiers;
    if (effects != nullptr && !effects->empty()) {
        // for (MobEffectInstance effect : effects)
        for (auto it = effects->begin(); it != effects->end(); ++it) {
            MobEffectInstance* effect = *it;
            std::wstring effectString =
                app.GetString(effect->getDescriptionId());

            MobEffect* mobEffect = MobEffect::effects[effect->getId()];
            std::unordered_map<Attribute*, AttributeModifier*>*
                effectModifiers = mobEffect->getAttributeModifiers();

            if (effectModifiers != nullptr && effectModifiers->size() > 0) {
                for (auto it = effectModifiers->begin();
                     it != effectModifiers->end(); ++it) {
                    // 4J - anonymous modifiers added here are destroyed
                    // shortly?
                    AttributeModifier* original = it->second;
                    AttributeModifier* modifier = new AttributeModifier(
                        mobEffect->getAttributeModifierValue(
                            effect->getAmplifier(), original),
                        original->getOperation());
                    modifiers.insert(
                        std::pair<eATTRIBUTE_ID, AttributeModifier*>(
                            it->first->getId(), modifier));
                }
            }

            // Don't want to delete this (that's a pointer to mobEffects
            // internal vector of modifiers) delete effectModifiers;

            if (effect->getAmplifier() > 0) {
                std::wstring potencyString = L"";
                switch (effect->getAmplifier()) {
                    case 1:
                        potencyString = L" ";
                        potencyString += app.GetString(IDS_POTION_POTENCY_1);
                        break;
                    case 2:
                        potencyString = L" ";
                        potencyString += app.GetString(IDS_POTION_POTENCY_2);
                        break;
                    case 3:
                        potencyString = L" ";
                        potencyString += app.GetString(IDS_POTION_POTENCY_3);
                        break;
                    default:
                        potencyString = app.GetString(IDS_POTION_POTENCY_0);
                        break;
                }
                effectString +=
                    potencyString;  // + I18n.get("potion.potency." +
                                    // effect.getAmplifier()).trim();
            }
            if (effect->getDuration() > SharedConstants::TICKS_PER_SECOND) {
                effectString +=
                    L" (" + MobEffect::formatDuration(effect) + L")";
            }

            eMinecraftColour color = eMinecraftColour_NOT_SET;

            if (mobEffect->isHarmful()) {
                color = eHTMLColor_c;
            } else {
                color = eHTMLColor_7;
            }

            lines->push_back(HtmlString(effectString, color));
        }
    } else {
        std::wstring effectString = app.GetString(
            IDS_POTION_EMPTY);  // I18n.get("potion.empty").trim();

        lines->push_back(HtmlString(effectString, eHTMLColor_7));  //"�7"
    }

    if (!modifiers.empty()) {
        // Add new line
        lines->push_back(HtmlString(L""));
        lines->push_back(HtmlString(app.GetString(IDS_POTION_EFFECTS_WHENDRANK),
                                    eHTMLColor_5));

        // Add modifier descriptions
        for (auto it = modifiers.begin(); it != modifiers.end(); ++it) {
            // 4J: Moved modifier string building to AttributeModifier
            lines->push_back(it->second->getHoverText(it->first));
        }
    }
}

bool PotionItem::isFoil(std::shared_ptr<ItemInstance> itemInstance) {
    std::vector<MobEffectInstance*>* mobEffects = getMobEffects(itemInstance);
    return mobEffects != nullptr && !mobEffects->empty();
}

unsigned int PotionItem::getUseDescriptionId(
    std::shared_ptr<ItemInstance> instance) {
    int brew = instance->getAuxValue();
    if (brew == 0)
        return IDS_POTION_DESC_WATER_BOTTLE;
    else if (MACRO_POTION_IS_REGENERATION(brew))
        return IDS_POTION_DESC_REGENERATION;
    else if (MACRO_POTION_IS_SPEED(brew))
        return IDS_POTION_DESC_MOVESPEED;
    else if (MACRO_POTION_IS_FIRE_RESISTANCE(brew))
        return IDS_POTION_DESC_FIRERESISTANCE;
    else if (MACRO_POTION_IS_INSTANTHEALTH(brew))
        return IDS_POTION_DESC_HEAL;
    else if (MACRO_POTION_IS_NIGHTVISION(brew))
        return IDS_POTION_DESC_NIGHTVISION;
    else if (MACRO_POTION_IS_INVISIBILITY(brew))
        return IDS_POTION_DESC_INVISIBILITY;
    else if (MACRO_POTION_IS_WEAKNESS(brew))
        return IDS_POTION_DESC_WEAKNESS;
    else if (MACRO_POTION_IS_STRENGTH(brew))
        return IDS_POTION_DESC_DAMAGEBOOST;
    else if (MACRO_POTION_IS_SLOWNESS(brew))
        return IDS_POTION_DESC_MOVESLOWDOWN;
    else if (MACRO_POTION_IS_POISON(brew))
        return IDS_POTION_DESC_POISON;
    else if (MACRO_POTION_IS_INSTANTDAMAGE(brew))
        return IDS_POTION_DESC_HARM;
    return IDS_POTION_DESC_EMPTY;
}

void PotionItem::registerIcons(IconRegister* iconRegister) {
    iconDrinkable = iconRegister->registerIcon(DEFAULT_ICON);
    iconThrowable = iconRegister->registerIcon(THROWABLE_ICON);
    iconOverlay = iconRegister->registerIcon(CONTENTS_ICON);
}

Icon* PotionItem::getTexture(const std::wstring& name) {
    if (name.compare(DEFAULT_ICON) == 0) return Item::potion->iconDrinkable;
    if (name.compare(THROWABLE_ICON) == 0) return Item::potion->iconThrowable;
    if (name.compare(CONTENTS_ICON) == 0) return Item::potion->iconOverlay;
    return nullptr;
}

// 4J Stu - Based loosely on a function that gets added in java much later on
// (1.3)
std::vector<std::pair<int, int> >* PotionItem::getUniquePotionValues() {
    if (s_uniquePotionValues.empty()) {
        for (int brew = 0; brew <= PotionBrewing::BREW_MASK; ++brew) {
            std::vector<MobEffectInstance*>* effects =
                PotionBrewing::getEffects(brew, false);

            if (effects != nullptr) {
                if (!effects->empty()) {
                    // 4J Stu - Based on implementation of Java List.hashCode()
                    // at
                    // http://docs.oracle.com/javase/6/docs/api/java/util/List.html#hashCode()
                    // and adding deleting to clear up as we go
                    int effectsHashCode = 1;
                    for (auto it = effects->begin(); it != effects->end();
                         ++it) {
                        MobEffectInstance* mei = *it;
                        effectsHashCode =
                            31 * effectsHashCode +
                            (mei == nullptr ? 0 : mei->hashCode());
                        delete (*it);
                    }

                    bool toAdd = true;
                    for (auto it = s_uniquePotionValues.begin();
                         it != s_uniquePotionValues.end(); ++it) {
                        // Some potions hash the same (identical effects) but
                        // are throwable so account for that
                        if (it->first == effectsHashCode &&
                            !(!isThrowable(it->second) && isThrowable(brew))) {
                            toAdd = false;
                            break;
                        }
                    }
                    if (toAdd) {
                        s_uniquePotionValues.push_back(
                            std::pair<int, int>(effectsHashCode, brew));
                    }
                }
                delete effects;
            }
        }
    }
    return &s_uniquePotionValues;
}