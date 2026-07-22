#include "MobCategory.h"

#include <vector>

#include "minecraft/world/level/material/Material.h"

MobCategory* MobCategory::monster = nullptr;
MobCategory* MobCategory::creature = nullptr;
MobCategory* MobCategory::ambient = nullptr;
MobCategory* MobCategory::waterCreature = nullptr;

MobCategory* MobCategory::creature_wolf = nullptr;
MobCategory* MobCategory::creature_chicken = nullptr;
MobCategory* MobCategory::creature_mushroomcow = nullptr;

std::vector<MobCategory*> MobCategory::values = std::vector<MobCategory*>(7);

void MobCategory::staticCtor() {
    
    
    monster = new MobCategory(70, Material::air, false, false, eTYPE_MONSTER,
                              false, CONSOLE_MONSTERS_HARD_LIMIT);
    creature = new MobCategory(10, Material::air, true, true,
                               eTYPE_ANIMALS_SPAWN_LIMIT_CHECK, false,
                               CONSOLE_ANIMALS_HARD_LIMIT);
    ambient = new MobCategory(15, Material::air, true, false, eTYPE_AMBIENT,
                              false, CONSOLE_AMBIENT_HARD_LIMIT),
    waterCreature =
        new MobCategory(5, Material::water, true, false, eTYPE_WATERANIMAL,
                        false, CONSOLE_SQUID_HARD_LIMIT);

    values[0] = monster;
    values[1] = creature;
    values[2] = ambient;
    values[3] = waterCreature;
    
    
    creature_wolf = new MobCategory(3, Material::air, true, true, eTYPE_WOLF,
                                    true, MAX_XBOX_WOLVES);
    creature_chicken = new MobCategory(2, Material::air, true, true,
                                       eTYPE_CHICKEN, true, MAX_XBOX_CHICKENS);
    creature_mushroomcow =
        new MobCategory(2, Material::air, true, true, eTYPE_MUSHROOMCOW, true,
                        MAX_XBOX_MUSHROOMCOWS);
    values[4] = creature_wolf;
    values[5] = creature_chicken;
    values[6] = creature_mushroomcow;
}

MobCategory::MobCategory(int maxVar, Material* spawnPositionMaterial,
                         bool isFriendly, bool isPersistent, eINSTANCEOF eBase,
                         bool isSingleType, int maxPerLevel)
    : m_max(maxVar),
      spawnPositionMaterial(spawnPositionMaterial),
      m_isFriendly(isFriendly),
      m_isPersistent(isPersistent),
      m_eBase(eBase),
      m_isSingleType(isSingleType),
      m_maxPerLevel(maxPerLevel) {}


const eINSTANCEOF MobCategory::getEnumBaseClass() { return m_eBase; }

int MobCategory::getMaxInstancesPerChunk() { return m_max; }

int MobCategory::getMaxInstancesPerLevel()  
{
    return m_maxPerLevel;
}

Material* MobCategory::getSpawnPositionMaterial() {
    return (Material*)spawnPositionMaterial;
}

bool MobCategory::isFriendly() { return m_isFriendly; }

bool MobCategory::isSingleType() { return m_isSingleType; }

bool MobCategory::isPersistent() { return m_isPersistent; }
