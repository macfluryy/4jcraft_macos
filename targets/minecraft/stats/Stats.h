#pragma once

#include <format>
#include <unordered_map>
#include <vector>

#include "Stat.h"

class ItemStat;
class Stat;

class Stats {
    friend class Stat;

private:
    static const int BLOCKS_MINED_OFFSET;
    static const int ITEMS_COLLECTED_OFFSET;
    static const int ITEMS_CRAFTED_OFFSET;
    static const int ADDITIONAL_STATS_OFFSET;

protected:
    static std::unordered_map<int, Stat*>* statsById;

public:
    static std::vector<Stat*>* all;
    static std::vector<Stat*>* generalStats;
    static std::vector<ItemStat*>* blocksMinedStats;
    static std::vector<ItemStat*>* itemsCollectedStats;
    static std::vector<ItemStat*>* itemsCraftedStats;

#if (defined _EXTENDED_ACHIEVEMENTS) && (!0)
    static std::vector<ItemStat*>* blocksPlacedStats;
#endif

    static Stat* walkOneM;
    static Stat* swimOneM;
    static Stat* fallOneM;
    static Stat* climbOneM;
    static Stat* minecartOneM;
    static Stat* boatOneM;
    static Stat* pigOneM;
    static Stat* portalsCreated;
    static Stat* cowsMilked;
    static Stat* netherLavaCollected;

    static Stat* killsZombie;
    static Stat* killsSkeleton;
    static Stat* killsCreeper;
    static Stat* killsSpider;
    static Stat* killsSpiderJockey;
    static Stat* killsZombiePigman;
    static Stat* killsSlime;
    static Stat* killsGhast;
    static Stat* killsNetherZombiePigman;

    
    static Stat* befriendsWolf;
    static Stat* totalBlocksMined;
    static Stat*
        timePlayed;  
                     
                     

    
    static std::vector<Stat*> blocksMined;
    static std::vector<Stat*> itemsCollected;
    static std::vector<Stat*> itemsCrafted;

#if (defined _EXTENDED_ACHIEVEMENTS) && (!0)
    static std::vector<Stat*> blocksPlaced;
#endif

    
    static Stat* killsEnderdragon;  
                                    
    static Stat* completeTheEnd;    
                                    

#if (defined _EXTENDED_ACHIEVEMENTS) && (!0)
    static std::vector<Stat*> biomesVisisted;
    static std::vector<Stat*> rainbowCollection;
#endif

    static void staticCtor();

    static void init();

private:
    static bool blockStatsLoaded;
    static bool itemStatsLoaded;
    static bool craftableStatsLoaded;

public:
    static void buildBlockStats();
    static void buildItemStats();
    static void buildCraftableStats();

    
    
    static void buildAdditionalStats();

    static Stat* get(int key);
};
