#include "CompleteAllRuleDefinition.h"

#include <memory>
#include <unordered_map>
#include <utility>

#include "Minecraft.Client/Common/src/GameRules/LevelRules/RuleDefinitions/CompleteAllRuleDefinition.h"
#include "Minecraft.Client/Common/src/GameRules/LevelRules/RuleDefinitions/CompoundGameRuleDefinition.h"
#include "Minecraft.Client/Common/src/GameRules/LevelRules/RuleDefinitions/GameRuleDefinition.h"
#include "Minecraft.Client/Common/src/GameRules/LevelRules/Rules/GameRule.h"
#include "Minecraft.Client/Linux/Linux_App.h"
#include "console_helpers/StringHelpers.h"
#include "minecraft/network/Connection.h"
#include "minecraft/network/packet/UpdateGameRuleProgressPacket.h"

void CompleteAllRuleDefinition::getChildren(
    std::vector<GameRuleDefinition*>* children) {
    CompoundGameRuleDefinition::getChildren(children);
}

bool CompleteAllRuleDefinition::onUseTile(GameRule* rule, int tileId, int x,
                                          int y, int z) {
    bool statusChanged =
        CompoundGameRuleDefinition::onUseTile(rule, tileId, x, y, z);
    if (statusChanged) updateStatus(rule);
    return statusChanged;
}

bool CompleteAllRuleDefinition::onCollectItem(
    GameRule* rule, std::shared_ptr<ItemInstance> item) {
    bool statusChanged = CompoundGameRuleDefinition::onCollectItem(rule, item);
    if (statusChanged) updateStatus(rule);
    return statusChanged;
}

void CompleteAllRuleDefinition::updateStatus(GameRule* rule) {
    int goal = 0;
    int progress = 0;
    for (auto it = rule->m_parameters.begin(); it != rule->m_parameters.end();
         ++it) {
        if (it->second.isPointer) {
            goal += it->second.gr->getGameRuleDefinition()->getGoal();
            progress += it->second.gr->getGameRuleDefinition()->getProgress(
                it->second.gr);
        }
    }
    if (rule->getConnection() != nullptr) {
        PacketData data;
        data.goal = goal;
        data.progress = progress;

        int icon = -1;
        int auxValue = 0;

        if (m_lastRuleStatusChanged != nullptr) {
            icon = m_lastRuleStatusChanged->getIcon();
            auxValue = m_lastRuleStatusChanged->getAuxValue();
            m_lastRuleStatusChanged = nullptr;
        }
        rule->getConnection()->send(
            std::shared_ptr<UpdateGameRuleProgressPacket>(
                new UpdateGameRuleProgressPacket(
                    getActionType(), this->m_descriptionId, icon, auxValue, 0,
                    &data, sizeof(PacketData))));
    }
    app.DebugPrintf("Updated CompleteAllRule - Completed %d of %d\n", progress,
                    goal);
}

std::wstring CompleteAllRuleDefinition::generateDescriptionString(
    const std::wstring& description, void* data, int dataLength) {
    PacketData* values = (PacketData*)data;
    std::wstring newDesc = description;
    newDesc =
        replaceAll(newDesc, L"{*progress*}", _toString<int>(values->progress));
    newDesc = replaceAll(newDesc, L"{*goal*}", _toString<int>(values->goal));
    return newDesc;
}