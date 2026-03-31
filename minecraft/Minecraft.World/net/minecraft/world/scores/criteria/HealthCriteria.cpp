#include <memory>
#include <vector>

#include "HealthCriteria.h"
#include "Minecraft.World/net/minecraft/util/Mth.h"
#include "Minecraft.World/net/minecraft/world/entity/player/Player.h"
#include "Minecraft.World/net/minecraft/world/scores/criteria/DummyCriteria.h"

HealthCriteria::HealthCriteria(const std::wstring& id) : DummyCriteria(id) {}

int HealthCriteria::getScoreModifier(
    std::vector<std::shared_ptr<Player> >* players) {
    float health = 0;

    for (auto it = players->begin(); it != players->end(); ++it) {
        std::shared_ptr<Player> player = *it;
        health += player->getHealth() + player->getAbsorptionAmount();
    }

    if (players->size() > 0) health /= players->size();

    return Mth::ceil(health);
}

bool HealthCriteria::isReadOnly() { return true; }