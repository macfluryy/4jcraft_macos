#include <unordered_map>

#include "DummyCriteria.h"
#include "minecraft/world/scores/criteria/ObjectiveCriteria.h"

DummyCriteria::DummyCriteria(const std::wstring& name) {
    this->name = name;
    ObjectiveCriteria::CRITERIA_BY_NAME[name] = this;
}

std::wstring DummyCriteria::getName() { return name; }

int DummyCriteria::getScoreModifier(
    std::vector<std::shared_ptr<Player> >* players) {
    return 0;
}

bool DummyCriteria::isReadOnly() { return false; }