#include "../Platform/stdafx.h"
#include "../Headers/net.minecraft.world.entity.h"
#include "../Headers/net.minecraft.world.level.h"
#include "../Headers/net.minecraft.world.level.tile.h"
#include "../Headers/net.minecraft.world.phys.h"
#include "../Headers/net.minecraft.world.damagesource.h"
#include "CombatTracker.h"

CombatTracker::CombatTracker(LivingEntity* mob) { this->mob = mob; }

CombatTracker::~CombatTracker() {
    for (AUTO_VAR(it, entries.begin()); it != entries.end(); ++it) {
        delete (*it);
    }
}

void CombatTracker::prepareForDamage() {
    resetPreparedStatus();

    if (mob->onLadder()) {
        int type = mob->level->getTile(
            Mth::floor(mob->x), Mth::floor(mob->bb->y0), Mth::floor(mob->z));

        if (type == Tile::ladder->id) {
            nextLocation = eLocation_LADDER;
        } else if (type == Tile::vine->id) {
            nextLocation = eLocation_VINES;
        }
    } else if (mob->isInWater()) {
        nextLocation = eLocation_WATER;
    }
}

void CombatTracker::recordDamage(DamageSource* source, float health,
                                 float damage) {
    recheckStatus();
    prepareForDamage();

    CombatEntry* entry = new CombatEntry(source, mob->tickCount, health, damage,
                                         nextLocation, mob->fallDistance);

    entries.push_back(entry);
    lastDamageTime = mob->tickCount;
    takingDamage = true;
    inCombat |= entry->isCombatRelated();
}

std::shared_ptr<ChatPacket> CombatTracker::getDeathMessagePacket() {
    if (entries.size() == 0)
        return std::shared_ptr<ChatPacket>(
            new ChatPacket(mob->getNetworkName()));

    CombatEntry* knockOffEntry = getMostSignificantFall();
    CombatEntry* killingBlow = entries[entries.size() - 1];

    std::shared_ptr<ChatPacket> result;

    std::shared_ptr<Entity> killingEntity =
        killingBlow->getSource()->getEntity();

    if (knockOffEntry != NULL &&
        killingBlow->getSource()->equals(DamageSource::fall)) {
        std::shared_ptr<Entity> attackerEntity =
            knockOffEntry->getSource()->getEntity();

        if (knockOffEntry->getSource()->equals(DamageSource::fall) ||
            knockOffEntry->getSource()->equals(DamageSource::outOfWorld)) {
            ChatPacket::EChatPacketMessage message =
                ChatPacket::e_ChatDeathFellAccidentGeneric;

            switch (getFallLocation(knockOffEntry)) {
                case eLocation_GENERIC:
                    message = ChatPacket::e_ChatDeathFellAccidentGeneric;
                    break;
                case eLocation_LADDER:
                    message = ChatPacket::e_ChatDeathFellAccidentLadder;
                    break;
                case eLocation_VINES:
                    message = ChatPacket::e_ChatDeathFellAccidentVines;
                    break;
                case eLocation_WATER:
                    message = ChatPacket::e_ChatDeathFellAccidentWater;
                    break;
                case eLocation_COUNT:
                    break;
            }

            result = std::shared_ptr<ChatPacket>(
                new ChatPacket(mob->getNetworkName(), message));
        } else if (attackerEntity != NULL &&
                   (killingEntity == NULL || attackerEntity != killingEntity)) {
            std::shared_ptr<ItemInstance> attackerItem =
                attackerEntity->instanceof
                (eTYPE_LIVINGENTITY)
                    ? std::dynamic_pointer_cast<LivingEntity>(attackerEntity)
                          ->getCarriedItem()
                    : nullptr;

            if (attackerItem != NULL && attackerItem->hasCustomHoverName()) {
                result = std::shared_ptr<ChatPacket>(new ChatPacket(
                    mob->getNetworkName(),
                    ChatPacket::e_ChatDeathFellAssistItem,
                    attackerEntity->GetType(), attackerEntity->getNetworkName(),
                    attackerItem->getHoverName()));
            } else {
                result = std::shared_ptr<ChatPacket>(new ChatPacket(
                    mob->getNetworkName(), ChatPacket::e_ChatDeathFellAssist,
                    attackerEntity->GetType(),
                    attackerEntity->getNetworkName()));
            }
        } else if (killingEntity != NULL) {
            std::shared_ptr<ItemInstance> killerItem = killingEntity->instanceof
                (eTYPE_LIVINGENTITY)
                    ? std::dynamic_pointer_cast<LivingEntity>(killingEntity)
                          ->getCarriedItem()
                    : nullptr;
            if (killerItem != NULL && killerItem->hasCustomHoverName()) {
                result = std::shared_ptr<ChatPacket>(new ChatPacket(
                    mob->getNetworkName(),
                    ChatPacket::e_ChatDeathFellFinishItem,
                    killingEntity->GetType(), killingEntity->getNetworkName(),
                    killerItem->getHoverName()));
            } else {
                result = std::shared_ptr<ChatPacket>(new ChatPacket(
                    mob->getNetworkName(), ChatPacket::e_ChatDeathFellFinish,
                    killingEntity->GetType(), killingEntity->getNetworkName()));
            }
        } else {
            result = std::shared_ptr<ChatPacket>(new ChatPacket(
                mob->getNetworkName(), ChatPacket::e_ChatDeathFellKiller));
        }
    } else {
        result = killingBlow->getSource()->getDeathMessagePacket(
            std::dynamic_pointer_cast<LivingEntity>(mob->shared_from_this()));
    }

    return result;
}

std::shared_ptr<LivingEntity> CombatTracker::getKiller() {
    std::shared_ptr<LivingEntity> bestMob = nullptr;
    std::shared_ptr<Player> bestPlayer = nullptr;
    float bestMobDamage = 0;
    float bestPlayerDamage = 0;

    for (AUTO_VAR(it, entries.begin()); it != entries.end(); ++it) {
        CombatEntry* entry = *it;
        if (entry->getSource() != NULL &&
                entry->getSource()->getEntity() != NULL &&
                entry->getSource()->getEntity()->instanceof
            (eTYPE_PLAYER) &&
                (bestPlayer == NULL || entry->getDamage() > bestPlayerDamage)) {
            bestPlayerDamage = entry->getDamage();
            bestPlayer = std::dynamic_pointer_cast<Player>(
                entry->getSource()->getEntity());
        }

        if (entry->getSource() != NULL &&
                entry->getSource()->getEntity() != NULL &&
                entry->getSource()->getEntity()->instanceof
            (eTYPE_LIVINGENTITY) &&
                (bestMob == NULL || entry->getDamage() > bestMobDamage)) {
            bestMobDamage = entry->getDamage();
            bestMob = std::dynamic_pointer_cast<LivingEntity>(
                entry->getSource()->getEntity());
        }
    }

    if (bestPlayer != NULL && bestPlayerDamage >= bestMobDamage / 3) {
        return bestPlayer;
    } else {
        return bestMob;
    }
}

CombatEntry* CombatTracker::getMostSignificantFall() {
    CombatEntry* result = NULL;
    CombatEntry* alternative = NULL;
    int altDamage = 0;
    float bestFall = 0;

    for (int i = 0; i < entries.size(); i++) {
        CombatEntry* entry = entries.at(i);
        CombatEntry* previous = i > 0 ? entries.at(i - 1) : NULL;

        bool isFall = entry->getSource()->equals(DamageSource::fall);
        bool isOutOfWorld =
            entry->getSource()->equals(DamageSource::outOfWorld);

        if ((isFall || isOutOfWorld) && (entry->getFallDistance() > 0) &&
            (result == NULL || entry->getFallDistance() > bestFall)) {
            if (i > 0) {
                result = previous;
            } else {
                result = entry;
            }
            bestFall = entry->getFallDistance();
        }

        if (entry->getLocation() != eLocation_GENERIC &&
            (alternative == NULL || entry->getDamage() > altDamage)) {
            alternative = entry;
        }
    }

    if (bestFall > 5 && result != NULL) {
        return result;
    } else if (altDamage > 5 && alternative != NULL) {
        return alternative;
    } else {
        return NULL;
    }
}

CombatTracker::eLOCATION CombatTracker::getFallLocation(CombatEntry* entry) {
    return entry->getLocation();
}

bool CombatTracker::isTakingDamage() {
    recheckStatus();
    return takingDamage;
}

bool CombatTracker::isInCombat() {
    recheckStatus();
    return inCombat;
}

void CombatTracker::resetPreparedStatus() { nextLocation = eLocation_GENERIC; }

void CombatTracker::recheckStatus() {
    int reset = inCombat ? RESET_COMBAT_STATUS_TIME : RESET_DAMAGE_STATUS_TIME;

    if (takingDamage && mob->tickCount - lastDamageTime > reset) {
        for (AUTO_VAR(it, entries.begin()); it != entries.end(); ++it) {
            delete (*it);
        }
        entries.clear();
        takingDamage = false;
        inCombat = false;
    }
}
