#pragma once

#include <memory>
#include <string>

#include "GameRuleDefinition.h"
#include "Minecraft.Client/Common/src/GameRules/ConsoleGameRulesConstants.h"

class ItemInstance;

class AddEnchantmentRuleDefinition : public GameRuleDefinition {
private:
    int m_enchantmentId;
    int m_enchantmentLevel;

public:
    AddEnchantmentRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType() {
        return ConsoleGameRules::eGameRuleType_AddEnchantment;
    }

    virtual void writeAttributes(DataOutputStream*, unsigned int numAttrs);

    virtual void addAttribute(const std::wstring& attributeName,
                              const std::wstring& attributeValue);

    bool enchantItem(std::shared_ptr<ItemInstance> item);
};