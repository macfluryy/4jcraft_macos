#include "CollectItemRuleDefinition.h"

#include "Minecraft.Client/Common/src/GameRules/LevelRules/Rules/GameRule.h"
#include "Minecraft.Client/Linux/Linux_App.h"
#include "console_helpers/StringHelpers.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "minecraft/network/Connection.h"
#include "minecraft/network/packet/UpdateGameRuleProgressPacket.h"
#include "minecraft/world/item/ItemInstance.h"
#include "Minecraft.Client/Common/src/GameRules/ConsoleGameRulesConstants.h"
#include "Minecraft.Client/Common/src/GameRules/LevelRules/RuleDefinitions/GameRuleDefinition.h"
#include "Minecraft.Client/Common/src/GameRules/LevelRules/Rules/GameRulesInstance.h"

CollectItemRuleDefinition::CollectItemRuleDefinition() {
    m_itemId = 0;
    m_auxValue = 0;
    m_quantity = 0;
}

CollectItemRuleDefinition::~CollectItemRuleDefinition() {}

void CollectItemRuleDefinition::writeAttributes(DataOutputStream* dos,
                                                unsigned int numAttributes) {
    GameRuleDefinition::writeAttributes(dos, numAttributes + 3);

    ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_itemId);
    dos->writeUTF(_toString(m_itemId));

    ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_auxValue);
    dos->writeUTF(_toString(m_auxValue));

    ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_quantity);
    dos->writeUTF(_toString(m_quantity));
}

void CollectItemRuleDefinition::addAttribute(
    const std::wstring& attributeName, const std::wstring& attributeValue) {
    if (attributeName.compare(L"itemId") == 0) {
        m_itemId = _fromString<int>(attributeValue);
        app.DebugPrintf("CollectItemRule: Adding parameter itemId=%d\n",
                        m_itemId);
    } else if (attributeName.compare(L"auxValue") == 0) {
        m_auxValue = _fromString<int>(attributeValue);
        app.DebugPrintf("CollectItemRule: Adding parameter m_auxValue=%d\n",
                        m_auxValue);
    } else if (attributeName.compare(L"quantity") == 0) {
        m_quantity = _fromString<int>(attributeValue);
        app.DebugPrintf("CollectItemRule: Adding parameter m_quantity=%d\n",
                        m_quantity);
    } else {
        GameRuleDefinition::addAttribute(attributeName, attributeValue);
    }
}

int CollectItemRuleDefinition::getGoal() { return m_quantity; }

int CollectItemRuleDefinition::getProgress(GameRule* rule) {
    GameRule::ValueType value = rule->getParameter(L"iQuantity");
    return value.i;
}

void CollectItemRuleDefinition::populateGameRule(
    GameRulesInstance::EGameRulesInstanceType type, GameRule* rule) {
    GameRule::ValueType value;
    value.i = 0;
    rule->setParameter(L"iQuantity", value);

    GameRuleDefinition::populateGameRule(type, rule);
}

bool CollectItemRuleDefinition::onCollectItem(
    GameRule* rule, std::shared_ptr<ItemInstance> item) {
    bool statusChanged = false;
    if (item != nullptr && item->id == m_itemId &&
        item->getAuxValue() == m_auxValue &&
        item->get4JData() == m_4JDataValue) {
        if (!getComplete(rule)) {
            GameRule::ValueType value = rule->getParameter(L"iQuantity");
            int quantityCollected = (value.i += item->count);
            rule->setParameter(L"iQuantity", value);

            statusChanged = true;

            if (quantityCollected >= m_quantity) {
                setComplete(rule, true);
                app.DebugPrintf(
                    "Completed CollectItemRule with info - itemId:%d, "
                    "auxValue:%d, quantity:%d, dataTag:%d\n",
                    m_itemId, m_auxValue, m_quantity, m_4JDataValue);

                if (rule->getConnection() != nullptr) {
                    rule->getConnection()->send(
                        std::shared_ptr<UpdateGameRuleProgressPacket>(
                            new UpdateGameRuleProgressPacket(
                                getActionType(), this->m_descriptionId,
                                m_itemId, m_auxValue, this->m_4JDataValue,
                                nullptr, 0)));
                }
            }
        }
    }
    return statusChanged;
}

std::wstring CollectItemRuleDefinition::generateXml(
    std::shared_ptr<ItemInstance> item) {
    // 4J Stu - This should be kept in sync with the GameRulesDefinition.xsd
    std::wstring xml = L"";
    if (item != nullptr) {
        xml = L"<CollectItemRule itemId=\"" + _toString<int>(item->id) +
              L"\" quantity=\"SET\" descriptionName=\"OPTIONAL\" "
              L"promptName=\"OPTIONAL\"";
        if (item->getAuxValue() != 0)
            xml +=
                L" auxValue=\"" + _toString<int>(item->getAuxValue()) + L"\"";
        if (item->get4JData() != 0)
            xml += L" dataTag=\"" + _toString<int>(item->get4JData()) + L"\"";
        xml += L"/>\n";
    }
    return xml;
}