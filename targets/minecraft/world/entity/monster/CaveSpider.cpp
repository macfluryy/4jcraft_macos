#include "CaveSpider.h"

#include <memory>

#include "minecraft/SharedConstants.h"
#include "minecraft/world/Difficulty.h"
#include "minecraft/world/effect/MobEffect.h"
#include "minecraft/world/effect/MobEffectInstance.h"
#include "minecraft/world/entity/Entity.h"
#include "minecraft/world/entity/LivingEntity.h"
#include "minecraft/world/entity/ai/attributes/AttributeInstance.h"
#include "minecraft/world/entity/monster/SharedMonsterAttributes.h"
#include "minecraft/world/entity/monster/Spider.h"
#include "minecraft/world/level/Level.h"

CaveSpider::CaveSpider(Level* level) : Spider(level) {
    
    
    registerAttributes();

    this->setSize(0.7f, 0.5f);
}

void CaveSpider::registerAttributes() {
    Spider::registerAttributes();

    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(12);
}

bool CaveSpider::doHurtTarget(std::shared_ptr<Entity> target) {
    if (Spider::doHurtTarget(target)) {
        if (target->instanceof(eTYPE_LIVINGENTITY)) {
            int poisonTime = 0;
            if (level->difficulty <= Difficulty::EASY) {
                
            } else if (level->difficulty == Difficulty::NORMAL) {
                poisonTime = 7;
            } else if (level->difficulty == Difficulty::HARD) {
                poisonTime = 15;
            }

            if (poisonTime > 0) {
                std::dynamic_pointer_cast<LivingEntity>(target)->addEffect(
                    new MobEffectInstance(
                        MobEffect::poison->id,
                        poisonTime * SharedConstants::TICKS_PER_SECOND, 0));
            }
        }

        return true;
    }
    return false;
}

MobGroupData* CaveSpider::finalizeMobSpawn(
    MobGroupData* groupData, int extraData )  
{
    
    return groupData;
}