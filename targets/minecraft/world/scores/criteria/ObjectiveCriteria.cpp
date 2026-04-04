#include "ObjectiveCriteria.h"

#include "minecraft/world/scores/criteria/DummyCriteria.h"
#include "minecraft/world/scores/criteria/HealthCriteria.h"

std::unordered_map<std::wstring, ObjectiveCriteria*>
    ObjectiveCriteria::CRITERIA_BY_NAME;

ObjectiveCriteria* ObjectiveCriteria::DUMMY = new DummyCriteria(L"dummy");
ObjectiveCriteria* ObjectiveCriteria::DEATH_COUNT =
    new DummyCriteria(L"deathCount");
ObjectiveCriteria* ObjectiveCriteria::KILL_COUNT_PLAYERS =
    new DummyCriteria(L"playerKillCount");
ObjectiveCriteria* ObjectiveCriteria::KILL_COUNT_ALL =
    new DummyCriteria(L"totalKillCount");
ObjectiveCriteria* ObjectiveCriteria::HEALTH = new HealthCriteria(L"health");