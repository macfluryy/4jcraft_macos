#pragma once

#include <vector>

#include "java/Random.h"

class Random;




class WeighedRandomItem {
    friend class WeighedRandom;

protected:
    int randomWeight;

public:
    WeighedRandomItem(int randomWeight) { this->randomWeight = randomWeight; }
};

class WeighedRandom {
public:
    
    static int getTotalWeight(std::vector<WeighedRandomItem*>* items);
    static WeighedRandomItem* getRandomItem(
        Random* random, std::vector<WeighedRandomItem*>* items,
        int totalWeight);
    static WeighedRandomItem* getRandomItem(
        Random* random, std::vector<WeighedRandomItem*>* items);
    static int getTotalWeight(const std::vector<WeighedRandomItem*>& items);
    static WeighedRandomItem* getRandomItem(
        Random* random, const std::vector<WeighedRandomItem*>& items,
        int totalWeight);
    static WeighedRandomItem* getRandomItem(
        Random* random, const std::vector<WeighedRandomItem*>& items);
};
