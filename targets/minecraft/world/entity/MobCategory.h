#pragma once

#include <format>
#include <typeinfo>
#include <vector>

#include "java/Class.h"

class Material;

class MobCategory {
public:
    
    
    static const int CONSOLE_MONSTERS_HARD_LIMIT =
        50;  
             
    static const int CONSOLE_ANIMALS_HARD_LIMIT =
        50;  
             
    static const int CONSOLE_AMBIENT_HARD_LIMIT = 20;  

    static const int MAX_XBOX_CHICKENS =
        8;  
    static const int MAX_XBOX_WOLVES =
        8;  
    static const int MAX_XBOX_MUSHROOMCOWS =
        2;  
    static const int MAX_XBOX_SNOWMEN =
        16;  
             
    static const int MAX_XBOX_IRONGOLEM =
        16;  
             
    static const int CONSOLE_SQUID_HARD_LIMIT = 5;
    static const int MAX_CONSOLE_BOSS =
        1;  

    static const int MAX_XBOX_ANIMALS_WITH_BREEDING =
        CONSOLE_ANIMALS_HARD_LIMIT + 20;  
                                          
    static const int MAX_XBOX_CHICKENS_WITH_BREEDING =
        MAX_XBOX_CHICKENS + 8;  
                                
    static const int MAX_XBOX_MUSHROOMCOWS_WITH_BREEDING =
        MAX_XBOX_MUSHROOMCOWS + 20;  
                                     
    static const int MAX_XBOX_WOLVES_WITH_BREEDING =
        MAX_XBOX_WOLVES + 8;  
                              
    static const int MAX_VILLAGERS_WITH_BREEDING = 35;

    static const int MAX_XBOX_ANIMALS_WITH_SPAWN_EGG =
        MAX_XBOX_ANIMALS_WITH_BREEDING + 20;
    static const int MAX_XBOX_CHICKENS_WITH_SPAWN_EGG =
        MAX_XBOX_CHICKENS_WITH_BREEDING + 10;
    static const int MAX_XBOX_WOLVES_WITH_SPAWN_EGG =
        MAX_XBOX_WOLVES_WITH_BREEDING + 10;
    static const int MAX_XBOX_MONSTERS_WITH_SPAWN_EGG =
        CONSOLE_MONSTERS_HARD_LIMIT + 20;
    static const int MAX_XBOX_VILLAGERS_WITH_SPAWN_EGG =
        MAX_VILLAGERS_WITH_BREEDING +
        15;  
    static const int MAX_XBOX_MUSHROOMCOWS_WITH_SPAWN_EGG =
        MAX_XBOX_MUSHROOMCOWS_WITH_BREEDING + 8;
    static const int MAX_XBOX_SQUIDS_WITH_SPAWN_EGG =
        CONSOLE_SQUID_HARD_LIMIT + 8;
    static const int MAX_AMBIENT_WITH_SPAWN_EGG =
        CONSOLE_AMBIENT_HARD_LIMIT + 8;

    














    static MobCategory* monster;
    static MobCategory* creature;
    static MobCategory* ambient;
    static MobCategory* waterCreature;
    
    
    static MobCategory* creature_wolf;
    static MobCategory* creature_chicken;
    static MobCategory* creature_mushroomcow;

    
    
    
    static std::vector<MobCategory*> values;

private:
    const int m_max;
    const int m_maxPerLevel;
    const Material* spawnPositionMaterial;
    const bool m_isFriendly;
    const bool m_isPersistent;
    const bool m_isSingleType;  
    const eINSTANCEOF m_eBase;  

    MobCategory(int maxVar, Material* spawnPositionMaterial, bool isFriendly,
                bool isPersistent, eINSTANCEOF eBase, bool isSingleType,
                int maxPerLevel);

public:
    const std::type_info getBaseClass();
    const eINSTANCEOF getEnumBaseClass();  
    int getMaxInstancesPerChunk();
    int getMaxInstancesPerLevel();  
    Material* getSpawnPositionMaterial();
    bool isFriendly();
    bool isSingleType();
    bool isPersistent();

public:
    static void staticCtor();
};
