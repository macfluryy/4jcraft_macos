#include "Objective.h"
#include "Minecraft.World/net/minecraft/world/scores/Scoreboard.h"

Objective::Objective(Scoreboard* scoreboard, const std::wstring& name,
                     ObjectiveCriteria* criteria) {
    this->scoreboard = scoreboard;
    this->name = name;
    this->criteria = criteria;

    displayName = name;
}

Scoreboard* Objective::getScoreboard() { return scoreboard; }

std::wstring Objective::getName() { return name; }

ObjectiveCriteria* Objective::getCriteria() { return criteria; }

std::wstring Objective::getDisplayName() { return displayName; }

void Objective::setDisplayName(const std::wstring& name) {
    displayName = name;
    scoreboard->onObjectiveChanged(this);
}