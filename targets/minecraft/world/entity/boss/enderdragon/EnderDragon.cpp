#include "EnderDragon.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>

#include "java/Random.h"
#include "minecraft/SharedConstants.h"
#include "minecraft/core/particles/ParticleTypes.h"
#include "minecraft/sounds/SoundTypes.h"
#include "minecraft/util/Mth.h"
#include "minecraft/world/damageSource/DamageSource.h"
#include "minecraft/world/entity/Entity.h"
#include "minecraft/world/entity/ExperienceOrb.h"
#include "minecraft/world/entity/SyncedEntityData.h"
#include "minecraft/world/entity/ai/attributes/AttributeInstance.h"
#include "minecraft/world/entity/boss/MultiEntityMob.h"
#include "minecraft/world/entity/boss/MultiEntityMobPart.h"
#include "minecraft/world/entity/boss/enderdragon/EnderCrystal.h"
#include "minecraft/world/entity/monster/SharedMonsterAttributes.h"
#include "minecraft/world/entity/player/Player.h"
#include "minecraft/world/entity/projectile/DragonFireball.h"
#include "minecraft/world/level/GameRules.h"
#include "minecraft/world/level/Level.h"
#include "minecraft/world/level/pathfinder/BinaryHeap.h"
#include "minecraft/world/level/pathfinder/Node.h"
#include "minecraft/world/level/pathfinder/Path.h"
#include "minecraft/world/level/tile/LevelEvent.h"
#include "minecraft/world/level/tile/Tile.h"
#include "minecraft/world/level/tile/entity/TheEndPortalTile.h"
#include "nbt/CompoundTag.h"

#define PRINT_DRAGON_STATE_CHANGE_MESSAGES 1


const int EnderDragon::CRYSTAL_COUNT = 8;
const int EnderDragon::FLAME_TICKS = 60;
const float EnderDragon::FLAME_ANGLE = 22.5f;
const int EnderDragon::FLAME_PASSES =
    4;  
const int EnderDragon::FLAME_FREQUENCY =
    2;  
        
const int EnderDragon::FLAME_RANGE = 10;

const int EnderDragon::ATTACK_TICKS =
    SharedConstants::TICKS_PER_SECOND * 2;  

const int EnderDragon::SITTING_ATTACK_Y_VIEW_RANGE =
    10;  
         
const int EnderDragon::SITTING_ATTACK_VIEW_RANGE = EnderDragon::FLAME_RANGE * 2;
const int EnderDragon::SITTING_ATTACK_RANGE = EnderDragon::FLAME_RANGE * 2;
const int EnderDragon::SITTING_POST_ATTACK_IDLE_TICKS = 40;
const int EnderDragon::SITTING_SCANNING_IDLE_TICKS = 100;
const int EnderDragon::SITTING_FLAME_ATTACKS_COUNT =
    4;  
        



const float EnderDragon::SITTING_ALLOWED_DAMAGE_PERCENTAGE = 0.25f;

void EnderDragon::_init() {
    
    
    this->defineSynchedData();
    registerAttributes();
    setHealth(getMaxHealth());

    xTarget = yTarget = zTarget = 0.0;
    posPointer = -1;
    oFlapTime = 0;
    flapTime = 0;
    newTarget = false;
    inWall = false;
    attackTarget = nullptr;
    dragonDeathTime = 0;
    nearestCrystal = nullptr;

    
    m_remainingCrystalsCount = CRYSTAL_COUNT;
    m_fireballCharge = 0;
    m_holdingPatternAngle = 0.0f;
    m_holdingPatternClockwise = true;
    setSynchedAction(e_EnderdragonAction_HoldingPattern);
    m_actionTicks = 0;
    m_sittingDamageReceived = 0;
    m_headYRot = 0.0;
    m_acidArea = AABB(-4, -10, -3, 6, 3, 3);
    m_flameAttacks = 0;

    for (int i = 0; i < positionsLength; i++) {
        positions[i][0] = 0;
        positions[i][1] = 0;
        positions[i][2] = 0;
    }

    m_nodes = new std::vector<Node*>(24);
    openSet = new BinaryHeap();
    m_currentPath = nullptr;
}

EnderDragon::EnderDragon(Level* level) : Mob(level) {
    _init();

    setSize(16, 8);

    noPhysics = true;
    fireImmune = true;

    yTarget = 100;

    m_iGrowlTimer = 100;

    noCulling = true;
}


void EnderDragon::AddParts() {
    head = std::make_shared<MultiEntityMobPart>(
        std::dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"head",
        6, 6);
    neck = std::make_shared<MultiEntityMobPart>(
        std::dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"neck",
        6,
        6);  
    body = std::make_shared<MultiEntityMobPart>(
        std::dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"body",
        8, 8);
    tail1 = std::make_shared<MultiEntityMobPart>(
        std::dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"tail",
        4, 4);
    tail2 = std::make_shared<MultiEntityMobPart>(
        std::dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"tail",
        4, 4);
    tail3 = std::make_shared<MultiEntityMobPart>(
        std::dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"tail",
        4, 4);
    wing1 = std::make_shared<MultiEntityMobPart>(
        std::dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"wing",
        4, 4);
    wing2 = std::make_shared<MultiEntityMobPart>(
        std::dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"wing",
        4, 4);

    subEntities.push_back(head);
    subEntities.push_back(neck);  
    subEntities.push_back(body);
    subEntities.push_back(tail1);
    subEntities.push_back(tail2);
    subEntities.push_back(tail3);
    subEntities.push_back(wing1);
    subEntities.push_back(wing2);
}

EnderDragon::~EnderDragon() {
    if (m_nodes != nullptr) {
        for (unsigned int i = 0; i < m_nodes->size(); ++i) {
            if ((*m_nodes)[i] != nullptr) delete (*m_nodes)[i];
        }
        delete m_nodes;
    }
    delete openSet;
    if (m_currentPath != nullptr) delete m_currentPath;
}

void EnderDragon::registerAttributes() {
    Mob::registerAttributes();

    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(200);
}

void EnderDragon::defineSynchedData() {
    Mob::defineSynchedData();

    
    entityData->define(DATA_ID_SYNCHED_ACTION,
                       e_EnderdragonAction_HoldingPattern);
}

void EnderDragon::getLatencyPos(std::vector<double>& result, int step,
                                float a) {
    if (getHealth() <= 0) {
        a = 0;
    }

    a = 1 - a;

    int p0 = (posPointer - step * 1) & 63;
    int p1 = (posPointer - step * 1 - 1) & 63;

    
    
    
    

    double yr0 = positions[p0][0];
    double yrd = Mth::wrapDegrees(positions[p1][0] - yr0);
    result[0] = yr0 + yrd * a;

    yr0 = positions[p0][1];
    yrd = positions[p1][1] - yr0;

    result[1] = yr0 + yrd * a;
    result[2] = positions[p0][2] + (positions[p1][2] - positions[p0][2]) * a;
}

void EnderDragon::aiStep() {
    if (level->isClientSide) {
        
        
        
        
        setHealth(getHealth());

        float flap = cosf(flapTime * std::numbers::pi * 2);
        float oldFlap = cosf(oFlapTime * std::numbers::pi * 2);

        if (oldFlap <= -0.3f && flap >= -0.3f) {
            level->playLocalSound(x, y, z, eSoundType_MOB_ENDERDRAGON_MOVE, 1,
                                  0.8f + random->nextFloat() * .3f, false,
                                  100.0f);
        }
        
        if (!(getSynchedAction() == e_EnderdragonAction_Sitting_Flaming ||
              getSynchedAction() == e_EnderdragonAction_Sitting_Scanning ||
              getSynchedAction() == e_EnderdragonAction_Sitting_Attacking)) {
            m_iGrowlTimer--;
            if (m_iGrowlTimer < 0) {
                level->playLocalSound(x, y, z, eSoundType_MOB_ENDERDRAGON_GROWL,
                                      0.5f, 0.8f + random->nextFloat() * .3f,
                                      false, 100.0f);
                m_iGrowlTimer = 200 + (random->nextInt(200));
            }
        }
    }

    oFlapTime = flapTime;

    if (getHealth() <= 0) {
        
        
        
        float xo = (random->nextFloat() - 0.5f) * 8;
        float yo = (random->nextFloat() - 0.5f) * 4;
        float zo = (random->nextFloat() - 0.5f) * 8;
        level->addParticle(eParticleType_largeexplode, x + xo, y + 2 + yo,
                           z + zo, 0, 0, 0);
        return;
    }

    checkCrystals();

    float flapSpeed = 0.2f / (sqrt(xd * xd + zd * zd) * 10.0f + 1);
    flapSpeed *= (float)pow(2.0, yd);
    if (getSynchedAction() == e_EnderdragonAction_Sitting_Flaming ||
        getSynchedAction() == e_EnderdragonAction_Sitting_Scanning ||
        getSynchedAction() == e_EnderdragonAction_Sitting_Attacking) {
        
        
        flapTime += 0.1f;
    } else if (inWall) {
        flapTime += flapSpeed * 0.5f;
    } else {
        flapTime += flapSpeed;
    }

    yRot = Mth::wrapDegrees(yRot);

    if (posPointer < 0) {
        for (int i = 0; i < positionsLength; i++) {
            positions[i][0] = yRot;
            positions[i][1] = y;
        }
    }

    if (++posPointer == positionsLength) posPointer = 0;
    positions[posPointer][0] = yRot;
    positions[posPointer][1] = y;

    if (level->isClientSide) {
        if (lSteps > 0) {
            double xt = x + (lx - x) / lSteps;
            double yt = y + (ly - y) / lSteps;
            double zt = z + (lz - z) / lSteps;

            
            
            
            
            
            
            
            
            

            
            
            
            {
                double yrd = Mth::wrapDegrees(lyr - yRot);

                m_headYRot = 0.0;
                yRot += (yrd) / lSteps;
            }
            xRot += (lxr - xRot) / lSteps;

            lSteps--;
            this->setPos(xt, yt, zt);
            this->setRot(yRot, xRot);

            






        }

        if (getSynchedAction() == e_EnderdragonAction_Landing ||
            (getSynchedAction() == e_EnderdragonAction_Sitting_Flaming &&
             tickCount % 2 == 0)) {
            double xP = 0.0;
            double yP = 0.0;
            double zP = 0.0;
            Vec3 v = getHeadLookVector(1);  
            
            
            
            {
                Vec3 vN = Vec3{v.x, v.y, v.z}.normalize();
                vN.yRot(-std::numbers::pi / 4);

                for (unsigned int i = 0; i < 8; ++i) {
                    if (getSynchedAction() == e_EnderdragonAction_Landing) {
                        
                        {
                            xP = head->x;  
                            yP =
                                head->bb.y0 +
                                head->bbHeight /
                                    2;  
                                        
                            zP = head->z;  
                            xP += (level->random->nextBoolean() ? 1 : -1) *
                                  level->random->nextFloat() / 2;
                            yP += (level->random->nextBoolean() ? 1 : -1) *
                                  level->random->nextFloat() / 2;
                            zP += (level->random->nextBoolean() ? 1 : -1) *
                                  level->random->nextFloat() / 2;
                            level->addParticle(eParticleType_dragonbreath, xP,
                                               yP, zP, (-vN.x * 0.08) + xd,
                                               (-vN.y * 0.3) + yd,
                                               (-vN.z * 0.08) + zd);
                        }
                    } else {
                        double yVelocity = 0.6;
                        double xzVelocity = 0.08;
                        for (unsigned int j = 0; j < 6; ++j) {
                            xP = head->x;  
                            yP =
                                head->bb.y0 +
                                head->bbHeight /
                                    2;  
                                        
                            zP = head->z;  
                            xP += (level->random->nextBoolean() ? 1 : -1) *
                                  level->random->nextFloat() / 2;
                            yP += (level->random->nextBoolean() ? 1 : -1) *
                                  level->random->nextFloat() / 2;
                            zP += (level->random->nextBoolean() ? 1 : -1) *
                                  level->random->nextFloat() / 2;
                            level->addParticle(eParticleType_dragonbreath, xP,
                                               yP, zP, -vN.x * xzVelocity * j,
                                               -vN.y * yVelocity,
                                               -vN.z * xzVelocity * j);
                        }
                    }
                    vN.yRot(std::numbers::pi / (2 * 8));
                }
            }
        } else if (getSynchedAction() ==
                   e_EnderdragonAction_Sitting_Attacking) {
            
            
            level->playLocalSound(x, y, z, eSoundType_MOB_ENDERDRAGON_GROWL,
                                  0.5f, 0.8f + random->nextFloat() * .3f, false,
                                  100.0f);
        }
    } else {
        double xdd = xTarget - x;
        double ydd = yTarget - y;
        double zdd = zTarget - z;

        double dist = xdd * xdd + ydd * ydd + zdd * zdd;

        if (getSynchedAction() == e_EnderdragonAction_Sitting_Flaming) {
            --m_actionTicks;
            if (m_actionTicks <= 0) {
                if (m_flameAttacks >= SITTING_FLAME_ATTACKS_COUNT) {
                    setSynchedAction(e_EnderdragonAction_Takeoff);
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
                    app.DebugPrintf("Dragon action is now: Takeoff\n");
#endif
                    newTarget = true;
                } else {
                    setSynchedAction(e_EnderdragonAction_Sitting_Scanning);
                    attackTarget = level->getNearestPlayer(
                        shared_from_this(), SITTING_ATTACK_VIEW_RANGE,
                        SITTING_ATTACK_Y_VIEW_RANGE);
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
                    app.DebugPrintf("Dragon action is now: SittingScanning\n");
#endif
                }
            }
        } else if (getSynchedAction() == e_EnderdragonAction_Sitting_Scanning) {
            attackTarget = level->getNearestPlayer(shared_from_this(),
                                                   SITTING_ATTACK_VIEW_RANGE,
                                                   SITTING_ATTACK_Y_VIEW_RANGE);

            ++m_actionTicks;
            if (attackTarget != nullptr) {
                if (m_actionTicks > SITTING_SCANNING_IDLE_TICKS / 4) {
                    setSynchedAction(e_EnderdragonAction_Sitting_Attacking);
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
                    app.DebugPrintf("Dragon action is now: SittingAttacking\n");
#endif
                    m_actionTicks = ATTACK_TICKS;
                }
            } else {
                if (m_actionTicks >= SITTING_SCANNING_IDLE_TICKS) {
                    setSynchedAction(e_EnderdragonAction_Takeoff);
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
                    app.DebugPrintf("Dragon action is now: Takeoff\n");
#endif
                    newTarget = true;
                }
            }
        } else if (getSynchedAction() ==
                   e_EnderdragonAction_Sitting_Attacking) {
            --m_actionTicks;
            if (m_actionTicks <= 0) {
                ++m_flameAttacks;
                setSynchedAction(e_EnderdragonAction_Sitting_Flaming);
                attackTarget = level->getNearestPlayer(
                    shared_from_this(), SITTING_ATTACK_VIEW_RANGE,
                    SITTING_ATTACK_Y_VIEW_RANGE);
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
                app.DebugPrintf("Dragon action is now: SittingFlaming\n");
#endif
                m_actionTicks = FLAME_TICKS;
            }
        } else if (!newTarget &&
                   getSynchedAction() == e_EnderdragonAction_Takeoff) {
            int eggHeight = level->getTopSolidBlock(
                PODIUM_X_POS, PODIUM_Z_POS);  

            float dist = distanceToSqr(PODIUM_X_POS, eggHeight, PODIUM_Z_POS);
            if (dist > (10.0f * 10.0f)) {
                setSynchedAction(e_EnderdragonAction_HoldingPattern);
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
                app.DebugPrintf("Dragon action is now: HoldingPattern\n");
#endif
            }
        } else if (newTarget ||
                   ((getSynchedAction() != e_EnderdragonAction_Landing &&
                     dist < 10 * 10) ||
                    dist < 1) ||
                   dist > 150 * 150 || horizontalCollision ||
                   verticalCollision) {
            findNewTarget();
        }

        if (getSynchedAction() == e_EnderdragonAction_Sitting_Flaming ||
            getSynchedAction() == e_EnderdragonAction_Landing) {
            if (m_actionTicks < (FLAME_TICKS - 10)) {
                std::vector<std::shared_ptr<Entity> > targets;
                level->getEntities(shared_from_this(), &m_acidArea, targets);

                for (auto it = targets.begin(); it != targets.end(); ++it) {
                    if ((*it)->instanceof(eTYPE_LIVINGENTITY)) {
                        
                        std::shared_ptr<LivingEntity> e =
                            std::dynamic_pointer_cast<LivingEntity>(*it);
                        e->hurt(DamageSource::dragonbreath, 2);
                    }
                }
            }
        }
        if (getSynchedAction() == e_EnderdragonAction_Sitting_Flaming) {
            
        } else if (getSynchedAction() == e_EnderdragonAction_Sitting_Scanning) {
            if (attackTarget != nullptr) {
                Vec3 aim = Vec3((attackTarget->x - x), 0, (attackTarget->z - z))
                               .normalize();

                Vec3 dir = Vec3(sin(yRot * std::numbers::pi / 180), 0,
                                -cos(yRot * std::numbers::pi / 180))
                               .normalize();

                float dot = (float)dir.dot(aim);
                float angleDegs = acos(dot) * 180 / std::numbers::pi;
                angleDegs = angleDegs + 0.5f;

                if (angleDegs < 0 || angleDegs > 10) {
                    double xdd = attackTarget->x - head->x;
                    
                    
                    
                    double zdd = attackTarget->z - head->z;

                    double yRotT =
                        (180) - atan2(xdd, zdd) * 180 / std::numbers::pi;
                    double yRotD = Mth::wrapDegrees(yRotT - yRot);

                    if (yRotD > 50) yRotD = 50;
                    if (yRotD < -50) yRotD = -50;

                    double xd = xTarget - x;
                    double zd = zTarget - z;
                    yRotA *= 0.80f;

                    float rotSpeed = sqrt(xd * xd + zd * zd) * 1 + 1;
                    double distToTarget = sqrt(xd * xd + zd * zd) * 1 + 1;
                    if (distToTarget > 40) distToTarget = 40;
                    yRotA += yRotD * ((0.7f / distToTarget) / rotSpeed);
                    yRot += yRotA;
                } else {
                    
                }
            } else {
                
                
                
            }
        } else if (getSynchedAction() ==
                   e_EnderdragonAction_Sitting_Attacking) {
        } else {
            
            
            
            if (getSynchedAction() == e_EnderdragonAction_StrafePlayer &&
                attackTarget != nullptr && m_currentPath != nullptr &&
                m_currentPath->isDone()) {
                xTarget = attackTarget->x;
                zTarget = attackTarget->z;

                double xd = xTarget - x;
                double zd = zTarget - z;
                double sd = sqrt(xd * xd + zd * zd);
                double ho = 0.4f + sd / 80.0f - 1;
                if (ho > 10) ho = 10;
                yTarget = attackTarget->bb.y0 + ho;
            } else {
                
                
            }
            ydd = ydd / (sqrt(xdd * xdd + zdd * zdd));
            float max = 0.6f;
            if (getSynchedAction() == e_EnderdragonAction_Landing) max = 1.5f;
            if (ydd < -max) ydd = -max;
            if (ydd > max) ydd = max;
            yd += (ydd) * 0.1f;
            while (yRot < -180) yRot += 180 * 2;
            while (yRot >= 180) yRot -= 180 * 2;

            double yRotT = (180) - atan2(xdd, zdd) * 180 / std::numbers::pi;
            double yRotD = yRotT - yRot;
            while (yRotD < -180) yRotD += 180 * 2;
            while (yRotD >= 180) yRotD -= 180 * 2;

            if (yRotD > 50) yRotD = 50;
            if (yRotD < -50) yRotD = -50;

            Vec3 aim =
                Vec3((xTarget - x), (yTarget - y), (zTarget - z)).normalize();

            Vec3 dir = Vec3(sin(yRot * std::numbers::pi / 180), yd,
                            -cos(yRot * std::numbers::pi / 180))
                           .normalize();
            float dot = (float)(dir.dot(aim) + 0.5f) / 1.5f;
            if (dot < 0) dot = 0;

            yRotA *= 0.80f;

            float rotSpeed = sqrt(xd * xd + zd * zd) * 1 + 1;
            double distToTarget = sqrt(xd * xd + zd * zd) * 1 + 1;
            if (distToTarget > 40) distToTarget = 40;
            if (getSynchedAction() == e_EnderdragonAction_Landing) {
                yRotA += yRotD * (distToTarget / rotSpeed);
            } else {
                yRotA += yRotD * ((0.7f / distToTarget) / rotSpeed);
            }
            yRot += yRotA * 0.1f;

            float span = (float)(2.0f / (distToTarget + 1));
            float speed = 0.06f;
            moveRelative(0, -1, speed * (dot * span + (1 - span)));
            if (inWall) {
                move(xd * 0.8f, yd * 0.8f, zd * 0.8f);
            } else {
                move(xd, yd, zd);
            }

            Vec3 actual = Vec3(xd, yd, zd).normalize();
            float slide = (float)(actual.dot(dir) + 1) / 2.0f;
            slide = 0.8f + 0.15f * slide;

            xd *= slide;
            zd *= slide;
            yd *= 0.91f;
        }
    }

    yBodyRot = yRot;

    head->bbWidth = head->bbHeight =
        1;  
    neck->bbWidth = neck->bbHeight = 3;
    tail1->bbWidth = tail1->bbHeight = 2;
    tail2->bbWidth = tail2->bbHeight = 2;
    tail3->bbWidth = tail3->bbHeight = 2;
    body->bbHeight = 3;
    body->bbWidth = 5;
    wing1->bbHeight = 2;
    wing1->bbWidth = 4;
    wing2->bbHeight = 3;
    wing2->bbWidth = 4;

    
    
    
    
    
    

    
    
    float tilt = (float)getTilt(1) / 180.0f * std::numbers::pi;
    float ccTilt = cos(tilt);

    
    
    
    float ssTilt = sin(tilt);

    float rot1 = yRot * std::numbers::pi / 180;
    float ss1 = sin(rot1);
    float cc1 = cos(rot1);

    body->tick();
    body->moveTo(x + ss1 * 0.5f, y, z - cc1 * 0.5f, 0, 0);
    wing1->tick();
    wing1->moveTo(x + cc1 * 4.5f, y + 2, z + ss1 * 4.5f, 0, 0);
    wing2->tick();
    wing2->moveTo(x - cc1 * 4.5f, y + 2, z - ss1 * 4.5f, 0, 0);

    if (!level->isClientSide) checkAttack();
    if (!level->isClientSide && hurtDuration == 0) {
        std::vector<std::shared_ptr<Entity> > nearby;
        AABB wing_mov = wing1->bb.grow(4, 2, 4).move(0, -2, 0);
        level->getEntities(shared_from_this(), &wing_mov, nearby);
        knockBack(&nearby);
        wing_mov = wing2->bb.grow(4, 2, 4).move(0, -2, 0);
        level->getEntities(shared_from_this(), &wing_mov, nearby);
        knockBack(&nearby);

        AABB neck_bb = neck->bb.grow(1, 1, 1);
        AABB head_bb = head->bb.grow(1, 1, 1);
        level->getEntities(shared_from_this(), &neck_bb, nearby);
        hurt(&nearby);
        level->getEntities(shared_from_this(), &head_bb, nearby);
        hurt(&nearby);
    }

    double p1components[3];
    std::vector<double> p1 =
        std::vector<double>(p1components, p1components + 3);
    getLatencyPos(p1, 5, 1);

    {
        
        
        

        double yRotDiff = getHeadYRotDiff(1);

        float ss =
            sin((yRot + yRotDiff) * std::numbers::pi / 180 - yRotA * 0.01f);
        float cc =
            cos((yRot + yRotDiff) * std::numbers::pi / 180 - yRotA * 0.01f);
        head->tick();
        neck->tick();
        double yOffset = getHeadYOffset(1);  

        
        
        head->moveTo(x + ss * 6.5f * ccTilt, y + yOffset + ssTilt * 6.5f,
                     z - cc * 6.5f * ccTilt, 0, 0);

        
        
        neck->moveTo(x + ss * 5.5f * ccTilt, y + yOffset + ssTilt * 5.5f,
                     z - cc * 5.5f * ccTilt, 0, 0);

        double acidX = x + ss * 9.5f * ccTilt;
        double acidY = y + yOffset + ssTilt * 10.5f;
        double acidZ = z - cc * 9.5f * ccTilt;
        m_acidArea = {acidX - 5, acidY - 17, acidZ - 5,
                      acidX + 5, acidY + 4,  acidZ + 5};

        
        
        
        
        
        
        
        
        
        
        
        
    }

    
    for (int i = 0; i < 3; i++) {
        std::shared_ptr<MultiEntityMobPart> part = nullptr;

        if (i == 0) part = tail1;
        if (i == 1) part = tail2;
        if (i == 2) part = tail3;

        double p0components[3];
        std::vector<double> p0 =
            std::vector<double>(p0components, p0components + 3);
        getLatencyPos(p0, 12 + i * 2, 1);

        float rot = yRot * std::numbers::pi / 180 +
                    rotWrap(p0[0] - p1[0]) * std::numbers::pi / 180 * (1);
        float ss = sin(rot);
        float cc = cos(rot);

        float dd1 = 1.5f;
        float dd = (i + 1) * 2.0f;
        part->tick();
        part->moveTo(x - (ss1 * dd1 + ss * dd) * ccTilt,
                     y + (p0[1] - p1[1]) * 1 - (dd + dd1) * ssTilt + 1.5f,
                     z + (cc1 * dd1 + cc * dd) * ccTilt, 0, 0);
    }

    
    if (!level->isClientSide) {
        double maxDist = 64.0f;
        if (getSynchedAction() == e_EnderdragonAction_StrafePlayer &&
            attackTarget != nullptr &&
            attackTarget->distanceToSqr(shared_from_this()) <
                maxDist * maxDist) {
            if (this->canSee(attackTarget)) {
                m_fireballCharge++;
                Vec3 aim = Vec3((attackTarget->x - x), 0, (attackTarget->z - z))
                               .normalize();

                Vec3 dir = Vec3(sin(yRot * std::numbers::pi / 180), 0,
                                -cos(yRot * std::numbers::pi / 180))
                               .normalize();

                float dot = (float)dir.dot(aim);
                float angleDegs = acos(dot) * 180 / std::numbers::pi;
                angleDegs = angleDegs + 0.5f;

                if (m_fireballCharge >= 20 &&
                    (angleDegs >= 0 && angleDegs < 10)) {
                    double d = 1;
                    Vec3 v = getViewVector(1);
                    float startingX = head->x - v.x * d;
                    float startingY = head->y + head->bbHeight / 2 + 0.5f;
                    float startingZ = head->z - v.z * d;

                    double xdd = attackTarget->x - startingX;
                    double ydd =
                        (attackTarget->bb.y0 + attackTarget->bbHeight / 2) -
                        (startingY + head->bbHeight / 2);
                    double zdd = attackTarget->z - startingZ;

                    level->levelEvent(nullptr, LevelEvent::SOUND_GHAST_FIREBALL,
                                      (int)x, (int)y, (int)z, 0);
                    std::shared_ptr<DragonFireball> ie =
                        std::make_shared<DragonFireball>(
                            level,
                            std::dynamic_pointer_cast<Mob>(shared_from_this()),
                            xdd, ydd, zdd);
                    ie->x = startingX;
                    ie->y = startingY;
                    ie->z = startingZ;
                    level->addEntity(ie);
                    m_fireballCharge = 0;

                    app.DebugPrintf(
                        "Finding new target due to having fired a fireball\n");
                    if (m_currentPath != nullptr) {
                        while (!m_currentPath->isDone()) {
                            m_currentPath->next();
                        }
                    }
                    newTarget = true;
                    findNewTarget();
                }
            } else {
                if (m_fireballCharge > 0) m_fireballCharge--;
            }
        } else {
            if (m_fireballCharge > 0) m_fireballCharge--;
        }
    }
    

    if (!level->isClientSide) {
        inWall = checkWalls(&head->bb) | checkWalls(&neck->bb) |
                 checkWalls(&body->bb);
    }
}

void EnderDragon::checkCrystals() {
    if (nearestCrystal != nullptr) {
        if (nearestCrystal->removed) {
            if (!level->isClientSide) {
                hurt(head, DamageSource::explosion(nullptr), 10);
            }

            nearestCrystal = nullptr;
        } else if (tickCount % 10 == 0) {
            if (getHealth() < getMaxHealth()) setHealth(getHealth() + 1);
        }
    }

    if (random->nextInt(10) == 0) {
        float maxDist = 32;
        AABB grown = bb.grow(maxDist, maxDist, maxDist);
        std::vector<std::shared_ptr<Entity> >* crystals =
            level->getEntitiesOfClass(typeid(EnderCrystal), &grown);

        std::shared_ptr<EnderCrystal> crystal = nullptr;
        double nearest = std::numeric_limits<double>::max();
        
        for (auto it = crystals->begin(); it != crystals->end(); ++it) {
            std::shared_ptr<EnderCrystal> ec =
                std::dynamic_pointer_cast<EnderCrystal>(*it);
            double dist = ec->distanceToSqr(shared_from_this());
            if (dist < nearest) {
                nearest = dist;
                crystal = ec;
            }
        }
        delete crystals;

        nearestCrystal = crystal;
    }
}

void EnderDragon::checkAttack() {
    
    {
        
        
        
        

        
        
        
    }
}

void EnderDragon::knockBack(std::vector<std::shared_ptr<Entity> >* entities) {
    double xm = (body->bb.x0 + body->bb.x1) / 2;
    
    double zm = (body->bb.z0 + body->bb.z1) / 2;

    
    for (auto it = entities->begin(); it != entities->end(); ++it) {
        if ((*it)->instanceof(eTYPE_LIVINGENTITY))  
        {
            std::shared_ptr<LivingEntity> e =
                std::dynamic_pointer_cast<LivingEntity>(*it);
            double xd = e->x - xm;
            double zd = e->z - zm;
            double dd = xd * xd + zd * zd;
            e->push(xd / dd * 4, 0.2f, zd / dd * 4);
        }
    }
}

void EnderDragon::hurt(std::vector<std::shared_ptr<Entity> >* entities) {
    
    for (auto it = entities->begin(); it != entities->end(); ++it) {
        if ((*it)->instanceof(eTYPE_LIVINGENTITY))  
        {
            std::shared_ptr<LivingEntity> e =
                std::dynamic_pointer_cast<LivingEntity>(
                    *it);  
            DamageSource* damageSource = DamageSource::mobAttack(
                std::dynamic_pointer_cast<LivingEntity>(shared_from_this()));
            e->hurt(damageSource, 10);
            delete damageSource;
        }
    }
}

void EnderDragon::findNewTarget() {
    std::shared_ptr<Player> playerNearestToEgg = nullptr;

    
    switch (getSynchedAction()) {
        case e_EnderdragonAction_Takeoff:
        case e_EnderdragonAction_HoldingPattern: {
            if (!newTarget && m_currentPath != nullptr &&
                m_currentPath->isDone()) {
                
                int eggHeight =
                    std::max(level->seaLevel + 5,
                             level->getTopSolidBlock(
                                 PODIUM_X_POS,
                                 PODIUM_Z_POS));  
                playerNearestToEgg = level->getNearestPlayer(
                    PODIUM_X_POS, eggHeight, PODIUM_Z_POS, 64.0);
                double dist = 64.0f;
                if (playerNearestToEgg != nullptr) {
                    dist = playerNearestToEgg->distanceToSqr(
                        PODIUM_X_POS, eggHeight, PODIUM_Z_POS);
                    dist /= (8 * 8 * 8);
                }
                

                if (random->nextInt(m_remainingCrystalsCount + 3) == 0) {
                    setSynchedAction(e_EnderdragonAction_LandingApproach);
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
                    app.DebugPrintf("Dragon action is now: LandingApproach\n");
#endif
                }
                
                
                else if (playerNearestToEgg != nullptr &&
                         (random->nextInt(std::abs(dist) + 2) == 0 ||
                          random->nextInt(m_remainingCrystalsCount + 2) == 0)) {
                    setSynchedAction(e_EnderdragonAction_StrafePlayer);
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
                    app.DebugPrintf("Dragon action is now: StrafePlayer\n");
#endif
                }
            }
        } break;
        case e_EnderdragonAction_StrafePlayer:
            
            if (m_currentPath == nullptr ||
                (m_currentPath->isDone() && newTarget)) {
                setSynchedAction(e_EnderdragonAction_HoldingPattern);
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
                app.DebugPrintf("Dragon action is now: HoldingPattern\n");
#endif
            }
            break;
        case e_EnderdragonAction_Landing:
            
            
            
            
            

            m_flameAttacks = 0;
            setSynchedAction(e_EnderdragonAction_Sitting_Scanning);
            attackTarget = level->getNearestPlayer(shared_from_this(),
                                                   SITTING_ATTACK_VIEW_RANGE,
                                                   SITTING_ATTACK_Y_VIEW_RANGE);
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
            app.DebugPrintf("Dragon action is now: SittingScanning\n");
#endif
            m_actionTicks = 0;
            break;
        default:
            break;
    };

    newTarget = false;

    
    if (getSynchedAction() == e_EnderdragonAction_StrafePlayer &&
        playerNearestToEgg != nullptr) {
        attackTarget = playerNearestToEgg;
        strafeAttackTarget();
    } else if (getSynchedAction() == e_EnderdragonAction_LandingApproach) {
        
        if (m_currentPath == nullptr || m_currentPath->isDone()) {
            int currentNodeIndex = findClosestNode();

            
            
            int eggHeight = std::max(
                level->seaLevel + 5,
                level->getTopSolidBlock(
                    PODIUM_X_POS, PODIUM_Z_POS));  
            playerNearestToEgg = level->getNearestPlayer(
                PODIUM_X_POS, eggHeight, PODIUM_Z_POS, 128.0);

            int targetNodeIndex = 0;
            if (playerNearestToEgg != nullptr) {
                Vec3 aim = Vec3(playerNearestToEgg->x, 0, playerNearestToEgg->z)
                               .normalize();

                
                
                targetNodeIndex =
                    findClosestNode(-aim.x * 40, 105.0, -aim.z * 40);
            } else {
                targetNodeIndex = findClosestNode(40.0, eggHeight, 0.0);
            }
            Node finalNode(PODIUM_X_POS, eggHeight, PODIUM_Z_POS);

            if (m_currentPath != nullptr) delete m_currentPath;
            m_currentPath =
                findPath(currentNodeIndex, targetNodeIndex, &finalNode);

            
            if (m_currentPath != nullptr) m_currentPath->next();
        }

        m_actionTicks = 0;

        navigateToNextPathNode();

        if (m_currentPath != nullptr && m_currentPath->isDone()) {
            setSynchedAction(e_EnderdragonAction_Landing);
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
            app.DebugPrintf("Dragon action is now: Landing\n");
#endif
        }
    } else if (getSynchedAction() == e_EnderdragonAction_Sitting_Flaming ||
               getSynchedAction() == e_EnderdragonAction_Sitting_Attacking ||
               getSynchedAction() == e_EnderdragonAction_Sitting_Scanning) {
        
    } else {
        
        
        if (m_currentPath == nullptr || m_currentPath->isDone()) {
            int currentNodeIndex = findClosestNode();
            int targetNodeIndex = currentNodeIndex;
            
            

            if (getSynchedAction() == e_EnderdragonAction_Takeoff) {
                Vec3 v = getHeadLookVector(1);
                targetNodeIndex = findClosestNode(-v.x * 40, 105.0, -v.z * 40);
            } else {
                if (random->nextInt(8) == 0) {
                    m_holdingPatternClockwise = !m_holdingPatternClockwise;
                    targetNodeIndex = targetNodeIndex + 6;
                }

                if (m_holdingPatternClockwise)
                    targetNodeIndex = targetNodeIndex + 1;
                else
                    targetNodeIndex = targetNodeIndex - 1;
            }

            if (m_remainingCrystalsCount <= 0) {
                
                targetNodeIndex -= 12;
                targetNodeIndex =
                    targetNodeIndex &
                    7;  
                        
                targetNodeIndex += 12;
            } else {
                
                targetNodeIndex = targetNodeIndex % 12;
                if (targetNodeIndex < 0) targetNodeIndex += 12;
            }

            if (m_currentPath != nullptr) delete m_currentPath;
            m_currentPath = findPath(currentNodeIndex, targetNodeIndex);

            
            if (m_currentPath != nullptr) m_currentPath->next();
        }

        navigateToNextPathNode();

        if (getSynchedAction() != e_EnderdragonAction_StrafePlayer)
            attackTarget = nullptr;
    }
}

float EnderDragon::rotWrap(double d) {
    while (d >= 180) d -= 360;
    while (d < -180) d += 360;
    return (float)d;
}

bool EnderDragon::checkWalls(AABB* bb) {
    int x0 = Mth::floor(bb->x0);
    int y0 = Mth::floor(bb->y0);
    int z0 = Mth::floor(bb->z0);
    int x1 = Mth::floor(bb->x1);
    int y1 = Mth::floor(bb->y1);
    int z1 = Mth::floor(bb->z1);
    bool hitWall = false;
    bool destroyedTile = false;
    for (int x = x0; x <= x1; x++) {
        for (int y = y0; y <= y1; y++) {
            for (int z = z0; z <= z1; z++) {
                int t = level->getTile(x, y, z);
                
                if (t == 0 || t == Tile::fire_Id) {
                } else if (t == Tile::obsidian_Id || t == Tile::endStone_Id ||
                           t == Tile::unbreakable_Id ||
                           !level->getGameRules()->getBoolean(
                               GameRules::RULE_MOBGRIEFING)) {
                    hitWall = true;
                } else {
                    destroyedTile = level->removeTile(x, y, z) || destroyedTile;
                }
            }
        }
    }

    if (destroyedTile) {
        double x = bb->x0 + (bb->x1 - bb->x0) * random->nextFloat();
        double y = bb->y0 + (bb->y1 - bb->y0) * random->nextFloat();
        double z = bb->z0 + (bb->z1 - bb->z0) * random->nextFloat();
        level->addParticle(eParticleType_largeexplode, x, y, z, 0, 0, 0);
    }

    return hitWall;
}

bool EnderDragon::hurt(std::shared_ptr<MultiEntityMobPart> MultiEntityMobPart,
                       DamageSource* source, float damage) {
    if (MultiEntityMobPart != head) {
        damage = damage / 4 + 1;
    }

    
    
    

    
    
    
    

    if (source->getEntity() != nullptr &&
            source->getEntity()->instanceof(eTYPE_PLAYER) ||
        source->isExplosion()) {
        int healthBefore = getHealth();
        reallyHurt(source, damage);

        
        
        if (getHealth() <= 0 &&
            !(getSynchedAction() == e_EnderdragonAction_Sitting_Flaming ||
              getSynchedAction() == e_EnderdragonAction_Sitting_Scanning ||
              getSynchedAction() == e_EnderdragonAction_Sitting_Attacking)) {
            setHealth(1);

            if (setSynchedAction(e_EnderdragonAction_LandingApproach)) {
                if (m_currentPath != nullptr) {
                    while (!m_currentPath->isDone()) {
                        m_currentPath->next();
                    }
                }
                app.DebugPrintf("Dragon should be dead, so landing.\n");
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
                app.DebugPrintf("Dragon action is now: LandingApproach\n");
#endif
                findNewTarget();
            }
        }

        if (getSynchedAction() == e_EnderdragonAction_Sitting_Flaming ||
            getSynchedAction() == e_EnderdragonAction_Sitting_Scanning ||
            getSynchedAction() == e_EnderdragonAction_Sitting_Attacking) {
            m_sittingDamageReceived += healthBefore - getHealth();

            if (m_sittingDamageReceived >
                (SITTING_ALLOWED_DAMAGE_PERCENTAGE * getMaxHealth())) {
                m_sittingDamageReceived = 0;
                setSynchedAction(e_EnderdragonAction_Takeoff);
                newTarget = true;
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
                app.DebugPrintf("Dragon action is now: Takeoff\n");
#endif
            }
        }
    }
    return true;
}

bool EnderDragon::hurt(DamageSource* source, float damage) { return false; }

bool EnderDragon::reallyHurt(DamageSource* source, float damage) {
    return Mob::hurt(source, damage);
}

void EnderDragon::tickDeath() {
    if (getSynchedAction() != e_EnderdragonAction_Sitting_Flaming &&
        getSynchedAction() != e_EnderdragonAction_Sitting_Scanning &&
        getSynchedAction() != e_EnderdragonAction_Sitting_Attacking) {
        if (!level->isClientSide) setHealth(1);
        return;
    }

    dragonDeathTime++;
    if (dragonDeathTime >= 180 && dragonDeathTime <= 200) {
        float xo = (random->nextFloat() - 0.5f) * 8;
        float yo = (random->nextFloat() - 0.5f) * 4;
        float zo = (random->nextFloat() - 0.5f) * 8;
        level->addParticle(eParticleType_hugeexplosion, x + xo, y + 2 + yo,
                           z + zo, 0, 0, 0);
    }
    if (!level->isClientSide) {
        if (dragonDeathTime > 150 && dragonDeathTime % 5 == 0) {
            int xpCount = 1000;
            while (xpCount > 0) {
                int newCount = ExperienceOrb::getExperienceValue(xpCount);
                xpCount -= newCount;
                level->addEntity(std::shared_ptr<ExperienceOrb>(
                    new ExperienceOrb(level, x, y, z, newCount)));
            }
        }
        if (dragonDeathTime == 1) {
            level->globalLevelEvent(LevelEvent::SOUND_DRAGON_DEATH, (int)x,
                                    (int)y, (int)z, 0);
        }
    }
    move(0, 0.1f, 0);
    yBodyRot = yRot += 20.0f;

    if (dragonDeathTime == 200 && !level->isClientSide) {
        
        

        int xpCount = 2000;
        while (xpCount > 0) {
            int newCount = ExperienceOrb::getExperienceValue(xpCount);
            xpCount -= newCount;
            level->addEntity(std::shared_ptr<ExperienceOrb>(
                new ExperienceOrb(level, x, y, z, newCount)));
        }
        int xo = 5 + random->nextInt(2) * 2 - 1;
        int zo = 5 + random->nextInt(2) * 2 - 1;
        if (random->nextInt(2) == 0) {
            xo = 0;
        } else {
            zo = 0;
        }
        
        spawnExitPortal(0, 0);  
        remove();
    }
}

void EnderDragon::spawnExitPortal(int x, int z) {
    int y = level->seaLevel;

    TheEndPortal::allowAnywhere(true);

    int r = 4;
    for (int yy = y - 1; yy <= y + 32; yy++) {
        for (int xx = x - r; xx <= x + r; xx++) {
            for (int zz = z - r; zz <= z + r; zz++) {
                double xd = xx - x;
                double zd = zz - z;
                double d = sqrt(xd * xd + zd * zd);
                if (d <= r - 0.5) {
                    if (yy < y) {
                        if (d > r - 1 - 0.5) {
                        } else {
                            level->setTileAndUpdate(xx, yy, zz,
                                                    Tile::unbreakable_Id);
                        }
                    } else if (yy > y) {
                        level->setTileAndUpdate(xx, yy, zz, 0);
                    } else {
                        if (d > r - 1 - 0.5) {
                            level->setTileAndUpdate(xx, yy, zz,
                                                    Tile::unbreakable_Id);
                        } else {
                            level->setTileAndUpdate(xx, yy, zz,
                                                    Tile::endPortalTile_Id);
                        }
                    }
                }
            }
        }
    }

    level->setTileAndUpdate(x, y + 0, z, Tile::unbreakable_Id);
    level->setTileAndUpdate(x, y + 1, z, Tile::unbreakable_Id);
    level->setTileAndUpdate(x, y + 2, z, Tile::unbreakable_Id);
    level->setTileAndUpdate(x - 1, y + 2, z, Tile::torch_Id);
    level->setTileAndUpdate(x + 1, y + 2, z, Tile::torch_Id);
    level->setTileAndUpdate(x, y + 2, z - 1, Tile::torch_Id);
    level->setTileAndUpdate(x, y + 2, z + 1, Tile::torch_Id);
    level->setTileAndUpdate(x, y + 3, z, Tile::unbreakable_Id);
    level->setTileAndUpdate(x, y + 4, z, Tile::dragonEgg_Id);

    
    
    for (int yy = y - 5; yy < y - 1; yy++) {
        for (int xx = x - (r - 1); xx <= x + (r - 1); xx++) {
            for (int zz = z - (r - 1); zz <= z + (r - 1); zz++) {
                if (level->isEmptyTile(xx, yy, zz)) {
                    level->setTileAndUpdate(xx, yy, zz, Tile::endStone_Id);
                }
            }
        }
    }

    TheEndPortal::allowAnywhere(false);
}

void EnderDragon::checkDespawn() {}

std::vector<std::shared_ptr<Entity> >* EnderDragon::getSubEntities() {
    return &subEntities;
}

bool EnderDragon::isPickable() { return false; }

Level* EnderDragon::getLevel() { return level; }

int EnderDragon::getAmbientSound() {
    return eSoundType_MOB_ENDERDRAGON_GROWL;  
}

int EnderDragon::getHurtSound() {
    return eSoundType_MOB_ENDERDRAGON_HIT;  
}

float EnderDragon::getSoundVolume() { return 5; }


bool EnderDragon::setSynchedAction(EEnderdragonAction action,
                                   bool force ) {
    bool validTransition = false;
    
    switch (getSynchedAction()) {
        case e_EnderdragonAction_HoldingPattern:
            switch (action) {
                case e_EnderdragonAction_StrafePlayer:
                case e_EnderdragonAction_LandingApproach:
                    validTransition = true;
                    break;
                default:
                    break;
            };
            break;
        case e_EnderdragonAction_StrafePlayer:
            switch (action) {
                case e_EnderdragonAction_HoldingPattern:
                case e_EnderdragonAction_LandingApproach:
                    validTransition = true;
                    break;
                default:
                    break;
            };
            break;
        case e_EnderdragonAction_LandingApproach:
            switch (action) {
                case e_EnderdragonAction_Landing:
                    validTransition = true;
                    break;
                default:
                    break;
            };
            break;
        case e_EnderdragonAction_Landing:
            switch (action) {
                case e_EnderdragonAction_Sitting_Flaming:
                case e_EnderdragonAction_Sitting_Scanning:
                    validTransition = true;
                    break;
                default:
                    break;
            };
            break;
        case e_EnderdragonAction_Takeoff:
            switch (action) {
                case e_EnderdragonAction_HoldingPattern:
                    validTransition = true;
                    break;
                default:
                    break;
            };
            break;
        case e_EnderdragonAction_Sitting_Flaming:
            switch (action) {
                case e_EnderdragonAction_Sitting_Scanning:
                case e_EnderdragonAction_Sitting_Attacking:
                case e_EnderdragonAction_Takeoff:
                    validTransition = true;
                    break;
                default:
                    break;
            };
            break;
        case e_EnderdragonAction_Sitting_Scanning:
            switch (action) {
                case e_EnderdragonAction_Sitting_Flaming:
                case e_EnderdragonAction_Sitting_Attacking:
                case e_EnderdragonAction_Takeoff:
                    validTransition = true;
                    break;
                default:
                    break;
            };
            break;
        case e_EnderdragonAction_Sitting_Attacking:
            switch (action) {
                case e_EnderdragonAction_Sitting_Flaming:
                case e_EnderdragonAction_Sitting_Scanning:
                case e_EnderdragonAction_Takeoff:
                    validTransition = true;
                    break;
                default:
                    break;
            };
            break;
        default:
            break;
    };

    if (force || validTransition) {
        entityData->set(DATA_ID_SYNCHED_ACTION, action);
    } else {
        app.DebugPrintf("EnderDragon: Invalid state transition from %d to %d\n",
                        getSynchedAction(), action);
    }

    return force || validTransition;
}

EnderDragon::EEnderdragonAction EnderDragon::getSynchedAction() {
    return (EEnderdragonAction)entityData->getInteger(DATA_ID_SYNCHED_ACTION);
}

void EnderDragon::handleCrystalDestroyed(DamageSource* source) {
    AABB tempBB(PODIUM_X_POS, 84.0, PODIUM_Z_POS, PODIUM_X_POS + 1.0, 85.0,
                PODIUM_Z_POS + 1.0);
    AABB grown = tempBB.grow(48, 40, 48);
    std::vector<std::shared_ptr<Entity> >* crystals =
        level->getEntitiesOfClass(typeid(EnderCrystal), &grown);
    m_remainingCrystalsCount = (int)crystals->size() - 1;
    if (m_remainingCrystalsCount < 0) m_remainingCrystalsCount = 0;
    delete crystals;

    app.DebugPrintf("Crystal count is now %d\n", m_remainingCrystalsCount);

    

    if (m_remainingCrystalsCount % 2 == 0) {
        if (setSynchedAction(e_EnderdragonAction_LandingApproach)) {
            if (m_currentPath != nullptr) {
                while (!m_currentPath->isDone()) {
                    m_currentPath->next();
                }
            }
            m_actionTicks = 1;
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
            app.DebugPrintf("Dragon action is now: LandingApproach\n");
#endif
        }
    } else if (source->getEntity() != nullptr &&
               source->getEntity()->instanceof(eTYPE_PLAYER)) {
        if (setSynchedAction(e_EnderdragonAction_StrafePlayer)) {
            attackTarget =
                std::dynamic_pointer_cast<Player>(source->getEntity());
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
            app.DebugPrintf("Dragon action is now: StrafePlayer\n");
#endif
            strafeAttackTarget();
        }
    }
}

void EnderDragon::strafeAttackTarget() {
    app.DebugPrintf("Setting path to strafe attack target\n");
    int currentNodeIndex = findClosestNode();
    int targetNodeIndex =
        findClosestNode(attackTarget->x, attackTarget->y, attackTarget->z);

    int finalXTarget = attackTarget->x;
    int finalZTarget = attackTarget->z;

    double xd = finalXTarget - x;
    double zd = finalZTarget - z;
    double sd = sqrt(xd * xd + zd * zd);
    double ho = 0.4f + sd / 80.0f - 1;
    if (ho > 10) ho = 10;
    int finalYTarget = attackTarget->bb.y0 + ho;

    Node finalNode(finalXTarget, finalYTarget, finalZTarget);

    if (m_currentPath != nullptr) delete m_currentPath;
    m_currentPath = findPath(currentNodeIndex, targetNodeIndex, &finalNode);

    if (m_currentPath != nullptr) {
        
        m_currentPath->next();

        navigateToNextPathNode();
    }
}

void EnderDragon::navigateToNextPathNode() {
    if (m_currentPath != nullptr && !m_currentPath->isDone()) {
        Vec3 curr = m_currentPath->currentPos();

        m_currentPath->next();
        xTarget = curr.x;

        if (getSynchedAction() == e_EnderdragonAction_LandingApproach &&
            m_currentPath->isDone()) {
            
            
            yTarget = curr.y;
        } else {
            do {
                yTarget = curr.y + random->nextFloat() * 20;
            } while (yTarget < (curr.y));
        }
        zTarget = curr.z;
        app.DebugPrintf("Path node pos is (%f,%f,%f)\n", curr.x, curr.y,
                        curr.z);
        app.DebugPrintf("Setting new target to (%f,%f,%f)\n", xTarget, yTarget,
                        zTarget);
    }
}

int EnderDragon::findClosestNode() {
    
    if ((*m_nodes)[0] == nullptr) {
        
        
        
        
        int nodeX = 0;
        int nodeY = 0;
        int nodeZ = 0;
        int multiplier = 0;
        for (unsigned int i = 0; i < 24; ++i) {
            int yAdjustment = 5;
            multiplier = i;
            if (i < 12) {
                nodeX = 60 * cosf(2 * (-std::numbers::pi +
                                       (std::numbers::pi / 12) * multiplier));
                nodeZ = 60 * sinf(2 * (-std::numbers::pi +
                                       (std::numbers::pi / 12) * multiplier));
            } else if (i < 20) {
                multiplier -= 12;
                nodeX = 40 * cosf(2 * (-std::numbers::pi +
                                       (std::numbers::pi / 8) * multiplier));
                nodeZ = 40 * sinf(2 * (-std::numbers::pi +
                                       (std::numbers::pi / 8) * multiplier));
                yAdjustment +=
                    10;  
            } else {
                multiplier -= 20;
                nodeX = 20 * cosf(2 * (-std::numbers::pi +
                                       (std::numbers::pi / 4) * multiplier));
                nodeZ = 20 * sinf(2 * (-std::numbers::pi +
                                       (std::numbers::pi / 4) * multiplier));
            }
            
            
            nodeY =
                std::max((level->seaLevel + 10),
                         level->getTopSolidBlock(nodeX, nodeZ) + yAdjustment);

            app.DebugPrintf("Node %d is at (%d,%d,%d)\n", i, nodeX, nodeY,
                            nodeZ);

            (*m_nodes)[i] = new Node(nodeX, nodeY, nodeZ);

            
        }

        m_nodeAdjacency[0] = (1 << 11) | (1 << 1) | (1 << 12);
        m_nodeAdjacency[1] = (1 << 0) | (1 << 2) | (1 << 13);
        m_nodeAdjacency[2] = (1 << 1) | (1 << 3) | (1 << 13);
        m_nodeAdjacency[3] = (1 << 2) | (1 << 4) | (1 << 14);
        m_nodeAdjacency[4] = (1 << 3) | (1 << 5) | (1 << 15);
        m_nodeAdjacency[5] = (1 << 4) | (1 << 6) | (1 << 15);
        m_nodeAdjacency[6] = (1 << 5) | (1 << 7) | (1 << 16);
        m_nodeAdjacency[7] = (1 << 6) | (1 << 8) | (1 << 17);
        m_nodeAdjacency[8] = (1 << 7) | (1 << 9) | (1 << 17);
        m_nodeAdjacency[9] = (1 << 8) | (1 << 10) | (1 << 18);
        m_nodeAdjacency[10] = (1 << 9) | (1 << 11) | (1 << 19);
        m_nodeAdjacency[11] = (1 << 10) | (1 << 0) | (1 << 19);

        m_nodeAdjacency[12] = (1 << 0) | (1 << 13) | (1 << 20) | (1 << 19);
        m_nodeAdjacency[13] =
            (1 << 1) | (1 << 2) | (1 << 14) | (1 << 21) | (1 << 20) | (1 << 12);
        m_nodeAdjacency[14] = (1 << 3) | (1 << 15) | (1 << 21) | (1 << 13);
        m_nodeAdjacency[15] =
            (1 << 4) | (1 << 5) | (1 << 16) | (1 << 22) | (1 << 21) | (1 << 14);
        m_nodeAdjacency[16] = (1 << 6) | (1 << 17) | (1 << 22) | (1 << 15);
        m_nodeAdjacency[17] =
            (1 << 7) | (1 << 8) | (1 << 18) | (1 << 23) | (1 << 22) | (1 << 16);
        m_nodeAdjacency[18] = (1 << 9) | (1 << 19) | (1 << 23) | (1 << 17);
        m_nodeAdjacency[19] = (1 << 10) | (1 << 11) | (1 << 12) | (1 << 20) |
                              (1 << 23) | (1 << 18);

        m_nodeAdjacency[20] = (1 << 12) | (1 << 13) | (1 << 21) | (1 << 22) |
                              (1 << 23) | (1 << 19);
        m_nodeAdjacency[21] = (1 << 14) | (1 << 15) | (1 << 22) | (1 << 23) |
                              (1 << 20) | (1 << 13);
        m_nodeAdjacency[22] = (1 << 15) | (1 << 16) | (1 << 17) | (1 << 23) |
                              (1 << 20) | (1 << 21);
        m_nodeAdjacency[23] = (1 << 17) | (1 << 18) | (1 << 19) | (1 << 20) |
                              (1 << 21) | (1 << 22);
    }

    return findClosestNode(x, y, z);
}

int EnderDragon::findClosestNode(double tX, double tY, double tZ) {
    float closestDist = 100.0f;
    int closestIndex = 0;
    Node* currentPos = new Node((int)floor(tX), (int)floor(tY), (int)floor(tZ));
    int startIndex = 0;
    if (m_remainingCrystalsCount <= 0) {
        
        
        startIndex = 12;
    }
    for (unsigned int i = startIndex; i < 24; ++i) {
        if ((*m_nodes)[i] != nullptr) {
            float dist = (*m_nodes)[i]->distanceTo(currentPos);
            if (dist < closestDist) {
                closestDist = dist;
                closestIndex = i;
            }
        }
    }
    delete currentPos;
    return closestIndex;
}


Path* EnderDragon::findPath(int startIndex, int endIndex,
                            Node* finalNode ) {
    for (unsigned int i = 0; i < 24; ++i) {
        Node* n = (*m_nodes)[i];
        n->closed = false;
        n->f = 0;
        n->g = 0;
        n->h = 0;
        n->cameFrom = nullptr;
        n->heapIdx = -1;
    }

    Node* from = (*m_nodes)[startIndex];
    Node* to = (*m_nodes)[endIndex];

    from->g = 0;
    from->h = from->distanceTo(to);
    from->f = from->h;

    openSet->clear();
    openSet->insert(from);

    Node* closest = from;

    int minimumNodeIndex = 0;
    if (m_remainingCrystalsCount <= 0) {
        
        
        minimumNodeIndex = 12;
    }

    while (!openSet->isEmpty()) {
        Node* x = openSet->pop();

        if (x->equals(to)) {
            app.DebugPrintf("Found path from %d to %d\n", startIndex, endIndex);
            if (finalNode != nullptr) {
                finalNode->cameFrom = to;
                to = finalNode;
            }
            return reconstruct_path(from, to);
        }

        if (x->distanceTo(to) < closest->distanceTo(to)) {
            closest = x;
        }
        x->closed = true;

        unsigned int xIndex = 0;
        for (unsigned int i = 0; i < 24; ++i) {
            if ((*m_nodes)[i] == x) {
                xIndex = i;
                break;
            }
        }

        for (int i = minimumNodeIndex; i < 24; i++) {
            if (m_nodeAdjacency[xIndex] & (1 << i)) {
                Node* y = (*m_nodes)[i];

                if (y->closed) continue;

                float tentative_g_score = x->g + x->distanceTo(y);
                if (!y->inOpenSet() || tentative_g_score < y->g) {
                    y->cameFrom = x;
                    y->g = tentative_g_score;
                    y->h = y->distanceTo(to);
                    if (y->inOpenSet()) {
                        openSet->changeCost(y, y->g + y->h);
                    } else {
                        y->f = y->g + y->h;
                        openSet->insert(y);
                    }
                }
            }
        }
    }

    if (closest == from) return nullptr;
    app.DebugPrintf("Failed to find path from %d to %d\n", startIndex,
                    endIndex);
    if (finalNode != nullptr) {
        finalNode->cameFrom = closest;
        closest = finalNode;
    }
    return reconstruct_path(from, closest);
}


Path* EnderDragon::reconstruct_path(Node* from, Node* to) {
    int count = 1;
    Node* n = to;
    while (n->cameFrom != nullptr) {
        count++;
        n = n->cameFrom;
    }

    std::vector<Node*> nodes = std::vector<Node*>(count);
    n = to;
    nodes.data()[--count] = n;
    while (n->cameFrom != nullptr) {
        n = n->cameFrom;
        nodes.data()[--count] = n;
    }
    Path* ret = new Path(nodes);
    return ret;
}

void EnderDragon::addAdditonalSaveData(CompoundTag* entityTag) {
    app.DebugPrintf("Adding EnderDragon additional save data\n");
    entityTag->putShort(L"RemainingCrystals", m_remainingCrystalsCount);
    entityTag->putInt(L"DragonState", (int)getSynchedAction());

    Mob::addAdditonalSaveData(entityTag);
}

void EnderDragon::readAdditionalSaveData(CompoundTag* tag) {
    app.DebugPrintf("Reading EnderDragon additional save data\n");
    m_remainingCrystalsCount = tag->getShort(L"RemainingCrystals");
    if (!tag->contains(L"RemainingCrystals"))
        m_remainingCrystalsCount = CRYSTAL_COUNT;

    if (tag->contains(L"DragonState"))
        setSynchedAction((EEnderdragonAction)tag->getInt(L"DragonState"), true);

    Mob::readAdditionalSaveData(tag);
}

float EnderDragon::getTilt(float a) {
    float tilt = 0.0f;
    
    
    
    
    
    
    
    
    {
        double latencyPosAcomponents[3], latencyPosBcomponents[3];
        std::vector<double> latencyPosA = std::vector<double>(
            latencyPosAcomponents, latencyPosAcomponents + 3);
        std::vector<double> latencyPosB = std::vector<double>(
            latencyPosBcomponents, latencyPosBcomponents + 3);
        getLatencyPos(latencyPosA, 5, a);
        getLatencyPos(latencyPosB, 10, a);

        tilt = (latencyPosA[1] - latencyPosB[1]) * 10;
    }
    

    return tilt;
}

double EnderDragon::getHeadYOffset(float a) {
    double headYOffset = 0.0;
    if (getSynchedAction() == e_EnderdragonAction_Sitting_Flaming ||
        getSynchedAction() == e_EnderdragonAction_Sitting_Scanning ||
        getSynchedAction() == e_EnderdragonAction_Sitting_Attacking) {
        headYOffset = -1.0;
    } else {
        double p1components[3];
        std::vector<double> p1 =
            std::vector<double>(p1components, p1components + 3);
        getLatencyPos(p1, 5, 1);

        double p0components[3];
        std::vector<double> p0 =
            std::vector<double>(p0components, p0components + 3);
        getLatencyPos(p0, 0, 1);

        headYOffset = (p0[1] - p1[1]) * 1;
    }
    
    return headYOffset;
}

double EnderDragon::getHeadYRotDiff(float a) {
    double result = 0.0;
    
    
    
    
    
    
    return result;
}

double EnderDragon::getHeadPartYOffset(int partIndex,
                                       std::vector<double>& bodyPos,
                                       std::vector<double>& partPos) {
    double result = 0.0;
    if (getSynchedAction() == e_EnderdragonAction_Landing ||
        getSynchedAction() == e_EnderdragonAction_Takeoff) {
        int eggHeight = level->getTopSolidBlock(
            PODIUM_X_POS, PODIUM_Z_POS);  
        float dist =
            sqrt(distanceToSqr(PODIUM_X_POS, eggHeight, PODIUM_Z_POS)) / 4;
        if (dist < 1.0f) dist = 1.0f;
        result = partIndex / dist;
        
        
    } else if (getSynchedAction() == e_EnderdragonAction_Sitting_Flaming ||
               getSynchedAction() == e_EnderdragonAction_Sitting_Scanning ||
               getSynchedAction() == e_EnderdragonAction_Sitting_Attacking) {
        result = partIndex;
    } else {
        if (partIndex == 6) {
            result = 0.0;
        } else {
            result = partPos[1] - bodyPos[1];
        }
    }
    
    return result;
}

double EnderDragon::getHeadPartYRotDiff(int partIndex,
                                        std::vector<double>& bodyPos,
                                        std::vector<double>& partPos) {
    double result = 0.0;
    
    
    
    
    
    
    
    {
        result = partPos[0] - bodyPos[0];
    }
    
    return result;
}

Vec3 EnderDragon::getHeadLookVector(float a) {
    Vec3 result;

    if (getSynchedAction() == e_EnderdragonAction_Landing ||
        getSynchedAction() == e_EnderdragonAction_Takeoff) {
        int eggHeight = level->getTopSolidBlock(
            PODIUM_X_POS, PODIUM_Z_POS);  
        float dist =
            sqrt(distanceToSqr(PODIUM_X_POS, eggHeight, PODIUM_Z_POS)) / 4;
        if (dist < 1.0f) dist = 1.0f;
        
        float yOffset = 6.0f / dist;

        double xRotTemp = xRot;
        double rotScale = 1.5f;
        xRot = -yOffset * rotScale * 5.0f;

        double yRotTemp = yRot;
        yRot += getHeadYRotDiff(a);

        result = getViewVector(a);

        xRot = xRotTemp;
        yRot = yRotTemp;
    } else if (getSynchedAction() == e_EnderdragonAction_Sitting_Flaming ||
               getSynchedAction() == e_EnderdragonAction_Sitting_Scanning ||
               getSynchedAction() == e_EnderdragonAction_Sitting_Attacking) {
        double xRotTemp = xRot;
        double rotScale = 1.5f;
        
        xRot = -6.0f * rotScale * 5.0f;

        double yRotTemp = yRot;
        yRot += getHeadYRotDiff(a);

        result = getViewVector(a);

        xRot = xRotTemp;
        yRot = yRotTemp;
    } else {
        result = getViewVector(a);
    }

    return result;
}
