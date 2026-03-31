#include "LevelRules.h"
#include "Minecraft.Client/Common/Source Files/GameRules/LevelRules/LevelRules.h"

LevelRules::LevelRules() {}

void LevelRules::addLevelRule(const std::wstring& displayName,
                              std::uint8_t* pbData, unsigned int dataLength) {}

void LevelRules::addLevelRule(const std::wstring& displayName,
                              LevelRuleset* rootRule) {}

void LevelRules::removeLevelRule(LevelRuleset* removing) {
    // TODO ?
}
