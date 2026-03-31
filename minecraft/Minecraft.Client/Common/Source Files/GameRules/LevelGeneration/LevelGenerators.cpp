#include <algorithm>

#include "LevelGenerationOptions.h"
#include "LevelGenerators.h"
#include "Minecraft.Client/Common/Source Files/GameRules/LevelGeneration/LevelGenerators.h"

LevelGenerators::LevelGenerators() {}

void LevelGenerators::addLevelGenerator(const std::wstring& displayName,
                                        LevelGenerationOptions* generator) {
    if (!displayName.empty()) generator->setDisplayName(displayName);
    m_levelGenerators.push_back(generator);
}

void LevelGenerators::removeLevelGenerator(LevelGenerationOptions* generator) {
    std::vector<LevelGenerationOptions*>::iterator it;
    while ((it = find(m_levelGenerators.begin(), m_levelGenerators.end(),
                      generator)) != m_levelGenerators.end()) {
        m_levelGenerators.erase(it);
    }
}