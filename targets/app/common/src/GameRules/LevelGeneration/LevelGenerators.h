#pragma once

#include <format>
#include <string>
#include <vector>


class LevelGenerationOptions;

class LevelGenerators {
private:
    std::vector<LevelGenerationOptions*> m_levelGenerators;

public:
    LevelGenerators();

    void addLevelGenerator(const std::wstring& displayName,
                           LevelGenerationOptions* generator);
    void removeLevelGenerator(LevelGenerationOptions* generator);

    std::vector<LevelGenerationOptions*>* getLevelGenerators() {
        return &m_levelGenerators;
    }
};