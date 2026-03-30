#include "../Platform/stdafx.h"
#include "Enemy.h"

EntitySelector* Enemy::ENEMY_SELECTOR = new Enemy::EnemyEntitySelector();

bool Enemy::EnemyEntitySelector::matches(std::shared_ptr<Entity> entity) const {
    return (entity != nullptr) && entity->instanceof(eTYPE_ENEMY);
}